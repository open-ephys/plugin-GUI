/*
	 ------------------------------------------------------------------

	 This file is part of the Open Ephys GUI
	 Copyright (C) 2013 Open Ephys

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

#ifndef __PLUGINMGR__
#define __PLUGINMGR__

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CFBundle.h>
#endif

#include <list>
#include <string>
#include <sys/types.h>
#include "../../../JuceLibraryCode/JuceHeader.h"
#include "OpenEphysPlugin.h"

struct LoadedLibInfo : public Plugin::LibraryInfo
{
#ifdef WIN32
	HINSTANCE handle;
#elif defined(__APPLE__)
    CFBundleRef handle;
#else
	void* handle;
#endif
};

template<class T>
struct LoadedPluginInfo : public T
{
	int libIndex;
};


class GenericProcessor;

class PluginManager {

public:
	PluginManager();
	~PluginManager();
	void loadAllPlugins();
    void loadPlugins(const File &pluginPath);
	int loadPlugin(const String&);
	//void unloadPlugin(Plugin *);
	void removeAllPlugins();
	int getNumProcessors() const;
	int getNumDataThreads() const;
	int getNumRecordEngines() const;
	int getNumFileSources() const;
	Plugin::ProcessorInfo getProcessorInfo(int index) const;
	Plugin::ProcessorInfo getProcessorInfo(String name, String libName = String::empty) const;
	Plugin::DataThreadInfo getDataThreadInfo(int index) const;
	Plugin::DataThreadInfo getDataThreadInfo(String name, String libName = String::empty) const;
	Plugin::RecordEngineInfo getRecordEngineInfo(int index) const;
	Plugin::RecordEngineInfo getRecordEngineInfo(String name, String libName = String::empty) const;
	Plugin::FileSourceInfo getFileSourceInfo(int index) const;
	Plugin::FileSourceInfo getFileSourceInfo(String name, String libName = String::empty) const;
	String getLibraryName(int index) const;
	int getLibraryVersion(int index) const;
	int getLibraryIndexFromPlugin(Plugin::PluginType type, int index);

private:
	Array<LoadedLibInfo> libArray;
	Array<LoadedPluginInfo<Plugin::ProcessorInfo>> processorPlugins;
	Array<LoadedPluginInfo<Plugin::DataThreadInfo>> dataThreadPlugins;
	Array<LoadedPluginInfo<Plugin::RecordEngineInfo>> recordEnginePlugins;
	Array<LoadedPluginInfo<Plugin::FileSourceInfo>> fileSourcePlugins;

	template<class T>
	bool findPlugin(String name, String libName, const Array<LoadedPluginInfo<T>>& pluginArray, T& pluginInfo) const;

	/* Making the info structures have a constructor complicates the DLL interface. 
	It's easier to just add some static methods to create empty structures for when the calls fail*/
	static Plugin::ProcessorInfo getEmptyProcessorInfo();
	static Plugin::DataThreadInfo getEmptyDatathreadInfo();
	static Plugin::RecordEngineInfo getEmptyRecordengineInfo();
	static Plugin::FileSourceInfo getEmptyFileSourceInfo();
	/*Manager(void) {};
	~Manager(void) {};
	Manager(const Manager &) {};

	
	Manager &operator=(const Manager &) { return *getInstance(); };

	
	void removeListPlugin(PluginManager::Plugin *);
	std::list<PluginManager::Plugin *> pluginList;
	static Manager *instance;*/

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginManager);
};

#endif
