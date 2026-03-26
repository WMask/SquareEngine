/***************************************************************************
* SWindowsUtils.h
*/

#pragma once

#include "Core/SMathTypes.h"
#include "World/SWorldInterface.h"
#include "RenderSystem/SRenderSystemTypes.h"

#include <d3d11.h>
#include <wrl.h>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>

using Microsoft::WRL::ComPtr;

namespace SConst
{
	static const DXGI_FORMAT DefaultBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
}


/** Find display mode */
bool SFindDisplayMode(std::int32_t width, std::int32_t height, std::int32_t maxRefreshRate, DXGI_MODE_DESC* outMode);

/** Make window association to enable fullscreen mode on Alt + Enter */
void SMakeWindowAssociation(HWND hWnd);


/**
* Texture manager */
class STextureManagerDX11
{
public:
	STextureManagerDX11();
	//
	~STextureManagerDX11();
	//
	std::pair<STexID, bool> LoadTexture(const std::filesystem::path& texPath, ID3D11Device* device,
		ID3D11Texture2D** outTexture, ID3D11ShaderResourceView** outView, SSize2* outTexSize);
	//
	void RemoveTextures() { texturesCache.clear(); }
	/** If world is not null - only unused textures removed. If null - all textures removed. */
	void ClearCache(IWorld* world);
	//
	inline int GetNumTextures() const { return (int)texturesCache.size(); }


protected:
	//
	struct STextureDataDX11
	{
		ComPtr<ID3D11Texture2D> texture;
		//
		ComPtr<ID3D11ShaderResourceView> view;
		//
		SSize2 texSize{};
	};
	//
	std::hash<std::string> hasher;
	//
	std::unordered_map<STexID, STextureDataDX11> texturesCache;

};
