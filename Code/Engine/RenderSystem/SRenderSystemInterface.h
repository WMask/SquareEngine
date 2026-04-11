/***************************************************************************
* SRenderSystemInterface.h
*/

#pragma once

#include "Core/SUtils.h"
#include "Core/SMathTypes.h"
#include "RenderSystem/RenderSystemModule.h"
#include "RenderSystem/SRenderSystemTypes.h"
#include "Application/SApplicationTypes.h"
#include "World/SWorldInterface.h"

#include <string>
#include <map>


/***************************************************************************
* Render system interface
*/
class IRenderSystem : public SUncopyable
{
public:
	//
	struct SShaderData {};

public:
	/**
	* Virtual destructor */
	virtual ~IRenderSystem() {}
	/**
	* Request async texture loading and return id.
	* Loading may take some time, after that texture will be available at this id.
	* Render system skip rendering if texture not loaded yet.
	*/
	virtual STexID LoadTexture(const std::filesystem::path& texturePath) = 0;
	/**
	* Load textures and call delegate */
	virtual void PreloadTextures(const SPathList& paths, OnTexturesLoadedDelegate delegate) = 0;
	/**
	* Load cubemap from dds file */
	virtual void LoadCubemap(const std::filesystem::path& path, ECubemapType type) = 0;
	//
	virtual void RemoveCubemap(ECubemapType type) = 0;
	/**
	* Cubemap amount (0 - 1) */
	virtual void SetCubemapAmount(float amount, ECubemapType type) = 0;
	/**
	* Cubemap amount (0 - 1) */
	virtual float GetCubemapAmount(ECubemapType type) const noexcept = 0;
	/**
	* Load mesh scene instances and call delegate.
	* Loads meshes with material textures if instance's mesh not loaded yet.
	*/
	virtual void LoadStaticMeshInstances(const std::filesystem::path& path, SGroupID groupId, OnMeshInstancesLoadedDelegate delegate) = 0;
	/**
	* Load meshes with material textures and call delegate */
	virtual void PreloadStaticMeshes(const std::filesystem::path& path, OnMeshesLoadedDelegate delegate) = 0;
	/**
	* Remove all graphics objects: textures, fonts etc. */
	virtual void Clear(IWorld* world, bool removeRooted = false) = 0;
	/**
	* Request resize */
	virtual void RequestResize(std::uint32_t width, std::uint32_t height) = 0;
	/**
	* Update camera */
	virtual void UpdateCamera(const SCamera& camera) = 0;
	/**
	* Set window mode */
	virtual void SetMode(SAppMode mode) = 0;
	/**
	* Return client render size in pixels */
	virtual SSize2 GetRenderSize() const noexcept = 0;
	/**
	* Return current stats */
	virtual SRSStats GetStats() const noexcept = 0;
	/**
	* Return render system type */
	virtual SRSType GetType() const noexcept = 0;

};

using TRenderSystemPtr = std::unique_ptr<IRenderSystem>;


/***************************************************************************
* Extended render system interface
*/
class IRenderSystemEx : public IRenderSystem
{
public:
	/**
	* Load shaders */
	virtual void LoadShaders(const std::filesystem::path& folderPath) = 0;
	/**
	* Create render system */
	virtual void Create(void* windowHandle, SAppMode mode, const SAppContext& context) = 0;
	/**
	* Shutdown render system */
	virtual void Shutdown() = 0;
	/**
	* Subscribe to world scale delegate */
	virtual void Subscribe(const SAppContext& context) = 0;
	/**
	* Update world */
	virtual void Update(float deltaSeconds, const SAppContext& context) = 0;
	/**
	* Render world */
	virtual void Render(const SAppContext& context) = 0;
	/**
	* Remove all graphics objects: textures, fonts etc. */
	virtual void Clear(IWorld* world, bool removeRooted = false) = 0;
	/**
	* Return render system state */
	virtual bool CanRender() const noexcept = 0;
	/**
	* Request resize */
	virtual void RequestResize(std::uint32_t width, std::uint32_t height) = 0;
	/**
	* Resize render system */
	virtual void Resize(std::uint32_t width, std::uint32_t height, const SAppContext& context) = 0;
	/**
	* Adds draw calls debug info */
	virtual void AddDrawCalls(std::uint32_t drawCalls) noexcept = 0;

};

using TRenderSystemExPtr = std::unique_ptr<IRenderSystemEx>;
