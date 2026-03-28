/***************************************************************************
* SWindowsApplication.h
*/

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <chrono>

#include "Application/SInputSystem.h"
#include "Application/SApplicationInterface.h"
#include "RenderSystem/SRenderSystemInterface.h"
#include "World/SWorldInterface.h"
#include "Core/SCoreModule.h"

#pragma warning(disable : 4275)


/***************************************************************************
* Win32 Application class
*/
class SWindowsApplication : public IApplication
{
public:
	/**
	* Default constructor */
	SWindowsApplication();
	//
	using SClock = std::chrono::high_resolution_clock;
	//
	using SDuration = std::chrono::duration<float>;
	//
	using STimePoint = std::chrono::steady_clock::time_point;


public: // IApplication interface implementation
	//
	virtual ~SWindowsApplication() override;
	//
	virtual void Init(void* handle, const std::string& cmds, std::int32_t cmdsCount) noexcept override;
	//
	virtual void Init(void* handle, const std::string& cmds) noexcept override;
	//
	virtual void Init(void* handle) noexcept override;
	//
	virtual void SetInitHandler(IApplication::SInitHandler handler) noexcept override { initHandler = handler; }
	//
	virtual void SetUpdateHandler(IApplication::SUpdateHandler handler) noexcept override { updateHandler = handler; }
	//
	virtual void SetRenderSystem(TRenderSystemPtr render) noexcept override { renderSystem.reset(static_cast<IRenderSystemEx*>(render.release())); }
	//
	virtual void SetFeature(SAppFeature feature, const std::any& value) noexcept override { features[feature] = value; }
	//
	virtual std::any GetFeature(SAppFeature feature) const noexcept override;
	//
	virtual const SAppFeaturesMap& GetFeatures() const noexcept override { return features; }
	//
	virtual void SetWindowMode(SAppMode mode) noexcept override { appMode = mode; }
	//
	virtual SAppMode GetWindowMode() const noexcept override { return appMode; }
	//
	virtual void SetWindowSize(std::uint32_t width, std::uint32_t height, bool resizeRenderSystem = false) override;
	//
	virtual SSize2 GetWindowSize() const noexcept override { return windowSize; }
	//
	virtual void SetConfig(const SAppConfig& newConfig) noexcept override { cfg = newConfig; }
	//
	virtual const SAppConfig& GetConfig() const noexcept override { return cfg; }
	//
	virtual void Run() override;
	//
	virtual void RequestQuit() noexcept override { quit = true; }


protected:
	//
	void OnUpdate();


protected:
	//
	HINSTANCE hInstance{};
	//
	HWND hWnd{};
	//
	std::string cmds;
	//
	std::int32_t cmdsCount;
	//
	bool quit{};
	//
	SSize2 windowSize;
	//
	SAppMode appMode;
	//
	SAppFeaturesMap features;
	//
	SAppConfig cfg;
	//
	STimePoint startFrameTime;
	//
	STimePoint prevFrameTime;
	//
	std::uint32_t currentGameFrame;
	//
	std::uint32_t accumulatedFrames;
	//
	float accumulatedTime{};


protected:
	//
	SAppContext context;
	//
	TInputSystemPtr inputSystem;
	//
	TThreadPoolPtr threadPool;
	//
	IApplication::SInitHandler initHandler;
	//
	IApplication::SUpdateHandler updateHandler;
	//
	TRenderSystemExPtr renderSystem;
	//
	TWorldPtr world;

};
