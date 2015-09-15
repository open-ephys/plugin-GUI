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
	int loadPlugin(const String&);
	//void unloadPlugin(Plugin *);
	void removeAllPlugins();
	int getNumProcessors();
	Plugin::ProcessorInfo getProcessorInfo(int index);

private:
	Array<LoadedLibInfo> libArray;
	Array<LoadedPluginInfo<Plugin::ProcessorInfo>> processorPlugins;
	Array<LoadedPluginInfo<Plugin::PluginInfo>> pp;
	
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
