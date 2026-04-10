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
	void PreloadTextures(const SPathList& paths, OnTexturesLoadedDelegate delegate);
	//
	bool FindTexture(STexID id, ID3D11Texture2D** outTexture, ID3D11ShaderResourceView** outView, SSize2* outTexSize = nullptr) const;
	//
	void LoadCubemap(const std::filesystem::path& path, ECubemapType type, ID3D11Device* d3dDevice);
	//
	ID3D11ShaderResourceView* FindCubemap(ECubemapType type) const;
	//
	void RemoveCubemap(ECubemapType type);
	//
	void ClearCache(IWorld* world);
	//
	inline std::uint32_t GetNumTextures() const { return texturesCache.size(); }


protected:
	//
	struct STextureData
	{
		SBytes data;
		SSize2 texSize;
		STexID id;
	};
	//
	struct SCubemapData
	{
		SBytes data;
		ECubemapType type;
	};
	//
	struct STextureDataDX11
	{
		ComPtr<ID3D11Texture2D> texture;
		ComPtr<ID3D11ShaderResourceView> view;
		SSize2 texSize{};
	};
	//
	struct SCubemapDataDX11
	{
		ComPtr<ID3D11Resource> texture;
		ComPtr<ID3D11ShaderResourceView> view;
	};
	//
	bool LoadTextureData(const std::filesystem::path& path, SBytes* outData, SSize2* outTexSize);
	//
	bool CreateTexture(ID3D11Device* device, const STextureData& textureData, STextureDataDX11& outTexture);
	//
	void CheckPreloadFinished();


protected:
	//
	using TPathList = std::vector<std::filesystem::path>;
	//
	using TPreLoadDelegatesCache = std::list<std::pair<TPathList, OnTexturesLoadedDelegate>>;
	//
	using TCircularFIFOTextureQueue = Fifo4<STextureData>;
	//
	using TCircularFIFOCubemapQueue = Fifo4<SCubemapData>;
	//
	using TPendingMap = std::unordered_map<std::filesystem::path, std::uint32_t>;
	//
	std::unordered_map<ECubemapType, SCubemapDataDX11> cubemapsCache;
	//
	std::unordered_map<STexID, STextureDataDX11> texturesCache;
	//
	std::shared_ptr<TCircularFIFOTextureQueue> loadedTextures;
	//
	std::shared_ptr<TCircularFIFOCubemapQueue> loadedCubemaps;
	//
	TPreLoadDelegatesCache preloadDelegatesCache;
	//
	TPendingMap pendingLoadingMap;
	//
	IThreadPool* threadPool;

};
