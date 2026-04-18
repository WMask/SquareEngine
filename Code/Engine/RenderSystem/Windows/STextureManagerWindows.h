/***************************************************************************
* STextureManagerWindows.h
*/

#pragma once

#include "Core/SMathTypes.h"
#include "Core/ThreadPool/SThreadPoolInterface.h"
#include "RenderSystem/Windows/SUtilsWindows.h"
#include "RenderSystem/SRenderSystemTypes.h"
#include "World/SWorldInterface.h"
#include "Core/SUtils.h"

#include <cstdint>
#include <queue>
#include <map>


/***************************************************************************
* Texture manager
*/
class STextureManagerWindows
{
public:
	STextureManagerWindows() : textureLifetime(nullptr) , threadPool(nullptr) {}
	//
	~STextureManagerWindows();
	//
	void Init(IThreadPool* inThreadPool, ITextureLifetime* inTextureLifetime);
	//
	void Shutdown();
	//
	void Update();
	//
	STexID LoadTexture(const std::filesystem::path& path);
	//
	void PreloadTextures(const SPathList& paths, OnTexturesLoadedDelegate delegate);
	//
	STextureBase* FindTexture(STexID id) const;
	//
	bool RemoveTexture(STexID id);
	//
	void LoadCubemap(const std::filesystem::path& path, ECubemapType type);
	//
	STextureBase* FindCubemap(ECubemapType type) const;
	//
	bool RemoveCubemap(ECubemapType type);
	//
	void ClearCache(IWorld* world);
	//
	inline std::uint32_t GetNumTextures() const noexcept { return texturesCache.size(); }
	//
	inline std::uint32_t GetNumCubemaps() const noexcept { return cubemapsCache.size(); }


protected:
	//
	bool LoadTextureData(const std::filesystem::path& path, SBytes* outData, SSize2* outTexSize);
	//
	bool LoadCubemapData(const std::filesystem::path& path, SBytes* outData);
	//
	void CheckPreloadFinished(const STextureData& textureData);


protected:
	//
	using TTexIDList = std::vector<STexID>;
	//
	using TPreLoadDelegatesCache = std::list<std::pair<TTexIDList, OnTexturesLoadedDelegate>>;
	//
	using TCubemapsCache = std::unordered_map<ECubemapType, std::shared_ptr<STextureBase>>;
	//
	using TTexturesCache = std::unordered_map<STexID, std::shared_ptr<STextureBase>>;
	//
	using TTextureQueue = std::queue<STextureData>;
	//
	using TCubemapQueue = std::queue<SCubemapData>;
	//
	TPreLoadDelegatesCache preloadDelegatesCache;
	//
	TTexturesCache texturesCache;
	//
	TCubemapsCache cubemapsCache;
	//
	TTextureQueue loadedTextures;
	//
	TCubemapQueue loadedCubemaps;
	//
	ITextureLifetime* textureLifetime;
	//
	IThreadPool* threadPool;
	//
	std::mutex sync;

};
