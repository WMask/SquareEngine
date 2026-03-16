/***************************************************************************
* SWindowsApplication.h
*/

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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


public: // IApplication interface implementation
	//
	virtual ~SWindowsApplication() override;
	//
	virtual void Init(void* handle, const std::string& cmds, int cmdsCount) noexcept override;
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
	virtual void SetFeature(SAppFeature feature, const SAny& value) noexcept override { features[feature] = value; }
	//
	virtual SAny GetFeature(SAppFeature feature) const noexcept override { return features[feature]; }
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
	int cmdsCount;
	//
	bool quit{};
	//
	SSize2 windowSize;
	//
	SAppMode appMode;
	//
	LARGE_INTEGER prevTime;
	//
	LARGE_INTEGER frequency;
	//
	mutable SAppFeaturesMap features;


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
