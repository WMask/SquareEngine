/***************************************************************************
* SConstantBuffersDX11.cpp
*/

#include "RenderSystem/DX11/SConstantBuffersDX11.h"
#include "RenderSystem/DX11/SRenderSystemTypesDX11.h"
#include "Core/SException.h"
#include "Core/STypes.h"
#include "Core/SMath.h"
#include <directxmath.h>


struct SWVPBuffer
{
	SMatrix4 mWorld;
	SMatrix4 mView;
	SMatrix4 mProj;
	DirectX::XMVECTOR mWorldInverse[3];
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
	cubemapsBuffer.Reset();
	lightsBuffer.Reset();
	materialBuffer.Reset();
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
	wvpData.mWorld = SConst::IdentitySMatrix4;
	subResData.pSysMem = &wvpData;
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, wvpMatrixBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	SSettingsBuffer settingsData{};
	cbDesc.ByteWidth = Align16<SSettingsBuffer>();
	settingsData.cameraPos = SConvert::ToVector4(camera.GetPosition(SCameraSpace::Camera3D));
	settingsData.viewDir = SConvert::ToVector4(camera.GetViewDir());
	subResData.pSysMem = &settingsData;
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, settingsBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	SCubemapsBuffer cubemapsData{};
	cbDesc.ByteWidth = Align16<SCubemapsBuffer>();
	cubemapsData.bHasDiffuseCubemap = FALSE;
	cubemapsData.bHasSpecularCubemap = FALSE;
	cubemapsData.diffuseAmount = 1.0f;
	cubemapsData.specularAmount = 1.0f;
	subResData.pSysMem = &cubemapsData;
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, cubemapsBuffer.GetAddressOf())))
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

	SMaterialFlagsBuffer materialData{};
	cbDesc.ByteWidth = Align16<SMaterialFlagsBuffer>();
	subResData.pSysMem = &materialData;
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, materialBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	// set buffers
	d3dDeviceContext->VSSetConstantBuffers(0, 1, wvpMatrixBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(1, 1, settingsBuffer.GetAddressOf());
	d3dDeviceContext->PSSetConstantBuffers(1, 1, settingsBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(2, 1, materialBuffer.GetAddressOf());
	d3dDeviceContext->PSSetConstantBuffers(2, 1, materialBuffer.GetAddressOf());
	d3dDeviceContext->PSSetConstantBuffers(3, 1, cubemapsBuffer.GetAddressOf());
	d3dDeviceContext->PSSetConstantBuffers(4, 1, lightsBuffer.GetAddressOf());

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
	desc.Usage = D3D11_USAGE_IMMUTABLE;
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
		SMatrix4 mWorld = SMath::ScaleMatrix2(worldScale);
		SMatrix4 mProj = SMath::OrthoMatrix(SSize2{ width, height }, 1.0f, 0.0f);
		SMatrix4 mInvertY = SMath::ScaleMatrix2({ 1.0f, -1.0f });
		SMatrix4 mView = SMath::LookAtMatrix(
			camera.GetPosition(SCameraSpace::Camera2D),
			camera.GetTarget(SCameraSpace::Camera2D));
		SWVPBuffer wvpData{ SMath::TransposeM4(mWorld), SMath::TransposeM4(mView), SMath::TransposeM4(mProj) * mInvertY };
		d3dDeviceContext->UpdateSubresource(wvpMatrixBuffer.Get(), 0, NULL, &wvpData, 0, 0);
	}
}

void SConstantBuffersDX11::ApplyTransform3D(ID3D11DeviceContext* d3dDeviceContext, const SCamera& camera,
	std::uint32_t width, std::uint32_t height, float gameTime)
{
	if (d3dDeviceContext)
	{
		SMatrix4 mWorld = SMath::RotationMatrixY(cosf(gameTime) * 0.0f);
		const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		SMatrix4 mProj = SMath::ProjectionMatrix(camera.GetFOV(), aspectRatio, 0.1f, 10000.0f);
		SMatrix4 mView = SMath::LookAtMatrix(
			camera.GetPosition(SCameraSpace::Camera3D), camera.GetTarget(SCameraSpace::Camera3D));
		SWVPBuffer wvpData{ SMath::TransposeM4(mWorld), SMath::TransposeM4(mView), SMath::TransposeM4(mProj) };
		const DirectX::XMMATRIX worldInverse = SConvert::ToXMatrix(SMath::InverseM4(mWorld));
		wvpData.mWorldInverse[0] = worldInverse.r[0];
		wvpData.mWorldInverse[1] = worldInverse.r[1];
		wvpData.mWorldInverse[2] = worldInverse.r[2];
		d3dDeviceContext->UpdateSubresource(wvpMatrixBuffer.Get(), 0, NULL, &wvpData, 0, 0);
	}
}

void SConstantBuffersDX11::UpdateSettingsBuffer(const IRenderSystemDX11& renderSystem, const SCamera& camera,
	const SColor4F& inGlobalTint, const SColor4F& inBackLight, const SColor4F& inPbrGammaCorrection)
{
	auto deviceContext = renderSystem.GetDeviceContext();
	if (deviceContext && settingsBuffer)
	{
		SSettingsBuffer settings{};
		settings.cameraPos = SConvert::ToVector4(camera.GetPosition(SCameraSpace::Camera3D));
		settings.viewDir = SConvert::ToVector4(camera.GetViewDir());
		settings.globalTint = inGlobalTint;
		settings.backLight = inBackLight;
		settings.pbrGammaCorrection = inPbrGammaCorrection;

		deviceContext->UpdateSubresource(settingsBuffer.Get(), 0, NULL, &settings, 0, 0);
	}
}

void SConstantBuffersDX11::UpdateMaterialFlags(const IRenderSystemDX11& renderSystem, const SMaterialFlagsBuffer& materials)
{
	auto deviceContext = renderSystem.GetDeviceContext();
	if (deviceContext && materialBuffer)
	{
		deviceContext->UpdateSubresource(materialBuffer.Get(), 0, NULL, &materials, 0, 0);
	}
}

void SConstantBuffersDX11::UpdateLightSettings(const IRenderSystemDX11& renderSystem, const SLightsBuffer& lights)
{
	auto deviceContext = renderSystem.GetDeviceContext();
	if (deviceContext && lightsBuffer)
	{
		deviceContext->UpdateSubresource(lightsBuffer.Get(), 0, NULL, &lights, 0, 0);
	}
}

void SConstantBuffersDX11::UpdateCubemapSettings(const IRenderSystemDX11& renderSystem)
{
	auto deviceContext = renderSystem.GetDeviceContext();
	if (deviceContext && cubemapsBuffer)
	{
		SCubemapsBuffer cubemaps{};
		cubemaps.bHasDiffuseCubemap = renderSystem.FindCubemap(ECubemapType::Diffuse) ? TRUE : FALSE;
		cubemaps.bHasSpecularCubemap = renderSystem.FindCubemap(ECubemapType::Specular) ? TRUE : FALSE;
		cubemaps.diffuseAmount = renderSystem.GetCubemapAmount(ECubemapType::Diffuse);
		cubemaps.specularAmount = renderSystem.GetCubemapAmount(ECubemapType::Specular);
		cubemaps.maxCubemapMipLevels = renderSystem.GetCubemapMaxMipLevel(ECubemapType::Specular);

		deviceContext->UpdateSubresource(cubemapsBuffer.Get(), 0, NULL, &cubemaps, 0, 0);
	}
}
