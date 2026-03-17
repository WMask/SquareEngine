/***************************************************************************
* SException.h
*/

#pragma once

#include <exception>
#include <string>


/***************************************************************************
* Exception class
*/
class SException : public std::exception
{
public:
	SException() = default;
	//
	SException(const SException&) = default;
	//
	SException& operator=(const SException&) = default;
	//
	SException(const std::string& inMessage) : message(inMessage) {}
	//
	SException(const std::exception& prevEx, const char* curLocation)
	{
		message = prevEx.what();
		message += ">\n";
		message += curLocation;
	}
	//
	SException(const std::exception& prevEx, const char* preMsg, const char* msg, const char* postMsg)
	{
		message = prevEx.what();
		message += ">\n";
		message += preMsg;
		message += msg;
		message += postMsg;
	}


public:// std::exception interface implementation
	virtual char const* what() const override { return message.c_str(); }


protected:
	std::string message;

};


// Start catch block
#define S_CATCH } catch (const std::exception& ex)
// Throw appended exception
#define S_THROW(curLocation) throw SException(ex, curLocation);
// Throw appended exception
#define S_THROW_EX(preMsg, msg, postMsg) throw SException(ex, preMsg, msg, postMsg);
// Start try-catch block
#define S_TRY try\
{
