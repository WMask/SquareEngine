/***************************************************************************
* SFXAARenderSystemDX11.cpp
*/

#include "RenderSystem/DX11/SFXAARenderSystemDX11.h"
#include "RenderSystem/DX11/SRenderSystemTypesDX11.h"
#include "RenderSystem/DX11/SConstantBuffersDX11.h"
#include "RenderSystem/SECSComponents.h"
#include "Core/SException.h"


static const char* FXAAShaderName = "RenderTargetFXAA.shader";

SFXAARenderSystemDX11::SFXAARenderSystemDX11(IRenderSystemDX11& renderSystem) : renderSystemDX11(renderSystem)
{
}

SFXAARenderSystemDX11::~SFXAARenderSystemDX11()
{
	Shutdown();
}

void SFXAARenderSystemDX11::Shutdown()
{
	spriteVertexBuffer.Reset();
	spriteIndexBuffer.Reset();
}

bool SFXAARenderSystemDX11::CheckShaderName(const std::string& inShaderName)
{
	if (inShaderName.ends_with(FXAAShaderName))
	{
		shaderName = inShaderName;
		return true;
	}

	return false;
}

void SFXAARenderSystemDX11::Setup(IRenderSystem::SShaderData& shaderData)
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
		{ "SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD"   , 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if (FAILED(d3dDevice->CreateInputLayout(layout, std::size(layout),
		shaderDataDX11.vsCode->GetBufferPointer(),
		shaderDataDX11.vsCode->GetBufferSize(),
		shaderDataDX11.layout.GetAddressOf())))
	{
		throw std::exception("Cannot create input layout");
	}

	// create vertex buffer
	DX11RENDERTARGETVERTEX data[] = {
		DX11RENDERTARGETVERTEX{ SVector3{ 1.0f,-1.0f, 0.0f }, SVector2{ 1.0f, 1.0f} }, // rt
		DX11RENDERTARGETVERTEX{ SVector3{ 1.0f, 1.0f, 0.0f }, SVector2{ 1.0f, 0.0f} }, // rb
		DX11RENDERTARGETVERTEX{ SVector3{-1.0f,-1.0f, 0.0f }, SVector2{ 0.0f, 1.0f} }, // lt
		DX11RENDERTARGETVERTEX{ SVector3{-1.0f, 1.0f, 0.0f }, SVector2{ 0.0f, 0.0f} }  // lb
	};
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(data);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA bufferData{};
	bufferData.pSysMem = data;

	if (FAILED(d3dDevice->CreateBuffer(&bufferDesc, &bufferData, spriteVertexBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create vertex buffer");
	}

	// create index buffer
	std::uint16_t indexData[] = { 0, 1, 2, 3 };
	bufferDesc.ByteWidth = sizeof(indexData);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferData.pSysMem = indexData;

	if (FAILED(d3dDevice->CreateBuffer(&bufferDesc, &bufferData, spriteIndexBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create index buffer");
	}

	S_CATCH{ S_THROW("SFXAARenderSystemDX11::Setup()") }
}

void SFXAARenderSystemDX11::Render(ID3D11ShaderResourceView* sceneSRV, const SSize2& sceneSize)
{
	auto shader = renderSystemDX11.FindShader(shaderName);
	auto deviceContext = renderSystemDX11.GetDeviceContext();
	if (deviceContext && shader)
	{
		// setup context
		ID3D11Buffer* buffers[] = { spriteVertexBuffer.Get()};
		static UINT stride = sizeof(DX11RENDERTARGETVERTEX);
		static UINT offset = 0u;
		deviceContext->IASetVertexBuffers(0, 1, buffers, &stride, &offset);
		deviceContext->IASetIndexBuffer(spriteIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		deviceContext->IASetInputLayout(shader->layout.Get());
		deviceContext->VSSetShader(shader->vs.Get(), NULL, 0);
		deviceContext->PSSetShader(shader->ps.Get(), NULL, 0);

		// render sprites
		deviceContext->PSSetShaderResources(0, 1, &sceneSRV);
		deviceContext->DrawIndexed(4, 0, 0);
	}
}
