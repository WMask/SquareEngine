/***************************************************************************
* SConstantBuffersDX11.cpp
*/

#include "RenderSystem/DX11/SConstantBuffersDX11.h"
#include "RenderSystem/SRenderSystemInterface.h"
#include "Core/SException.h"
#include "Core/STypes.h"
#include "Core/SMath.h"
#include <directxmath.h>


struct SWVPBuffer
{
	SMatrix4 mTrans;
	SMatrix4 mView;
	SMatrix4 mProj;
};

SConstantBuffersDX11::~SConstantBuffersDX11()
{
	Shutdown();
}

void SConstantBuffersDX11::Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext,
	const SCamera& camera, std::uint32_t width, std::uint32_t height)
{
	S_TRY

	D3D11_BUFFER_DESC cbDesc{};
	cbDesc.ByteWidth = Align16<SWVPBuffer>();
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	D3D11_SUBRESOURCE_DATA subResData{};

	// create constant buffers
	SWVPBuffer wvpData;
	wvpData.mProj = SMath::OrthoMatrix(SSize2{ width, height }, 1.0f, 0.0f);
	wvpData.mView = SMath::LookAtMatrix(
		camera.GetPosition(SCameraSpace::Camera2D),
		camera.GetTarget(SCameraSpace::Camera2D), true);
	wvpData.mTrans = SConst::IdentitySMatrix4;
	subResData.pSysMem = &wvpData;
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, wvpMatrixBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	SSettingsBuffer settingsData;
	cbDesc.ByteWidth = Align16<SSettingsBuffer>();
	settingsData.worldTint = SConvert::ToVector4(SConst::White3);
	subResData.pSysMem = &settingsData;
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, settingsBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	SLightsBuffer lightsData;
	cbDesc.ByteWidth = Align16<SLightsBuffer>();
	lightsData.numLights = 0u;
	subResData.pSysMem = &lightsData;
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, lightsBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	// set buffers
	d3dDeviceContext->VSSetConstantBuffers(0, 1, wvpMatrixBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(1, 1, settingsBuffer.GetAddressOf());
	d3dDeviceContext->PSSetConstantBuffers(1, 1, settingsBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(2, 1, lightsBuffer.GetAddressOf());

	// create vertex buffer
	DX11SPRITEVERTEX data[] = {
		DX11SPRITEVERTEX{ SVector3{ 0.5f,-0.5f, 0.0f } }, // rt
		DX11SPRITEVERTEX{ SVector3{-0.5f,-0.5f, 0.0f } }, // lt
		DX11SPRITEVERTEX{ SVector3{ 0.5f, 0.5f, 0.0f } }, // rb
		DX11SPRITEVERTEX{ SVector3{-0.5f, 0.5f, 0.0f } }  // lb
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
	std::uint16_t indexData[] = {0, 1, 2, 3};
	bufferDesc.ByteWidth = sizeof(indexData);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferData.pSysMem = indexData;

	if (FAILED(d3dDevice->CreateBuffer(&bufferDesc, &bufferData, spriteIndexBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create index buffer");
	}

	S_CATCH{ S_THROW("SConstantBuffersDX11::Init()") }
}

void SConstantBuffersDX11::ApplyTransform2D(ID3D11DeviceContext* d3dDeviceContext,
	const SCamera& camera, SVector2 worldScale, std::uint32_t width, std::uint32_t height)
{
	if (d3dDeviceContext)
	{
		SWVPBuffer wvpData;
		wvpData.mProj = SMath::OrthoMatrix(SSize2{ width, height }, 1.0f, 0.0f);
		wvpData.mView = SMath::LookAtMatrix(
			camera.GetPosition(SCameraSpace::Camera2D),
			camera.GetTarget(SCameraSpace::Camera2D), true);
		wvpData.mTrans = SMath::ScaleMatrix2(worldScale);
		d3dDeviceContext->UpdateSubresource(wvpMatrixBuffer.Get(), 0, NULL, &wvpData, 0, 0);
	}
}

void SConstantBuffersDX11::ApplyTransform3D(ID3D11DeviceContext* d3dDeviceContext, const SCamera& camera, std::uint32_t width, std::uint32_t height)
{
	if (d3dDeviceContext)
	{
		SWVPBuffer wvpData;
		const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		wvpData.mProj = SMath::ProjectionMatrix(camera.GetFOV(), aspectRatio, 0.1f, 10000.0f);
		wvpData.mView = SMath::LookAtMatrix(
			camera.GetPosition(SCameraSpace::Camera3D), camera.GetTarget(SCameraSpace::Camera3D));
		wvpData.mTrans = SMath::ScaleMatrix3(SConst::OneSVector3);
		d3dDeviceContext->UpdateSubresource(wvpMatrixBuffer.Get(), 0, NULL, &wvpData, 0, 0);
	}
}

void SConstantBuffersDX11::Shutdown()
{
	wvpMatrixBuffer.Reset();
	spriteIndexBuffer.Reset();
	spriteVertexBuffer.Reset();
}
