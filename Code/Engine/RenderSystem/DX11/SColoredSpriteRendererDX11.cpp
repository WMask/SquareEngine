/***************************************************************************
* SColoredSpriteRendererDX11.cpp
*/

#include "RenderSystem/DX11/SColoredSpriteRendererDX11.h"
#include "RenderSystem/DX11/SRenderSystemDX11.h"
#include "RenderSystem/SECSComponents.h"
#include "Core/SException.h"

#ifdef min
# undef min
#endif

#include <algorithm>


static const char* ColoredSpriteShaderName = "ColoredSprite2d.shader";

struct DX11COLOREDSPRITE
{
	SVector3 pos;
	std::uint32_t index;
};

struct DX11COLOREDSPRITEINSTANCE
{
	SVector3 pos;
	SColor4F colors[4];
};

SColoredSpriteRendererDX11::~SColoredSpriteRendererDX11()
{
	vertexBuffer.Reset();
	instanceBuffer.Reset();
}

bool SColoredSpriteRendererDX11::CheckShaderName(const std::string& inShaderName)
{
	bool bResult = (inShaderName.find(ColoredSpriteShaderName) != inShaderName.npos);
	if (bResult)
	{
		shaderName = inShaderName;
	}

	return bResult;
}

void SColoredSpriteRendererDX11::Setup(IRenderSystem& renderSystem, IVisualRenderer::SShaderData& shaderData)
{
	S_TRY

	auto& shaderDataDX11 = static_cast<SShaderDataDX11&>(shaderData);
	auto& renderSystemDX11 = static_cast<SRenderSystemDX11&>(renderSystem);
	auto d3dDeviceContext = renderSystemDX11.GetD3D11DeviceContext();
	auto d3dDevice = renderSystemDX11.GetD3D11Device();
	if (!d3dDevice || !d3dDeviceContext)
	{
		throw std::exception("Invalid render device");
	}

	// create input layouts
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "INSTANCEID",  0, DXGI_FORMAT_R32_UINT,           0, 12, D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT,    1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "COLOR",       1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "COLOR",       2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 44, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "COLOR",       3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 60, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	if (FAILED(d3dDevice->CreateInputLayout(layout, 7, shaderDataDX11.vsCode->GetBufferPointer(), shaderDataDX11.vsCode->GetBufferSize(), shaderDataDX11.layout.GetAddressOf())))
	{
		throw std::exception("Cannot create input layout");
	}

	// create vertex buffer
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(DX11COLOREDSPRITE) * 6;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	DX11COLOREDSPRITE data[] = {
		DX11COLOREDSPRITE{ SVector3{ 50.5f, 50.5f, 0.0f }, 1 },
		DX11COLOREDSPRITE{ SVector3{ 50.5f,-50.5f, 0.0f }, 2 },
		DX11COLOREDSPRITE{ SVector3{-50.5f, 50.5f, 0.0f }, 0 },
		DX11COLOREDSPRITE{ SVector3{-50.5f, 50.5f, 0.0f }, 0 },
		DX11COLOREDSPRITE{ SVector3{ 50.5f,-50.5f, 0.0f }, 2 },
		DX11COLOREDSPRITE{ SVector3{-50.5f,-50.5f, 0.0f }, 3 }
	};
	D3D11_SUBRESOURCE_DATA vertexData{};
	vertexData.pSysMem = data;

	if (FAILED(d3dDevice->CreateBuffer(&bufferDesc, &vertexData, vertexBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create vertex buffer");
	}

	// create instance buffer
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(DX11COLOREDSPRITEINSTANCE) * MaxInstancedSpritesCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(d3dDevice->CreateBuffer(&bufferDesc, NULL, instanceBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create vertex buffer");
	}

	S_CATCH{ S_THROW("SColoredSpriteRendererDX11::Setup()") }
}

void SColoredSpriteRendererDX11::Render(IRenderSystem& renderSystem)
{
	S_TRY

	auto renderSystemDX11 = static_cast<SRenderSystemDX11&>(renderSystem);
	auto d3dDeviceContext = renderSystemDX11.GetD3D11DeviceContext();
	auto d3dDevice = renderSystemDX11.GetD3D11Device();
	auto shader = renderSystemDX11.FindShader(shaderName);
	auto world = renderSystemDX11.GetWorld();
	if (!d3dDevice || !d3dDeviceContext || !shader || !world)
	{
		throw std::exception("Invalid render params");
	}

	auto spritesView = world->GetEntities().view<SColoredSpriteComponent>();
	if (spritesView.empty())
	{
		return;
	}

	std::uint32_t numSprites = spritesView.size();
	std::uint32_t spriteId = 0;
	std::vector<DX11COLOREDSPRITEINSTANCE> instanceData;
	instanceData.reserve(MaxInstancedSpritesCount);

	for (auto spriteEntity : spritesView)
	{
		auto& spriteComponent = spritesView.get<SColoredSpriteComponent>(spriteEntity);

		// store instance data
		DX11COLOREDSPRITEINSTANCE instance{};
		instance.pos = spriteComponent.position;
		memcpy(instance.colors, spriteComponent.colors, sizeof(SColor4F) * 4);
		instanceData.push_back(instance);

		const bool bTimeToRenderBatch = (
			instanceData.size() == MaxInstancedSpritesCount ||
			instanceData.size() == numSprites ||
			spriteId == (numSprites - 1)
		);

		if (bTimeToRenderBatch)
		{
			// fill instanced vertex buffer
			D3D11_MAPPED_SUBRESOURCE mappedResource{};
			if (FAILED(d3dDeviceContext->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			{
				throw std::exception("Cannot update vertex buffer");
			}
			memcpy(mappedResource.pData, instanceData.data(), sizeof(DX11COLOREDSPRITEINSTANCE) * instanceData.size());
			d3dDeviceContext->Unmap(instanceBuffer.Get(), 0);

			// setup context
			ID3D11Buffer* buffers[2] = { vertexBuffer.Get(), instanceBuffer.Get() };
			UINT strides[2] = { sizeof(DX11COLOREDSPRITE), sizeof(DX11COLOREDSPRITEINSTANCE) };
			UINT offsets[2] = { 0, 0 };
			d3dDeviceContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
			d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			d3dDeviceContext->IASetInputLayout(shader->layout.Get());
			d3dDeviceContext->VSSetShader(shader->vs.Get(), NULL, 0);
			d3dDeviceContext->PSSetShader(shader->ps.Get(), NULL, 0);

			// render sprites
			d3dDeviceContext->DrawInstanced(6, instanceData.size(), 0, 0);

			// cleanup batch data
			instanceData.clear();
		}

		spriteId++;
	}

	S_CATCH{ S_THROW("SColoredSpriteRendererDX11::Render()") }
}
