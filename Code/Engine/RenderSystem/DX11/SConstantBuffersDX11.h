/***************************************************************************
* SConstantBuffersDX11.h
*/

#pragma once

#include "Core/STypes.h"
#include "World/SCamera.h"

#include <d3d11.h>
#include <wrl.h>
#include <string>
#include <map>

using Microsoft::WRL::ComPtr;


/** Sprite vertex data */
struct DX11SPRITEVERTEX
{
	SVector3 pos;
};

/***************************************************************************
* Constant buffers
*/
struct SConstantBuffersDX11
{
public:
	//
	SConstantBuffersDX11() {}
	//
	~SConstantBuffersDX11();
	//
	void Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext,
		const SCamera& camera, std::uint32_t width, std::uint32_t height);
	//
	void ApplyTransform2D(ID3D11DeviceContext* d3dDeviceContext, const SCamera& camera, SVector2 worldScale, std::uint32_t width, std::uint32_t height);
	//
	void ApplyTransform3D(ID3D11DeviceContext* d3dDeviceContext, const SCamera& camera, std::uint32_t width, std::uint32_t height);
	//
	void Shutdown();


public:
	//
	ComPtr<ID3D11Buffer> spriteVertexBuffer;
	//
	ComPtr<ID3D11Buffer> spriteIndexBuffer;
	//
	ComPtr<ID3D11Buffer> wvpMatrixBuffer;
	//
	ComPtr<ID3D11Buffer> settingsBuffer;
	//
	ComPtr<ID3D11Buffer> lightsBuffer;
	//
	ComPtr<ID3D11Buffer> meshBuffer;

};
