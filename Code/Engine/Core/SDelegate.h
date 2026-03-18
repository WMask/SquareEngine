/***************************************************************************
* SDelegate.h
*/

#pragma once

#include <functional>
#include <deque>


/**
* Multicast delegate class */
template<typename ...A>
class SDelegate
{
	typedef std::function<void(A...)> DType;

public:
	SDelegate() = default;
	//
	SDelegate(const SDelegate&) = default;
	//
	SDelegate& operator=(const SDelegate&) = default;
	//
	void AddListener(const DType& listener) { listeners.push_back(listener); }
	//
	void RemoveListener(const DType& listener) { listeners.erase(listener); }
	//
	void Broadcast(A... a)
	{
		for (DType& listener : listeners)
		{
			listener(a...);
		}
	}

protected:
	std::deque<DType> listeners;
};
