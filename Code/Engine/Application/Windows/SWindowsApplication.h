/***************************************************************************
* SWindowsApplication.h
*/

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <chrono>

#include "Application/SApplicationInterface.h"

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
	virtual void SetRenderSystem(TRenderSystemPtr render) noexcept override { renderSystem = std::move(render); }
	//
	virtual void SetFeature(SAppFeature feature, const std::any& value) noexcept override { features[feature] = value; }
	//
	virtual std::any GetFeature(SAppFeature feature) const noexcept override;
	//
	virtual void SetWindowMode(SAppMode mode) override;
	//
	virtual void SetWindowSize(std::int32_t width, std::int32_t height) override;
	//
	virtual SSize2 GetWindowSize() const noexcept override { return windowSize; }
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
	STimePoint startFrameTime;
	//
	STimePoint prevFrameTime;
	//
	std::uint32_t currentGameFrame;
	//
	std::int32_t accumulatedFrames;
	//
	float accumulatedTime;


protected:
	//
	SAppContext context;
	//
	IApplication::SInitHandler initHandler;
	//
	IApplication::SUpdateHandler updateHandler;
	//
	TRenderSystemPtr renderSystem;

};
