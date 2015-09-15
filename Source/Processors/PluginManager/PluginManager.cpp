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

#include <iostream>
#include <stdio.h>
#ifdef WIN32
//simple hack to avoid to much conditionals
#define dlclose(h) FreeLibrary(h)
#else
#include <dlfcn.h>
#include <execinfo.h>
#endif

#include "PluginManager.h"
#include "../../UI/ProcessorList.h"


#ifdef WIN32
//And this one because Windows doesn't provide an error-to-string anything
static const char* dlerror(void)
{
	static char buf[32];
	DWORD ret = GetLastError();
	if (!ret) return NULL;
	sprintf(buf, "DLL Error 0x%x",ret);
	return buf;
}
#define ERROR_MSG(fmt,...) do{fprintf(stderr,"%s:%d:",__FILE__,__LINE__); fprintf(stderr,fmt,__VA_ARGS__);} while(0)
#else
#define ERROR_MSG(fmt,args...) do{fprintf(stderr,"%s:%d:",__FILE__,__LINE__); fprintf(stderr,fmt,## args);} while(0)
#endif

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
}

/*
	 Takes the user-specified plugin and begins
	 dynamic loading process. We want to ensure that
	 no step is exectured without a checkpoint
	 because dynamic loading calls for rellocation of RAM
	 and works inside the same POSIX thread as the GUI.
 */

int PluginManager::loadPlugin(const String& pluginLoc) {
	/*
	Load in the selected processor. This takes the
	dynamic object (.so) and copies it into RAM
	Dynamic linker requires a C-style string, so we
	we have to convert first.
	*/
	const char* processorLocCString = static_cast<const char*>(pluginLoc.toUTF8());

#ifndef WIN32
	// Clear errors
	dlerror();

	/*
	Changing this to resolve all variables immediately upon loading.
	This will provide for quicker testing of the custom
	processor stability and to ensure that it doesn't crash due
	to memory mishaps.
	*/
	void *handle = 0;
	handle = dlopen(processorLocCString,RTLD_GLOBAL|RTLD_LAZY);
#else
	HINSTANCE handle;
	handle = LoadLibrary(processorLocCString);
#endif

	if (!handle) {
		ERROR_MSG("%s\n", dlerror());
		dlclose(handle);
		return -1;
	}
	dlerror();

	LibraryInfoFunction infoFunction = 0;
#ifdef WIN32
	infoFunction = (LibraryInfoFunction)GetProcAddress(handle, "getLibInfo");
#else
	infoFunction = (LibraryInfoFunction)(dlsym(handle, "getLibInfo"));
#endif

	if (!infoFunction)
	{
		ERROR_MSG("%s\n", dlerror());
		dlclose(handle);
		return -1;
	}
	dlerror();

	Plugin::LibraryInfo libInfo;
	infoFunction(&libInfo);

	if (libInfo.apiVersion != PLUGIN_API_VER)
	{
		std::cerr << pluginLoc << " invalid version" << std::endl;
		dlclose(handle);
		return -1;
	}

	PluginInfoFunction piFunction = 0;
#ifdef WIN32
	piFunction = (PluginInfoFunction)GetProcAddress(handle, "getPluginInfo");
#else
	piFunction = (PluginInfoFunction)(dlsym(handle, "getPluginInfo"));
#endif

	if (!piFunction)
	{
		ERROR_MSG("%s\n", dlerror());
		dlclose(handle);
		return -1;
	}
	dlerror();

	LoadedLibInfo lib;
	lib.apiVersion = libInfo.apiVersion;
	strcpy(lib.name,libInfo.name);
	lib.numPlugins = libInfo.numPlugins;
	lib.handle = handle;

	libArray.add(lib);

	Plugin::PluginInfo pInfo;
	for (int i = 0; i < lib.numPlugins; i++)
	{
		if (piFunction(i, &pInfo)) //if somehow there are less plugins than stated, stop adding
			break;
		switch (pInfo.type)
		{
		case Plugin::ProcessorPlugin:
			LoadedPluginInfo<Plugin::ProcessorInfo> info;
			info.creator = pInfo.processor.creator;
			strcpy(info.name, pInfo.processor.name);
			info.type = pInfo.processor.type;
			info.libIndex = libArray.size();
			processorPlugins.add(info);
			break;
		case Plugin::RecordEnginePlugin:
			break;
		case Plugin::DatathreadPlugin:
			break;
		case Plugin::FileSourcePlugin:
			break;
		}
	}
	AccessClass::getProcessorList()->fillItemList();
	return 0;
}

int PluginManager::getNumProcessors()
{
	return processorPlugins.size();
}

Plugin::ProcessorInfo PluginManager::getProcessorInfo(int index)
{
	return processorPlugins[index];
}


#if 0
PluginManager::Plugin::Plugin() {
}

PluginManager::Plugin::~Plugin() {
}

/*
	 Allows user to select custom-compiled processor for
	 loading into the GUI.
 */


void PluginManager::Manager::unloadPlugin(PluginManager::Plugin *processor) {
	if (!processor) {
		ERROR_MSG("PluginManager::unloadPlugin: Invalid processor\n");
		return;
	}
#ifdef WIN32
	HINSTANCE handle;
#else
	void *handle = 0;
#endif
	handle = processor->processorHandle;
	dlclose(handle);
	removeListPlugin(processor);
}

void PluginManager::Manager::insertListPlugin(PluginManager::Plugin *processor) {
	std::cout << "Size of list before is: " << pluginList.size() << std::endl;
	if(!processor) {
		ERROR_MSG("PluginManager::insertListPlugin: Invalid processor.\n");
		return;
	}
	pluginList.push_back(processor);
	AccessClass::getProcessorList()->addPluginItem(String("test"), size_t(0x1));
	std::cout << "Size of list after is: " << pluginList.size() << std::endl;
}

void PluginManager::Manager::removeListPlugin(PluginManager::Plugin *processor) {
	std::cout << "Size of list before is: " << pluginList.size() << std::endl;
	if(!processor) {
		ERROR_MSG("PluginManager::removeListPlugin: Invalid processor.\n");
		return;
	}
	pluginList.remove(processor);
	std::cout << "Size of list after is: " << pluginList.size() << std::endl;
}

void PluginManager::Manager::removeAllPlugins() {
#ifdef WIN32
	HINSTANCE handle;
#else
	void *handle;
#endif
	for(std::list<PluginManager::Plugin *>::iterator i = pluginList.begin(); i != pluginList.end(); i = pluginList.begin()) {
		std::cout << "Size of list before all is: " << pluginList.size() << std::endl;
		handle = (*i)->processorHandle;
		removeListPlugin(*i);
		delete *i;
		dlclose(handle);
		std::cout << "Size of list after all is: " << pluginList.size() << std::endl;
	}
}

PluginManager::Manager *PluginManager::Manager::instance = 0;
PluginManager::Manager *PluginManager::Manager::getInstance() {
	if (instance) {
		return instance;
	}

	if (!instance) {
		static Manager manager;
		instance = &manager;
	}
	return instance;
}
#endif