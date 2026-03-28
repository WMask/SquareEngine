/***************************************************************************
* STextureManagerDX11.h
*/

#pragma once

#include "Core/SMathTypes.h"
#include "Core/ThreadPool/SThreadPoolInterface.h"
#include "Core/ThreadPool/Fifo4.h"
#include "Core/SUtils.h"
#include "World/SWorldInterface.h"
#include "RenderSystem/SRenderSystemTypes.h"

#include <d3d11.h>
#include <wrl.h>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>

using Microsoft::WRL::ComPtr;


/**
* Texture manager */
class STextureManagerDX11
{
public:
	STextureManagerDX11() : threadPool(nullptr) {}
	//
	~STextureManagerDX11();
	//
	void Init(IThreadPool* threadPool);
	//
	void Shutdown();
	//
	void Update(ID3D11Device* d3dDevice);
	//
	STexID LoadTexture(const std::filesystem::path& path);
	//
	void PreLoadTextures(const SPathList& paths, OnPreLoadTexturesDelegate delegate);
	//
	bool FindTexture(STexID id, ID3D11Texture2D** outTexture, ID3D11ShaderResourceView** outView, SSize2* outTexSize = nullptr) const;
	/**
	* If world is not null - only unused textures removed. If null - all textures removed. */
	void ClearCache(IWorld* world);
	//
	inline int GetNumTextures() const { return (int)texturesCache.size(); }


protected:
	//
	struct STextureData
	{
		SBytes data;
		//
		SSize2 texSize;
		//
		STexID id;
	};
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
	STexID GenerateTexID(const std::filesystem::path& path) const;
	//
	bool LoadTextureData(const std::filesystem::path& path, SBytes* outData, SSize2* outTexSize);
	//
	bool CreateTexture(ID3D11Device* device, const STextureData& textureData, STextureDataDX11& outTexture);
	//
	void CheckPreLoadFinished();


protected:
	//
	using TPreLoadDelegatesCache = std::list<std::pair<TTexIDList, OnPreLoadTexturesDelegate>>;
	//
	using TCircularFIFOTextureQueue = Fifo4<STextureData>;
	//
	std::unordered_map<STexID, STextureDataDX11> texturesCache;
	//
	std::shared_ptr<TCircularFIFOTextureQueue> loadedTextures;
	//
	TPreLoadDelegatesCache preLoadDelegatesCache;
	//
	std::hash<std::string> hasher;
	//
	IThreadPool* threadPool;

};
