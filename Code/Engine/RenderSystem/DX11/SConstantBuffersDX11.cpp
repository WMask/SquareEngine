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

void SConstantBuffersDX11::Shutdown()
{
	wvpMatrixBuffer.Reset();
	spriteIndexBuffer.Reset();
	spriteVertexBuffer.Reset();
	defaultTextureView.Reset();
	defaultTexture.Reset();
	settingsBuffer.Reset();
	lightsBuffer.Reset();
	meshBuffer.Reset();
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

	SSettingsBuffer settingsData{};
	cbDesc.ByteWidth = Align16<SSettingsBuffer>();
	settingsData.worldTint = SConvert::ToVector4(SConst::White3);
	settingsData.cameraPos = SConvert::ToVector4(camera.GetPosition(SCameraSpace::Camera3D));
	settingsData.viewDir = SConvert::ToVector4(camera.GetViewDir());
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

	SMaterialBuffer materialData{};
	cbDesc.ByteWidth = Align16<SMaterialBuffer>();
	lightsData.numLights = 0u;
	subResData.pSysMem = &materialData;
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, meshBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	// set buffers
	d3dDeviceContext->VSSetConstantBuffers(0, 1, wvpMatrixBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(1, 1, settingsBuffer.GetAddressOf());
	d3dDeviceContext->PSSetConstantBuffers(1, 1, settingsBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(2, 1, meshBuffer.GetAddressOf());
	d3dDeviceContext->PSSetConstantBuffers(2, 1, meshBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(3, 1, lightsBuffer.GetAddressOf());

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

	// create default texture
	const std::uint32_t defaultTextureSize = 64;
	const std::uint32_t defaultTextureGrid = 8;
	std::vector<std::uint32_t> pixels;
	pixels.resize(defaultTextureSize * defaultTextureSize);

	for (std::uint32_t y = 0u; y < defaultTextureSize; y++)
	{
		bool bSwap = (y / defaultTextureGrid) % 2;
		for (std::uint32_t x = 0u; x < defaultTextureSize; x++)
		{
			std::uint32_t& pixel = pixels[y * defaultTextureSize + x];
			bool bIsBlack = ((x / defaultTextureGrid + y / defaultTextureGrid) % 2) == 0;
			pixel = bIsBlack ? 0xFF000000 : 0xFFFFFFFF;
		}
	}

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = defaultTextureSize;
	desc.Height = defaultTextureSize;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = pixels.data();
	initData.SysMemPitch = defaultTextureSize * 4;

	if (FAILED(d3dDevice->CreateTexture2D(&desc, &initData, defaultTexture.GetAddressOf())))
	{
		throw std::exception("Cannot create default texture");
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	if (FAILED(d3dDevice->CreateShaderResourceView(defaultTexture.Get(), &srvDesc, defaultTextureView.GetAddressOf())))
	{
		throw std::exception("Cannot create default texture view");
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

void SConstantBuffersDX11::UpdateSettingsBuffer(ID3D11DeviceContext* d3dDeviceContext,
	const SCamera& camera, const std::optional<SColor3>& globalTint, float envCubemapAmount)
{
	if (settingsBuffer)
	{
		SSettingsBuffer settings{};
		settings.cameraPos = SConvert::ToVector4(camera.GetPosition(SCameraSpace::Camera3D));
		settings.viewDir = SConvert::ToVector4(camera.GetViewDir());

		if (globalTint.has_value())
			settings.worldTint = SConvert::ToVector4(globalTint.value());
		else
			settings.worldTint = SConst::OneSVector4;

		if (envCubemapAmount >= 0.0f)
		{
			settings.bHasEnvCubemap = TRUE;
			settings.envCubemapAmount = envCubemapAmount;
		}
		else
		{
			settings.bHasEnvCubemap = FALSE;
			settings.envCubemapAmount = envCubemapAmount;
		}

		d3dDeviceContext->UpdateSubresource(settingsBuffer.Get(), 0, NULL, &settings, 0, 0);
	}
}
