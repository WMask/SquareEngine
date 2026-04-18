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
class STextureManagerWindows : public ITextureManager
{
public:
	//
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
	STextureBase* FindTexture(STexID id) const;
	//
	STextureBase* FindCubemap(ECubemapType type) const;
	//
	void ClearCache(IWorld* world);
	//
	inline std::uint32_t GetNumTextures() const noexcept { return texturesCache.size(); }
	//
	inline std::uint32_t GetNumCubemaps() const noexcept { return cubemapsCache.size(); }


public:// ITextureManager interface implementation
	//
	virtual STexID LoadTexture(const SPath& path) override;
	//
	virtual void PreloadTextures(const SPathList& paths) override;
	//
	virtual void PreloadTextures(const SPathList& paths, OnTexturesLoadedDelegate delegate) override;
	//
	virtual bool RemoveTexture(STexID id) override;
	//
	virtual void LoadCubemap(const SPath& path, ECubemapType type) override;
	//
	virtual bool RemoveCubemap(ECubemapType type) override;
	//
	virtual std::pair<SSize2, bool> GetTextureSize(STexID id) const override;


protected:
	//
	bool LoadTextureData(const SPath& path, SBytes* outData, SSize2* outTexSize);
	//
	bool LoadCubemapData(const SPath& path, SBytes* outData);
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
