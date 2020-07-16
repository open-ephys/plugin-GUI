#ifndef UTIL_H_DEFINED
#define UTIL_H_DEFINED

#include <chrono>
#include <functional>
#include <mutex>
#include <iostream>
#include <string>
#include <map>

#define CHANNEL_HOP 96
//#define DEBUG 1
#define LOGD OELogger::instance().LOGDebug

/* Function Timer */
template <typename Time = std::chrono::microseconds, typename Clock = std::chrono::high_resolution_clock>
struct func_timer
{
	template <typename F, typename... Args>
	static Time duration(F&& f, Args... args)
	{
		auto start = Clock::now();
		invoke(std::forward<F>(f), std::forward<Args>(args)...);
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
		std::lock_guard<std::mutex> lock(mt);
		(std::cout << ... << args);
		std::cout << std::endl;
	}
	
private:
	std::mutex mt;
};

/* Templates for getting keys and values from std::maps */

// From: http://www.lonecpluspluscoder.com/2015/08/13/an-elegant-way-to-extract-keys-from-a-c-map/

template<typename TK, typename TV>
std::vector<TK> extract_keys(std::map<TK, TV> const& input_map) {
  std::vector<TK> retval;
  for (auto const& element : input_map) {
    retval.push_back(element.first); 
  }
  return retval;
}

template<typename TK, typename TV>
std::vector<TV> extract_values(std::map<TK, TV> const& input_map) {
  std::vector<TV> retval;
  for (auto const& element : input_map) {
    retval.push_back(element.second);
  }
  return retval;
}

#endif


