/***************************************************************************
* SRenderSystemInterface.h
*/

#pragma once

#include "Core/SUtils.h"
#include "Core/STypes.h"
#include "RenderSystem/RenderSystemModule.h"
#include "RenderSystem/SRenderSystemTypes.h"
#include "Application/SApplicationTypes.h"
#include "World/SWorldInterface.h"

#include <string>
#include <map>

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif


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
	* Cubemap amount (0 - 1) */
	virtual void SetCubemapAmount(float amount, ECubemapType type) = 0;
	/**
	* Cubemap amount (0 - 1) */
	virtual float GetCubemapAmount(ECubemapType type) const noexcept = 0;
	/**
	* Set global shaders tint color */
	virtual void SetGlobalTint(const SColor3F& tint) = 0;
	/**
	* Get global shaders tint color */
	virtual SColor3F GetGlobalTint() const = 0;
	/**
	* Back light color for 3d world */
	virtual void SetBackLight(const SColor3F& color) = 0;
	/**
	* Back light color for 3d world */
	virtual SColor3F GetBackLight() const noexcept = 0;
	/**
	* Gamma correction on final pbr color */
	virtual void SetGammaCorrection(const SColor3F& color) = 0;
	/**
	* Gamma correction on final pbr color */
	virtual SColor3F GetGammaCorrection() const noexcept = 0;
	/**
	* Get mesh materials */
	virtual std::pair<SMaterialsList, bool> FindMeshMaterials(entt::entity entity) const = 0;
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
	* Return texture manager */
	virtual class ITextureManager* GetTextureManager() noexcept = 0;
	/**
	* Return mesh manager */
	virtual class IMeshManager* GetMeshManager() noexcept = 0;
	/**
	* Return client viewport client size in pixels */
	virtual SSize2 GetScreenSize() const noexcept = 0;
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
#ifdef WIN32
	/**
	* Create render system */
	virtual void Create(HWND inHWnd, SAppMode mode, const SAppContext& context) = 0;
#endif
	/**
	* Load shaders */
	virtual void LoadShaders(const SPath& folderPath) = 0;
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
	* Resize render system */
	virtual void Resize(const SSize2& size, const SAppContext& context,
		bool bForceResize = false) = 0;
	/**
	* Adds draw calls debug info */
	virtual void AddDrawCalls(std::uint32_t drawCalls) noexcept = 0;

};

using TRenderSystemExPtr = std::unique_ptr<IRenderSystemEx>;
