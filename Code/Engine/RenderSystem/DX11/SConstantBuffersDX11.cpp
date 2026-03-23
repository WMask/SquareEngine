/***************************************************************************
* SConstantBuffersDX11.cpp
*/

#include "RenderSystem/DX11/SConstantBuffersDX11.h"
#include "RenderSystem/SRenderSystemInterface.h"
#include "Core/SException.h"
#include "Core/STypes.h"
#include "Core/SMath.h"


void SConstantBuffersDX11::Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext,
	SVector3 cameraPos, SVector3 cameraTarget, std::uint32_t width, std::uint32_t height)
{
	S_TRY

	struct VS_MATRIX_BUFFER
	{
		SMatrix4 mat;
	};
	VS_MATRIX_BUFFER matData;

	struct VS_COLORS_BUFFER
	{
		SColor4F colors[4];
	};
	VS_COLORS_BUFFER colorsData;

	struct VS_CUSTOM_UV_BUFFER
	{
		SVector4 uv[4];
	};
	VS_CUSTOM_UV_BUFFER uvData;

	struct VS_FRAME_ANIM2D_BUFFER
	{
		SVector4 animData;
	};
	VS_FRAME_ANIM2D_BUFFER anim2dData;

	D3D11_BUFFER_DESC cbDesc{};
	cbDesc.ByteWidth = sizeof(VS_MATRIX_BUFFER);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	D3D11_SUBRESOURCE_DATA subResData{};
	subResData.pSysMem = &matData;

	SSettingsBuffer settingsData{};
	SSpriteFlagsBuffer flagsData{};

	// create constant buffers
	matData.mat = SMath::OrthoMatrix(SSize2{ width, height }, 1.0f, -1.0f);
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, projMatrixBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	matData.mat = SMath::LookAtMatrix(cameraPos, cameraTarget);
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, viewMatrixBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	SVector3 spritePos{ 0.0f, 0.0f, 0.0f };
	SVector2 spriteSize{ 1.0f, 1.0f };
	matData.mat = SMath::TransformMatrix(spritePos, spriteSize, 0.0f);
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, transMatrixBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	subResData.pSysMem = &colorsData;
	colorsData.colors[0] = SConst::OneSColor4F;
	colorsData.colors[1] = SConst::OneSColor4F;
	colorsData.colors[2] = SConst::OneSColor4F;
	colorsData.colors[3] = SConst::OneSColor4F;
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, colorsBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	subResData.pSysMem = &uvData;
	uvData.uv[1] = SVector4{ 1.0, 0.0, 0.0f, 0.0f };
	uvData.uv[2] = SVector4{ 1.0, 1.0, 0.0f, 0.0f };
	uvData.uv[0] = SVector4{ 0.0, 0.0, 0.0f, 0.0f };
	uvData.uv[3] = SVector4{ 0.0, 1.0, 0.0f, 0.0f };
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, customUvBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	cbDesc.ByteWidth = sizeof(VS_FRAME_ANIM2D_BUFFER);
	subResData.pSysMem = &anim2dData;
	anim2dData.animData = SVector4{ 1.0, 1.0, 0.0, 0.0 };
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, frameAnimBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	cbDesc.ByteWidth = sizeof(SSettingsBuffer);
	subResData.pSysMem = &settingsData;
	settingsData.worldTint = SVector4{ 1.0, 1.0, 1.0, 1.0 };
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, settingsBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	cbDesc.ByteWidth = sizeof(SSpriteFlagsBuffer);
	subResData.pSysMem = &flagsData;
	if (FAILED(d3dDevice->CreateBuffer(&cbDesc, &subResData, flagsBuffer.GetAddressOf())))
	{
		throw std::exception("Cannot create constant buffer");
	}

	// set buffers
	d3dDeviceContext->VSSetConstantBuffers(0, 1, projMatrixBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(1, 1, viewMatrixBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(2, 1, transMatrixBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(3, 1, colorsBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(4, 1, customUvBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(5, 1, frameAnimBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(6, 1, settingsBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(7, 1, flagsBuffer.GetAddressOf());

	S_CATCH{ S_THROW("SConstantBuffersDX11::Init()") }
}

void SConstantBuffersDX11::Destroy()
{
	flagsBuffer.Reset();
	settingsBuffer.Reset();
	frameAnimBuffer.Reset();
	customUvBuffer.Reset();
	colorsBuffer.Reset();
	transMatrixBuffer.Reset();
	projMatrixBuffer.Reset();
	viewMatrixBuffer.Reset();
}
