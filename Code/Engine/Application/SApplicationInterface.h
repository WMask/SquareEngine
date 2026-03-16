/***************************************************************************
* SApplicationInterface.h
*/

#pragma once

#include "Application/SApplicationTypes.h"
#include "Core/SMathTypes.h"
#include "Core/STypes.h"

#include <string>
#include <functional>

#if defined(_MSC_VER)
# pragma warning(disable : 4251)
#endif


/***************************************************************************
* Application interface
*/
class IApplication
{
public:
	/**
	Init handler */
	typedef std::function<void(SAppContext&)> SInitHandler;
	/**
	Update handler with delta seconds */
	typedef std::function<void(float, SAppContext&)> SUpdateHandler;


public:
	/**
	* Virtual destructor */
	virtual ~IApplication() {}
	/**
	* Set app parameters */
	virtual void Init(void* handle, const std::string& cmds, int cmdsCount) noexcept = 0;
	/**
	* Set app parameters */
	virtual void Init(void* handle, const std::string& cmds) noexcept = 0;
	/**
	* Set app parameters */
	virtual void Init(void* handle) noexcept = 0;
	/**
	* Set init handler */
	virtual void SetInitHandler(SInitHandler handler) noexcept = 0;
	/**
	* Set update handler */
	virtual void SetUpdateHandler(SUpdateHandler handler) noexcept = 0;
	/**
	* Set application feature */
	virtual void SetFeature(SAppFeature feature, const SAny& value) noexcept = 0;
	/**
	* Get application feature value */
	virtual SAny GetFeature(SAppFeature feature) const noexcept = 0;
	/**
	* Set window mode and update render system */
	virtual void SetWindowMode(SAppMode mode) = 0;
	/**
	* Set client window size in pixels and update render system */
	virtual void SetWindowSize(std::int32_t width, std::int32_t height) = 0;
	/**
	* Get client window size pixels */
	virtual SSize2 GetWindowSize() const noexcept = 0;
	/**
	* Run application main loop */
	virtual void Run() = 0;
	/**
	* Request application quit */
	virtual void RequestQuit() noexcept = 0;


protected:
	//
	IApplication() {}
	//
	IApplication(const IApplication&) = delete;
	//
	IApplication& operator=(const IApplication&) = delete;

};
