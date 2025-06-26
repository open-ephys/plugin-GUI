/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef UTIL_H_DEFINED
#define UTIL_H_DEFINED

#include <chrono>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>

#include "../Processors/PluginManager/PluginAPI.h"

/* Thread-safe logger */
class PLUGIN_API OELogger
{
public:
    static OELogger& instance()
    {
        static OELogger lg;
        return lg;
    }

    template <typename... Args>
    void LOGConsole (Args&&... args)
    {
        std::lock_guard<std::mutex> lock (mt);

        (std::cout << ... << args);
        std::cout << std::endl;

        LOGFile (args...);
    }

    template <typename... Args>
    void LOGError (Args&&... args)
    {
        std::lock_guard<std::mutex> lock (mt);

        (std::cerr << ... << args);
        std::cerr << std::endl;

        LOGFile (args...);
    }

    template <typename... Args>
    void LOGFile (Args&&... args)
    {
        logFile << "[" << getCurrentTimeIso() << "]";
        (logFile << ... << args);
        logFile << std::endl;
    }

    void createLogFile (std::string const& filePath)
    {
        // Each time the GUI is launched, a new error log is generated.
        // In case of a crash, the most recent file is appended with a datestring
        logFile.open (filePath, std::ios::out | std::ios::app);
        time_t now = time (0);
        logFile << "[open-ephys] Session start time: " << ctime (&now);
    }

    static std::string getModuleName();
    static std::string formatModuleName (const std::string& path);
    static std::string getCurrentTimeIso();

private:
    std::mutex mt;
    std::ofstream logFile;

    OELogger() = default;
    ~OELogger() = default;

    // Disable copy and move
    OELogger (const OELogger&) = delete;
    OELogger& operator= (const OELogger&) = delete;
};

/* Expose the Logger instance to plugins */
extern "C" PLUGIN_API OELogger& getOELogger();

/* Log Action -- taken by user */
#define LOGA(...) \
    getOELogger().LOGFile (getOELogger().getModuleName(), "[action] ", __VA_ARGS__);

/* Log Buffer -- related logs i.e. inside process() method */
#define LOGB(...) \
    getOELogger().LOGFile (getOELogger().getModuleName(), "[buffer] ", __VA_ARGS__);

/* Log Console -- gets printed to the GUI Debug Console */
#define LOGC(...) \
    getOELogger().LOGConsole (getOELogger().getModuleName(), " ", __VA_ARGS__);

/* Log Debug -- gets printed to the console in debug mode, to file otherwise */
#ifdef DEBUG

#define LOGD(...) \
    getOELogger().LOGConsole ("[open-ephys][debug] ", __VA_ARGS__);
#else
/* Log Debug -- gets printed to the log file */
#define LOGD(...) \
    getOELogger().LOGFile (getOELogger().getModuleName(), "[debug] ", __VA_ARGS__);
#endif

/* Log Deep Debug -- gets printed to log file (e.g. enable after a crash to get more details) */
#define LOGDD(...) \
    getOELogger().LOGFile (getOELogger().getModuleName(), "[ddebug] ", __VA_ARGS__);

/* Log Error -- gets printed to console with flare */
#define LOGE(...) \
    getOELogger().LOGError (getOELogger().getModuleName(), "***ERROR*** ", __VA_ARGS__);

/* Log File -- gets printed directly to main output file */
#define LOGF(...) LOGD (...)

/* Log Graph -- gets logs related to processor graph generation/modification events */
#define LOGG(...) \
    getOELogger().LOGFile (getOELogger().getModuleName(), "[graph] ", __VA_ARGS__);

/* Function Timer */
template <typename Time = std::chrono::microseconds, typename Clock = std::chrono::high_resolution_clock>
struct func_timer
{
    template <typename F, typename... Args>
    static Time duration (F&& f, Args... args)
    {
        auto start = Clock::now();
        invoke (std::forward<F> (f), std::forward<Args> (args)...);
        auto end = Clock::now();

        return std::chrono::duration_cast<Time> (end - start);
    }
};

/* Templates for getting keys and values from std::maps */

// From: http://www.lonecpluspluscoder.com/2015/08/13/an-elegant-way-to-extract-keys-from-a-c-map/

template <typename TK, typename TV>
std::vector<TK> extract_keys (std::map<TK, TV> const& input_map)
{
    std::vector<TK> retval;
    for (auto const& element : input_map)
    {
        retval.push_back (element.first);
    }
    return retval;
}

template <typename TK, typename TV>
std::vector<TV> extract_values (std::map<TK, TV> const& input_map)
{
    std::vector<TV> retval;
    for (auto const& element : input_map)
    {
        retval.push_back (element.second);
    }
    return retval;
}

#endif
