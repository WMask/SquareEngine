/***************************************************************************
* SLogger.h
*/

#pragma once

#include <memory>
#include <string>


/***************************************************************************
* Logger interface
*/
class ILogger
{
public:
	/**
	* Joins all worker threads before destruction */
	virtual ~ILogger() {}
	/**
	* Set logger mode */
	virtual void SetDebugLogging(bool bEnableDebugLogging) = 0;
	/**
	* Log message */
	virtual void Log(const std::string& msg) = 0;
	/**
	* Log message */
	virtual void LogConst(const std::string_view& msg) = 0;
	/**
	* Log message if debug logging feature enabled */
	virtual void LogDebug(const std::string& msg) = 0;
	/**
	* Log message if debug logging feature enabled */
	virtual void LogDebugConst(const std::string_view& msg) = 0;

};

using TLoggerPtr = std::unique_ptr<ILogger>;
