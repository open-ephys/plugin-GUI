#ifndef UTIL_H_DEFINED
#define UTIL_H_DEFINED

#include <chrono>
#include <functional>
#include <mutex>
#include <iostream>
#include <fstream>
#include <string>
#include <map>

/* Log Action -- taken by user */
#define LOGA(...) \
    OELogger::instance().LOGConsole("[open-ephys][action] ", __VA_ARGS__);

/* Log Buffer -- related logs i.e. inside process() method */
#define LOGB(...) \
    OELogger::instance().LOGFile("[open-ephys][buffer] ", __VA_ARGS__);

/* Log Console -- gets printed to the GUI Debug Console */
#define LOGC(...) \
    OELogger::instance().LOGConsole("[open-ephys] ", __VA_ARGS__);

/* Log Debug -- gets printed to the log file */
#define LOGD(...) \
    OELogger::instance().LOGFile("[open-ephys][debug] ", __VA_ARGS__);

/* Log Deep Debug -- gets printed to log file (e.g. enable after a crash to get more details) */
#define LOGDD(...) \
    OELogger::instance().LOGFile("[open-ephys][ddebug] ", __VA_ARGS__);

/* Log Error -- gets printed to console with flare */
#define LOGE(...) \
    OELogger::instance().LOGError("[open-ephys] ***ERROR*** ", __VA_ARGS__);

/* Log File -- gets printed directly to main output file */
#define LOGF(...) LOGD(...)

/* Log Graph -- gets logs related to processor graph generation/modification events */
#define LOGG(...) \
    OELogger::instance().LOGConsole("[open-ephys][graph] ", __VA_ARGS__);

/* Thread-safe logger */
class OELogger
{

std::ofstream logFile;

protected:

	OELogger() {
		// Each time the GUI is launched, a new error log is generated.
		// In case of a crash, the most recent file is appended with a datestring
		logFile.open("activity.log", std::ios::out | std::ios::trunc );
		time_t now = time(0);
		logFile << "[open-ephys] Session start time: " << ctime(&now);
	}
public:
	static OELogger& instance()
	{
		static OELogger lg;
		return lg;
	}

	OELogger(OELogger const&) = delete;
	OELogger& operator=(OELogger const&) = delete;

	template<typename ...Args>
	void LOGConsole(Args&& ...args)
	{
		std::lock_guard<std::mutex> lock(mt);
		
		(std::cout << ... << args);
		std::cout << std::endl;

		LOGFile(args...);
	}

	template<typename ...Args>
	void LOGError(Args&& ...args)
	{
		std::lock_guard<std::mutex> lock(mt);

		(std::cout << ... << args);
		std::cout << std::endl;

		LOGFile(args...);
	}

	template<typename ...Args>
	void LOGFile(Args&& ...args)
	{
		(logFile << ... << args);
		logFile << std::endl;
	}

private:
	std::mutex mt;
};








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

		return std::chrono::duration_cast<Time>(end - start);
	}
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
