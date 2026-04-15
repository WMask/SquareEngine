/***************************************************************************
* STexturedSpriteRenderSystemDX11.cpp
*/

#include "RenderSystem/DX11/STexturedSpriteRenderSystemDX11.h"
#include "RenderSystem/DX11/SRenderSystemTypesDX11.h"
#include "RenderSystem/DX11/SConstantBuffersDX11.h"
#include "RenderSystem/SECSComponents.h"
#include "Core/SException.h"


static const char* TexturedSpriteShaderName = "TexturedSprite2d.shader";

STexturedSpriteRenderSystemDX11::STexturedSpriteRenderSystemDX11(IRenderSystemDX11& renderSystem) : renderSystemDX11(renderSystem)
{
}

STexturedSpriteRenderSystemDX11::~STexturedSpriteRenderSystemDX11()
{
	Shutdown();
}

void STexturedSpriteRenderSystemDX11::Shutdown()
{
	if (instanceBuffer)
	{
		instanceBuffer.Reset();
	}

	spriteVertexBuffer = nullptr;
	spriteIndexBuffer = nullptr;
}

bool STexturedSpriteRenderSystemDX11::CheckShaderName(const std::string& inShaderName)
{
	if (inShaderName.ends_with(TexturedSpriteShaderName))
	{
		shaderName = inShaderName;
		return true;
	}

	return false;
}

void STexturedSpriteRenderSystemDX11::Setup(IRenderSystem::SShaderData& shaderData)
{
	S_TRY

	auto& shaderDataDX11 = static_cast<SShaderDataDX11&>(shaderData);
	spriteVertexBuffer = renderSystemDX11.GetConstantBuffers().GetSpriteVB();
	spriteIndexBuffer = renderSystemDX11.GetConstantBuffers().GetSpriteIB();
	auto d3dDevice = renderSystemDX11.GetDevice();
	if (!d3dDevice)
	{
		throw std::exception("Invalid render device");
	}

	// create input layouts
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "SV_Position",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,   D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "INSTANCEPOS",   0, DXGI_FORMAT_R32G32B32_FLOAT,    1, 0,   D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEROT",   0, DXGI_FORMAT_R32_FLOAT,          1, 12,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCESCALE", 0, DXGI_FORMAT_R32G32_FLOAT,       1, 16,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCECOLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 24,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCECOLOR", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 40,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCECOLOR", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 56,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCECOLOR", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 72,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEUV",    0, DXGI_FORMAT_R32G32_FLOAT,       1, 88,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEUV",    1, DXGI_FORMAT_R32G32_FLOAT,       1, 96,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEUV",    2, DXGI_FORMAT_R32G32_FLOAT,       1, 104, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEUV",    3, DXGI_FORMAT_R32G32_FLOAT,       1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
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
	bufferDesc.ByteWidth = sizeof(DX11TEXTUREDSPRITEINSTANCE) * SConst::MaxInstancedSpritesCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(d3dDevice->CreateBuffer(&bufferDesc, NULL, instanceBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create vertex buffer");
	}

	S_CATCH{ S_THROW("STexturedSpriteRenderSystemDX11::Setup()") }
}

void STexturedSpriteRenderSystemDX11::Render(float deltaSeconds, float gameTime)
{
	S_TRY

	auto deviceContext = renderSystemDX11.GetDeviceContext();
	auto shader = renderSystemDX11.FindShader(shaderName);
	auto world = renderSystemDX11.GetWorld();
	if (!deviceContext || !shader || !world || !spriteVertexBuffer || !spriteIndexBuffer || !instanceBuffer)
	{
		if (renderSystemDX11.IsNeedDebugTrace())
		{
			DebugMsg("[%s] STexturedSpriteRenderSystemDX11::Render(): wrong context\n",
				GetTimeStamp(std::chrono::system_clock::now()).c_str());
		}
		return;
	}

	// setup context
	ID3D11Buffer* buffers[2] = { spriteVertexBuffer, instanceBuffer.Get() };
	static UINT strides[2] = { sizeof(DX11SPRITEVERTEX), sizeof(DX11TEXTUREDSPRITEINSTANCE) };
	static UINT offsets[2] = { 0, 0 };
	deviceContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	deviceContext->IASetIndexBuffer(spriteIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	deviceContext->IASetInputLayout(shader->layout.Get());
	deviceContext->VSSetShader(shader->vs.Get(), NULL, 0);
	deviceContext->PSSetShader(shader->ps.Get(), NULL, 0);

	// prepare cached state
	if (batchData.capacity() < SConst::MaxInstancedSpritesCount) batchData.reserve(SConst::MaxInstancedSpritesCount);
	cachedTexView = nullptr;
	numSprites = 0;
	batchesRendered = 0;
	cachedTexId = 0;

	// render sprites
	const auto& registry = world->GetEntities();
	const auto& spritesView = registry.view<
		const STexturedComponent,
		const SColoredComponent,
		const SSpriteComponent,
		const SSpriteUVComponent>(entt::exclude<SSpriteFrameAnimComponent>);
	spritesView.each([this](
		const STexturedComponent& texturedComponent,
		const SColoredComponent& coloredComponent,
		const SSpriteComponent& spriteComponent,
		const SSpriteUVComponent& uvComponent)
	{
		if (!spriteComponent.bVisible) return;
		if (!cachedTexView)
		{
			auto [view, texSize] = renderSystemDX11.FindTexture(texturedComponent.id);
			cachedTexId = texturedComponent.id;
			cachedTexView = view;
		}

		if (cachedTexId != texturedComponent.id)
		{
			if (!batchData.empty())
			{
				// render if texture changed
				RenderBatch();
			}

			auto [view, texSize] = renderSystemDX11.FindTexture(texturedComponent.id);
			cachedTexId = texturedComponent.id;
			cachedTexView = view;
		}

		// store instance data
		DX11TEXTUREDSPRITEINSTANCE instance{};
		instance.position = spriteComponent.position;
		instance.rotation = spriteComponent.rotation;
		instance.scale = SConvert::ToVector2(spriteComponent.size);
		memcpy(instance.colors, coloredComponent.colors, sizeof(SColor4F) * 4);
		memcpy(instance.uvs, uvComponent.uvs.uvs, sizeof(SVector2) * 4);
		batchData.push_back(instance);

		if (batchData.size() == SConst::MaxInstancedSpritesCount)
		{
			// render if max number reached
			RenderBatch();
		}

		numSprites++;
	});

	if (!batchData.empty())
	{
		// render last
		RenderBatch();
	}

	if (renderSystemDX11.IsNeedDebugTrace())
	{
		renderSystemDX11.AddDrawCalls(batchesRendered);
		DebugMsg("[%s] STexturedSpriteRenderSystemDX11::Render(): %d batches, %d sprite instances\n",
			GetTimeStamp(std::chrono::system_clock::now()).c_str(), batchesRendered, numSprites);
	}

	S_CATCH{ S_THROW("STexturedSpriteRenderSystemDX11::Render()") }
}

void STexturedSpriteRenderSystemDX11::RenderBatch()
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
		memcpy(mappedResource.pData, batchData.data(), sizeof(DX11TEXTUREDSPRITEINSTANCE) * numInstances);
		deviceContext->Unmap(instanceBuffer.Get(), 0);

		// render sprites
		deviceContext->PSSetShaderResources(0, 1, &cachedTexView);
		deviceContext->DrawIndexedInstanced(4, numInstances, 0, 0, 0);
		batchesRendered++;
	}

	batchData.clear();
}
