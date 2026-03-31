/***************************************************************************
* SColoredSpriteRenderSystemDX11.cpp
*/

#include "RenderSystem/DX11/SColoredSpriteRenderSystemDX11.h"
#include "RenderSystem/DX11/SRenderSystemDX11.h"
#include "RenderSystem/SECSComponents.h"
#include "Core/SException.h"

static const char* ColoredSpriteShaderName = "ColoredSprite2d.shader";


SColoredSpriteRenderSystemDX11::SColoredSpriteRenderSystemDX11(SRenderSystemDX11& renderSystem) : renderSystemDX11(renderSystem)
{
}

SColoredSpriteRenderSystemDX11::~SColoredSpriteRenderSystemDX11()
{
	Shutdown();
}

void SColoredSpriteRenderSystemDX11::Shutdown()
{
	if (instanceBuffer)
	{
		instanceBuffer.Reset();
	}

	d3dDeviceContext = nullptr;
	spriteVertexBuffer = nullptr;
	spriteIndexBuffer = nullptr;
}

bool SColoredSpriteRenderSystemDX11::CheckShaderName(const std::string& inShaderName)
{
	if (inShaderName.ends_with(ColoredSpriteShaderName))
	{
		shaderName = inShaderName;
		return true;
	}

	return false;
}

void SColoredSpriteRenderSystemDX11::Setup(IRenderSystem::SShaderData& shaderData)
{
	S_TRY

	d3dDeviceContext = renderSystemDX11.GetD3D11DeviceContext();
	spriteVertexBuffer = renderSystemDX11.GetConstantBuffers().spriteVertexBuffer.Get();
	spriteIndexBuffer = renderSystemDX11.GetConstantBuffers().spriteIndexBuffer.Get();
	auto& shaderDataDX11 = static_cast<SShaderDataDX11&>(shaderData);
	auto d3dDevice = renderSystemDX11.GetD3D11Device();
	if (!d3dDevice)
	{
		throw std::exception("Invalid render device");
	}

	// create input layouts
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "INSTANCEPOS",   0, DXGI_FORMAT_R32G32B32_FLOAT,    1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCEROT",   0, DXGI_FORMAT_R32_FLOAT,          1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCESCALE", 0, DXGI_FORMAT_R32G32_FLOAT,       1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCECOLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCECOLOR", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 40, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCECOLOR", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 56, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCECOLOR", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 72, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
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
	bufferDesc.ByteWidth = sizeof(DX11COLOREDSPRITEINSTANCE) * MaxInstancedSpritesCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(d3dDevice->CreateBuffer(&bufferDesc, NULL, instanceBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create vertex buffer");
	}

	S_CATCH{ S_THROW("SColoredSpriteRenderSystemDX11::Setup()") }
}

void SColoredSpriteRenderSystemDX11::Render(float deltaSeconds, float gameTime)
{
	S_TRY

	auto shader = renderSystemDX11.FindShader(shaderName);
	auto world = renderSystemDX11.GetWorld();
	if (!shader || !world || !d3dDeviceContext || !spriteVertexBuffer || !spriteIndexBuffer || !instanceBuffer)
	{
		if (renderSystemDX11.IsNeedDebugTrace())
		{
			DebugMsg("[%s] SColoredSpriteRenderSystemDX11::Render(): wrong context\n",
				GetTimeStamp(std::chrono::system_clock::now()).c_str());
		}
		return;
	}

	// setup context
	ID3D11Buffer* buffers[2] = { spriteVertexBuffer, instanceBuffer.Get() };
	static UINT strides[2] = { sizeof(DX11SPRITEVERTEX), sizeof(DX11COLOREDSPRITEINSTANCE) };
	static UINT offsets[2] = { 0, 0 };
	d3dDeviceContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	d3dDeviceContext->IASetIndexBuffer(spriteIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	d3dDeviceContext->IASetInputLayout(shader->layout.Get());
	d3dDeviceContext->VSSetShader(shader->vs.Get(), NULL, 0);
	d3dDeviceContext->PSSetShader(shader->ps.Get(), NULL, 0);

	// prepare cached state
	if (batchData.capacity() < MaxInstancedSpritesCount) batchData.reserve(MaxInstancedSpritesCount);
	batchesRendered = 0;
	numSprites = 0;

	const auto& spritesView = world->GetEntities().view<
		const SColoredComponent,
		const SSpriteComponent>(entt::exclude<STexturedComponent>);
	spritesView.each([this](
		const SColoredComponent& coloredComponent,
		const SSpriteComponent& spriteComponent)
	{
		if (!spriteComponent.bVisible) return;

		// store instance data
		DX11COLOREDSPRITEINSTANCE instance{};
		instance.pos = spriteComponent.position;
		instance.rotation = spriteComponent.rotation;
		instance.scale = SConvert::ToVector2(spriteComponent.size);
		memcpy(instance.colors, coloredComponent.colors, sizeof(SColor4F) * 4);
		batchData.push_back(instance);

		if (batchData.size() == MaxInstancedSpritesCount)
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
		DebugMsg("[%s] SColoredSpriteRenderSystemDX11::Render(): %d batches, %d sprite instances\n",
			GetTimeStamp(std::chrono::system_clock::now()).c_str(), batchesRendered, numSprites);
	}

	S_CATCH{ S_THROW("SColoredSpriteRenderSystemDX11::Render()") }
}

void SColoredSpriteRenderSystemDX11::RenderBatch()
{
	// fill instanced vertex buffer
	const std::uint32_t numInstances = batchData.size();
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	if (FAILED(d3dDeviceContext->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		throw std::exception("Cannot update vertex buffer");
	}
	memcpy(mappedResource.pData, batchData.data(), sizeof(DX11COLOREDSPRITEINSTANCE) * numInstances);
	d3dDeviceContext->Unmap(instanceBuffer.Get(), 0);

	// render sprites
	d3dDeviceContext->DrawIndexedInstanced(4, numInstances, 0, 0, 0);

	// cleanup batch data
	batchData.clear();
	batchesRendered++;
}
