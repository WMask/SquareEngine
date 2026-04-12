/***************************************************************************
* STextRenderSystemDX11.cpp
*/

#include "RenderSystem/DX11/STextRenderSystemDX11.h"
#include "RenderSystem/DX11/SConstantBuffersDX11.h"
#include "RenderSystem/SECSComponents.h"
#include "Core/SException.h"


static const char* TextShaderName = "Text2d.shader";

STextRenderSystemDX11::STextRenderSystemDX11(IRenderSystemDX11& renderSystem) : renderSystemDX11(renderSystem)
{
}

STextRenderSystemDX11::~STextRenderSystemDX11()
{
	Shutdown();
}

void STextRenderSystemDX11::Shutdown()
{
	if (instanceBuffer)
	{
		instanceBuffer.Reset();
	}

	spriteVertexBuffer = nullptr;
	spriteIndexBuffer = nullptr;
}

bool STextRenderSystemDX11::CheckShaderName(const std::string& inShaderName)
{
	if (inShaderName.ends_with(TextShaderName))
	{
		shaderName = inShaderName;
		return true;
	}

	return false;
}

void STextRenderSystemDX11::Setup(IRenderSystem::SShaderData& shaderData)
{
	S_TRY

	auto& shaderDataDX11 = static_cast<SShaderDataDX11&>(shaderData);
	spriteVertexBuffer = renderSystemDX11.GetConstantBuffers().GetSpriteVB();
	spriteIndexBuffer = renderSystemDX11.GetConstantBuffers().GetSpriteIB();
	auto d3dDevice = renderSystemDX11.GetDevice();
	if (!d3dDevice)
	{
		throw std::exception("Wrong context");
	}

	// create input layouts
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "SV_Position",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "INSTANCEPOS",   0, DXGI_FORMAT_R32G32B32_FLOAT,    1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCESCALE", 0, DXGI_FORMAT_R32G32_FLOAT,       1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCECOLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 20, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEUV",    0, DXGI_FORMAT_R32G32_FLOAT,       1, 36, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEUV",    1, DXGI_FORMAT_R32G32_FLOAT,       1, 44, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEUV",    2, DXGI_FORMAT_R32G32_FLOAT,       1, 52, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEUV",    3, DXGI_FORMAT_R32G32_FLOAT,       1, 60, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	if (FAILED(d3dDevice->CreateInputLayout(layout, std::size(layout),
		shaderDataDX11.vsCode->GetBufferPointer(),
		shaderDataDX11.vsCode->GetBufferSize(),
		shaderDataDX11.layout.GetAddressOf())))
	{
		throw std::exception("Cannot create input layout");
	}

	// create instance buffer
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(DX11TEXTGLYPHINSTANCE) * SConst::MaxInstancedSpritesCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(d3dDevice->CreateBuffer(&bufferDesc, NULL, instanceBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create vertex buffer");
	}

	S_CATCH{ S_THROW("STextRenderSystemDX11::Setup()") }
}

