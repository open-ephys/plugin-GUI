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

#include <list>
#include <string>
#include <sys/types.h>
#include "../../../JuceLibraryCode/JuceHeader.h"
#include "OpenEphysPlugin.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined(__APPLE__)
namespace CF {
#include <CoreFoundation/CFBundle.h>
}
#endif

struct LoadedLibInfo : public Plugin::LibraryInfo
{
#ifdef _WIN32
	HINSTANCE handle;
#elif defined(__APPLE__)
    CF::CFBundleRef handle;
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

/**
* 
	Retrieves information about available plugins

 */
class PluginManager {

public:

	/** Constructor */
	PluginManager();

	/** Destructor */
	~PluginManager();

	/** Loads all available plugins, using default OS-dependent plugin paths*/
	void loadAllPlugins();

	/** Loads plugins in a particular directory */
    void loadPlugins(const File &pluginPath);

	/** Loads a plugin at a particular path*/
	int loadPlugin(const String&);

	/** Unloads a plugin (not implemented yet) */
	//void unloadPlugin(Plugin *);

	/** Uploads all plugins (not implemented yet) */
	//void removeAllPlugins();

	/** Returns the total number of processor plugins*/
	int getNumProcessors() const;

	/** Returns the total number of data thread plugins*/
	int getNumDataThreads() const;

	/** Returns the total number of record engine plugins*/
	int getNumRecordEngines() const;
	
	/** Returns the total number of file source plugins*/
	int getNumFileSources() const;

	/** Returns info about a processor plugin at a given index */
	Plugin::ProcessorInfo getProcessorInfo(int index) const;

	/** Returns info about a processor plugin with a given name */
	Plugin::ProcessorInfo getProcessorInfo(String name, String libName = String()) const;

	/** Returns info about a data thread plugin at a given index */
	Plugin::DataThreadInfo getDataThreadInfo(int index) const;

	/** Returns info about a data thread plugin with a given name */
	Plugin::DataThreadInfo getDataThreadInfo(String name, String libName = String()) const;

	/** Returns info about a record engine plugin at a given index */
	Plugin::RecordEngineInfo getRecordEngineInfo(int index) const;

	/** Returns info about a record engine plugin with a given name */
	Plugin::RecordEngineInfo getRecordEngineInfo(String name, String libName = String()) const;

	/** Returns info about a file source plugin at a given index */
	Plugin::FileSourceInfo getFileSourceInfo(int index) const;

	/** Returns info about a file source plugin with a given name */
	Plugin::FileSourceInfo getFileSourceInfo(String name, String libName = String()) const;

	/** Returns the library name for a plugin at a given index */
	String getLibraryName(int index) const;

	/** Returns library version for a plugin at a given index */
	String getLibraryVersion(int index) const;

	/** Returns the library index based on a plugin type/index combination */
	int getLibraryIndexFromPlugin(Plugin::Type type, int index);

	/** Finds and removes the plugin with the specified library name from the Plugin Manager */
	bool removePlugin(String libName);

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

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginManager);
};

#endif
