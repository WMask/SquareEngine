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
		const SColor4F& globalTint, const SColor4F& backLight, const SColor4F& pbrGammaCorrection);
	//
	void UpdateMaterialFlags(const IRenderSystemDX11& renderSystem, const SMaterialFlagsBuffer& materials);
	//
	void UpdateLightSettings(const IRenderSystemDX11& renderSystem, const SLightsBuffer& lights);
	//
	void UpdateCubemapSettings(const IRenderSystemDX11& renderSystem);
	//
	void UpdateTransform(const IRenderSystemDX11& renderSystem, const STransform& transform);
	//
	void Shutdown();


public:
	//
	ID3D11ShaderResourceView* GetDefaultTexture() const { return defaultTextureView.Get(); }
	//
	ID3D11Buffer* GetSpriteVB() const { return spriteVertexBuffer.Get(); }
	//
	ID3D11Buffer* GetSpriteIB() const { return spriteIndexBuffer.Get(); }


protected:
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
	ComPtr<ID3D11Buffer> cubemapsBuffer;
	//
	ComPtr<ID3D11Buffer> lightsBuffer;
	//
	ComPtr<ID3D11Buffer> materialBuffer;
	//
	ComPtr<ID3D11Buffer> transformBuffer;

};
