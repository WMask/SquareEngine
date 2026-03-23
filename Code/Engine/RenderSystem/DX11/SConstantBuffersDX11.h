/***************************************************************************
* SConstantBuffersDX11.h
*/

#pragma once

#include "Core/SMathTypes.h"

#include <d3d11.h>
#include <wrl.h>
#include <string>
#include <map>

using Microsoft::WRL::ComPtr;


/***************************************************************************
* Constant buffers
*/
struct SConstantBuffersDX11
{
public:
	//
	SConstantBuffersDX11() {}
	//
	void Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext,
		SVector3 cameraPos, SVector3 cameraTarget, std::uint32_t width, std::uint32_t height);
	//
	void Destroy();


public:
	//
	ComPtr<ID3D11Buffer> projMatrixBuffer;
	//
	ComPtr<ID3D11Buffer> transMatrixBuffer;
	//
	ComPtr<ID3D11Buffer> viewMatrixBuffer;
	//
	ComPtr<ID3D11Buffer> colorsBuffer;
	//
	ComPtr<ID3D11Buffer> customUvBuffer;
	//
	ComPtr<ID3D11Buffer> frameAnimBuffer;
	//
	ComPtr<ID3D11Buffer> settingsBuffer;
	//
	ComPtr<ID3D11Buffer> flagsBuffer;

};
