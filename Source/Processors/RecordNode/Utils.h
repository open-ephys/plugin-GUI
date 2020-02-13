#ifndef UTIL_H_DEFINED
#define UTIL_H_DEFINED

#include <chrono>
#include <functional>
#include <mutex>
#include <iostream>
#include <string>

#define CHANNEL_HOP 96
#define DEBUG 1
#define LOGD OELogger::instance().LOGDebug

/* Function Timer */
template <typename Time = std::chrono::microseconds, typename Clock = std::chrono::high_resolution_clock>
struct func_timer
{
	template <typename F, typename... Args>
	static Time duration(F&& f, Args... args)
	{
		auto start = Clock::now();
		//TODO: Not working on mac?
		//std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
		auto end = Clock::now();

		return std::chrono::duration_cast<Time>(end-start);
	}
};

/* Thread-safe logger */
class OELogger
{
protected: 
	OELogger() {}
public: 
	static OELogger& instance()
	{
		static OELogger lg;
		return lg;
	}

	OELogger(OELogger const &) = delete;
	OELogger& operator=(OELogger const &) = delete;

	template<typename ...Args>
	void LOGDebug(Args && ...args)
	{
	#ifdef DEBUG
		std::lock_guard<std::mutex> lock(mt);
		(std::cout << ... << args);
		std::cout << std::endl;
	#endif
	}
	
private:
	std::mutex mt;
};

#endif
