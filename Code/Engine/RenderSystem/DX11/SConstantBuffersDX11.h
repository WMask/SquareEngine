/***************************************************************************
* SConstantBuffersDX11.h
*/

#pragma once

#include "Core/STypes.h"
#include "World/SCamera.h"
#include "RenderSystem/DX11/SRenderSystemTypesDX11.h"
#include <string>
#include <map>


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
	void ApplyTransform2D(ID3D11DeviceContext* d3dDeviceContext, const SCamera& camera,
		SVector2 worldScale, std::uint32_t width, std::uint32_t height);
	//
	void ApplyTransform3D(ID3D11DeviceContext* d3dDeviceContext, const SCamera& camera,
		std::uint32_t width, std::uint32_t height, float gameTime);
	//
	void UpdateSettingsBuffer(const class IRenderSystemDX11& renderSystem, const SCamera& camera,
		const std::optional<SColor3>& globalTint);
	//
	void Shutdown();


public:
	//
	ComPtr<ID3D11ShaderResourceView> defaultTextureView;
	//
	ComPtr<ID3D11Texture2D> defaultTexture;
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
	ComPtr<ID3D11Buffer> materialBuffer;

};