void STextRenderSystemDX11::Render(float deltaSeconds, float gameTime)
{
	S_TRY

	auto deviceContext = renderSystemDX11.GetDeviceContext();
	auto shader = renderSystemDX11.FindShader(shaderName);
	auto world = renderSystemDX11.GetWorld();
	if (!deviceContext || !shader || !world || !spriteVertexBuffer || !spriteIndexBuffer || !instanceBuffer)
	{
		if (renderSystemDX11.IsNeedDebugTrace())
		{
			DebugMsg("[%s] STextRenderSystemDX11::Render(): wrong context\n",
				GetTimeStamp(std::chrono::system_clock::now()).c_str());
		}
		return;
	}

	// setup context
	ID3D11Buffer* buffers[2] = { spriteVertexBuffer, instanceBuffer.Get() };
	static const UINT strides[2] = { sizeof(DX11SPRITEVERTEX), sizeof(DX11TEXTGLYPHINSTANCE) };
	static const UINT offsets[2] = { 0, 0 };
	deviceContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	deviceContext->IASetIndexBuffer(spriteIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	deviceContext->IASetInputLayout(shader->layout.Get());
	deviceContext->VSSetShader(shader->vs.Get(), NULL, 0);
	deviceContext->PSSetShader(shader->ps.Get(), NULL, 0);

	// prepare cached state
	if (batchData.capacity() < SConst::MaxInstancedSpritesCount) batchData.reserve(SConst::MaxInstancedSpritesCount);
	cachedTexView = nullptr;
	batchesRendered = 0u;
	numGlyphs = 0u;
	cachedTexId = 0u;

	// render sprites
	const auto& registry = world->GetEntities();
	const auto& textView = registry.view<
		const SSpriteComponent,
		const SWidgetComponent,
		const STextComponent>();
	textView.each([this, world](
		const SSpriteComponent& spriteComponent,
		const SWidgetComponent& widgetComponent,
		const STextComponent& textComponent)
	{
		if (!widgetComponent.bVisible) return;

		const IFontSystem& fonts = world->GetFonts();
		auto [text, bTextFound] = fonts.GetLocale()->Get(textComponent.textId);
		if (!bTextFound)
		{
			DebugMsg("[%s] STextRenderSystemDX11::Render(): cannot find text id=%d\n",
				GetTimeStamp(std::chrono::system_clock::now()).c_str(), textComponent.textId);
			return;
		}

		auto [texId, bTexFound] = fonts.GetTextureId(textComponent.fontId, fonts.GetLocale()->GetCulture());
		if (!cachedTexView)
		{
			auto [view, texSize] = renderSystemDX11.FindTexture(texId);
			cachedTexId = texId;
			cachedTexSize = texSize;
			cachedTexView = view;
		}

		// get glyphs
		SSize2F textSize;
		std::vector<SGlyph> glyphs;
		glyphs.reserve(text.length());
		if (!fonts.FindGlyphs(textComponent.fontId, text, &glyphs, &textSize))
		{
			DebugMsg("[%s] STextRenderSystemDX11::Render(): cannot load string glyphs\n",
				GetTimeStamp(std::chrono::system_clock::now()).c_str(), textComponent.textId);
			return;
		}

		// compute text align
		glyphOffset = 0.0f;
		float alignOffset = (textSize.width / -2.0f);
		switch (textComponent.align)
		{
		case STextAlign::Begin:
			alignOffset = (spriteComponent.size.width / -2.0f);
			break;
		case STextAlign::End:
			alignOffset = (spriteComponent.size.width / 2.0f) - textSize.width;
			break;
		}

		// render glyphs
		for (SGlyph& glyph : glyphs)
		{
			// generate glyph uv
			glyph.size.height = textSize.height;
			SSpriteUV tmpUV;
			textComponent.GenerateGlyphUV(glyph, SConvert::ToSize2F(cachedTexSize), tmpUV);

			// store instance data
			DX11TEXTGLYPHINSTANCE instance{};
			instance.pos = spriteComponent.position;
			instance.pos.x += glyphOffset + alignOffset + glyph.size.width / 2.0f;
			instance.scale = SConvert::ToVector2(glyph.size);
			instance.color = textComponent.color;
			memcpy(instance.uvs, tmpUV.uvs, sizeof(SVector2) * 4);
			batchData.push_back(instance);
			glyphOffset += glyph.size.width;

			if (batchData.size() == SConst::MaxInstancedSpritesCount)
			{
				// render if max number reached
				RenderBatch();
			}

			numGlyphs++;
		}

		if (!batchData.empty())
		{
			// render full string
			RenderBatch();
		}
	});

	if (renderSystemDX11.IsNeedDebugTrace())
	{
		renderSystemDX11.AddDrawCalls(batchesRendered);
		DebugMsg("[%s] STextRenderSystemDX11::Render(): %d batches, %d glyph instances\n",
			GetTimeStamp(std::chrono::system_clock::now()).c_str(), batchesRendered, numGlyphs);
	}

	S_CATCH{ S_THROW("STextRenderSystemDX11::Render()") }
}

void STextRenderSystemDX11::RenderBatch()
{
	auto deviceContext = renderSystemDX11.GetDeviceContext();
	if (deviceContext && cachedTexView)
	{
		// fill instanced vertex buffer
		const std::uint32_t numInstances = batchData.size();
		D3D11_MAPPED_SUBRESOURCE mappedResource{};
		if (FAILED(deviceContext->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		{
			throw std::exception("Cannot update vertex buffer");
		}
		memcpy(mappedResource.pData, batchData.data(), sizeof(DX11TEXTGLYPHINSTANCE) * numInstances);
		deviceContext->Unmap(instanceBuffer.Get(), 0);

		// render sprites
		deviceContext->PSSetShaderResources(0, 1, &cachedTexView);
		deviceContext->DrawIndexedInstanced(4, numInstances, 0, 0, 0);
		batchesRendered++;
	}

	batchData.clear();
}
