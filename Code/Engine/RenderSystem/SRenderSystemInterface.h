/***************************************************************************
* SRenderSystemInterface.h
*/

#pragma once

#include "Core/SMathTypes.h"
#include "RenderSystem/RenderSystemModule.h"
#include "RenderSystem/SRenderSystemTypes.h"
#include "Application/SApplicationTypes.h"

#include <filesystem>
#include <string>
#include <map>


/***************************************************************************
* Render system interface
*/
class IRenderSystem
{
public:
	/**
	* Virtual destructor */
	virtual ~IRenderSystem() {}
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
	virtual void Clear(class IWorld* world, bool removeRooted = false) = 0;
	/**
	* Return render system state */
	virtual bool CanRender() const = 0;
	/**
	* Request resize */
	virtual void RequestResize(std::int32_t width, std::int32_t height) = 0;
	/**
	* Resize render system */
	virtual void Resize(std::int32_t width, std::int32_t height, const SAppContext& context) = 0;
	/**
	* Set window mode */
	virtual void SetMode(SAppMode mode) = 0;
	/**
	* Update camera */
	virtual void UpdateCamera(float deltaSeconds, SVector3 newPos, SVector3 newTarget) = 0;
	/**
	* Return current stats */
	virtual SRSStats GetStats() const = 0;
	/**
	* Return render system type */
	virtual SRSType GetType() const = 0;

};

using TRenderSystemPtr = std::unique_ptr<IRenderSystem>;
