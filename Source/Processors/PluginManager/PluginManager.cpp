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
#if !defined(WIN32) && !defined(__APPLE__)
#include <dlfcn.h>
#include <execinfo.h>
#endif

#include "PluginManager.h"
#include "../../UI/ProcessorList.h"
#include "../../UI/ControlPanel.h"

#include "../../Utils/Utils.h"


static inline void closeHandle(decltype(LoadedLibInfo::handle) handle) {
    if (handle) {
#ifdef WIN32
        FreeLibrary(handle);
#elif defined(__APPLE__)
        CFRelease(handle);
#else
        dlclose(handle);
#endif
    }
}


static void errorMsg(const char *file, int line, const char *msg) {
    fprintf(stderr, "%s:%d: %s", file, line, msg);
    
#ifdef WIN32
    DWORD ret = GetLastError();
    if (ret) {
        fprintf(stderr, ": DLL Error 0x%x", ret);
    }
#elif defined(__APPLE__)
    // Any additional error messages are logged directly by the system
    // and are not available to the application
#else
    const char *error = dlerror();
    if (error) {
        fprintf(stderr, ": %s", error);
    }
#endif
    
    fprintf(stderr, "\n");
}

#define ERROR_MSG(msg) errorMsg(__FILE__, __LINE__, msg)


PluginManager::PluginManager()
{
#ifdef _WIN32

	String appDir = File::getSpecialLocation(File::currentApplicationFile).getFullPathName();

	//Shared directory at the same level as executable
	File sharedPath = File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("shared");
	//Shared directory managed by Plugin Installer at C:/ProgramData
	File installSharedPath = File::getSpecialLocation(File::commonApplicationDataDirectory).getChildFile("Open Ephys/shared");

	if(appDir.contains("plugin-GUI\\Build\\"))
	{
		SetDllDirectory(sharedPath.getFullPathName().toUTF8());
	}
	else
    {
		if (!installSharedPath.isDirectory())
        {
			LOGD("Copying shared dependencies to ", installSharedPath.getFullPathName());
            sharedPath.copyDirectoryTo(installSharedPath);
        }
        SetDllDirectory(installSharedPath.getFullPathName().toUTF8());
    }

#elif __linux__
	File installSharedPath = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(".open-ephys/shared");
	if (!installSharedPath.isDirectory()) {
        installSharedPath.createDirectory();
    }
#endif
}

PluginManager::~PluginManager()
{
}


void PluginManager::loadAllPlugins()
{
    Array<File> paths;
    
#ifdef __APPLE__
    paths.add(File::getSpecialLocation(File::currentApplicationFile).getChildFile("Contents/PlugIns"));
    paths.add(File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Application Support/open-ephys/plugins"));
#elif _WIN32
	paths.add(File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("plugins"));

    String appDir = File::getSpecialLocation(File::currentApplicationFile).getFullPathName();
    if(!appDir.contains("plugin-GUI\\Build\\"))
	    paths.add(File::getSpecialLocation(File::commonApplicationDataDirectory).getChildFile("Open Ephys/plugins"));
#else
	paths.add(File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("plugins"));

    String appDir = File::getSpecialLocation(File::currentApplicationFile).getFullPathName();
    if(!appDir.contains("plugin-GUI/Build/"))
	    paths.add(File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile(".open-ephys/plugins"));	
#endif

    for (auto &pluginPath : paths) {
        if (!pluginPath.isDirectory()) {
			LOGD("Plugin path not found: ", pluginPath.getFullPathName(), "\nCreating new plugins directory...");
			pluginPath.createDirectory();
        } else {
            loadPlugins(pluginPath);
        }
    }
}

void PluginManager::loadPlugins(const File &pluginPath) {
    Array<File> foundDLLs;
    
#ifdef WIN32
    String pluginExt("*.dll");
#elif defined(__APPLE__)
    String pluginExt("*.bundle");
#else
    String pluginExt("*.so");
#endif
    
#ifdef __APPLE__
    pluginPath.findChildFiles(foundDLLs, File::findDirectories, false, pluginExt);
#else
	pluginPath.findChildFiles(foundDLLs, File::findFiles, true, pluginExt);
#endif

	for (int i = 0; i < foundDLLs.size(); i++)
	{
		LOGD("Loading Plugin: ", foundDLLs[i].getFileNameWithoutExtension(), "... ");
		int res = loadPlugin(foundDLLs[i].getFullPathName());
		if (res < 0)
		{
			LOGD(" DLL Load FAILED");
		}
		else
		{
			LOGDD("Loaded with ", res, " plugins");
		}
	}
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

#ifdef WIN32
	HINSTANCE handle;
	handle = LoadLibrary(processorLocCString);
#elif defined(__APPLE__)
    CFURLRef bundleURL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
                                                                 reinterpret_cast<const UInt8 *>(processorLocCString),
                                                                 strlen(processorLocCString),
                                                                 true);
    assert(bundleURL);
    CFBundleRef handle = CFBundleCreate(kCFAllocatorDefault, bundleURL);
    CFRelease(bundleURL);
#else
	// Clear errors
	dlerror();

	/*
	Changing this to resolve all variables immediately upon loading.
	This will provide for quicker testing of the custom
	processor stability and to ensure that it doesn't crash due
	to memory mishaps.
	*/
	void *handle = 0;
	handle = dlopen(processorLocCString,RTLD_GLOBAL|RTLD_NOW);
#endif

	if (!handle) {
		ERROR_MSG("Failed to load plugin DLL");
		closeHandle(handle);
		return -1;
	}

	LibraryInfoFunction infoFunction = 0;
#ifdef WIN32
	infoFunction = (LibraryInfoFunction)GetProcAddress(handle, "getLibInfo");
#elif defined(__APPLE__)
    infoFunction = (LibraryInfoFunction)CFBundleGetFunctionPointerForName(handle, CFSTR("getLibInfo"));
#else
    dlerror();
	infoFunction = (LibraryInfoFunction)(dlsym(handle, "getLibInfo"));
#endif

	if (!infoFunction)
	{
		ERROR_MSG("Failed to load function 'getLibInfo'");
		closeHandle(handle);
		return -1;
	}

	Plugin::LibraryInfo libInfo;
	infoFunction(&libInfo);

	if (libInfo.apiVersion != PLUGIN_API_VER)
	{
		std::cerr << pluginLoc << " invalid version" << std::endl;
		closeHandle(handle);
		return -1;
	}

	PluginInfoFunction piFunction = 0;
#ifdef WIN32
	piFunction = (PluginInfoFunction)GetProcAddress(handle, "getPluginInfo");
#elif defined(__APPLE__)
    piFunction = (PluginInfoFunction)CFBundleGetFunctionPointerForName(handle, CFSTR("getPluginInfo"));
#else
    dlerror();
	piFunction = (PluginInfoFunction)(dlsym(handle, "getPluginInfo"));
#endif

	if (!piFunction)
	{
        ERROR_MSG("Failed to load function 'getPluginInfo'");
		closeHandle(handle);
		return -1;
	}

	LoadedLibInfo lib;
	lib.apiVersion = libInfo.apiVersion;
	lib.name = libInfo.name;
	lib.libVersion = libInfo.libVersion;
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
		case Plugin::PLUGIN_TYPE_PROCESSOR:
		{
			LoadedPluginInfo<Plugin::ProcessorInfo> info;
			info.creator = pInfo.processor.creator;
			info.name = pInfo.processor.name;
			info.type = pInfo.processor.type;
			info.libIndex = libArray.size()-1;
			Plugin::ProcessorInfo pi = getProcessorInfo(String::fromUTF8(info.name));
			if(pi.name == nullptr)
				processorPlugins.add(info);
			break;
		}
		case Plugin::PLUGIN_TYPE_RECORD_ENGINE:
		{
			LoadedPluginInfo<Plugin::RecordEngineInfo> info;
			info.creator = pInfo.recordEngine.creator;
			info.name = pInfo.recordEngine.name;
			info.libIndex = libArray.size() - 1;
			Plugin::RecordEngineInfo rei = getRecordEngineInfo(String::fromUTF8(info.name));
			if(rei.name == nullptr)
				recordEnginePlugins.add(info);
			break;
		}
		case Plugin::PLUGIN_TYPE_DATA_THREAD:
		{
			LoadedPluginInfo<Plugin::DataThreadInfo> info;
			info.creator = pInfo.dataThread.creator;
			info.name = pInfo.dataThread.name;
			info.libIndex = libArray.size() - 1;
			Plugin::DataThreadInfo dti = getDataThreadInfo(String::fromUTF8(info.name));
			if(dti.name == nullptr)
				dataThreadPlugins.add(info);
			break;
		}
		case Plugin::PLUGIN_TYPE_FILE_SOURCE:
		{
			LoadedPluginInfo<Plugin::FileSourceInfo> info;
			info.creator = pInfo.fileSource.creator;
			info.name = pInfo.fileSource.name;
			info.extensions = pInfo.fileSource.extensions;
			info.libIndex = libArray.size();
			Plugin::FileSourceInfo fsi = getFileSourceInfo(String::fromUTF8(info.name));
			if(fsi.name == nullptr)
				fileSourcePlugins.add(info);
			break;
		}
		default:
		{
			std::cerr << pluginLoc << " invalid plugin type: " << pInfo.type << std::endl;
			break;
		}
		}
	}
	return lib.numPlugins;
}

int PluginManager::getNumProcessors() const
{
	return processorPlugins.size();
}

int PluginManager::getNumDataThreads() const
{
	return dataThreadPlugins.size();
}

int PluginManager::getNumRecordEngines() const
{
	return recordEnginePlugins.size();
}

int PluginManager::getNumFileSources() const
{
	return fileSourcePlugins.size();
}

Plugin::ProcessorInfo PluginManager::getProcessorInfo(int index) const
{
	if (index < processorPlugins.size())
		return processorPlugins[index];
	else
		return getEmptyProcessorInfo();
}

Plugin::DataThreadInfo PluginManager::getDataThreadInfo(int index) const
{
	if (index < dataThreadPlugins.size())
		return dataThreadPlugins[index];
	else
		return getEmptyDatathreadInfo();
}

Plugin::RecordEngineInfo PluginManager::getRecordEngineInfo(int index) const
{
	if (index < recordEnginePlugins.size())
		return recordEnginePlugins[index];
	else 
		return getEmptyRecordengineInfo();
}

Plugin::FileSourceInfo PluginManager::getFileSourceInfo(int index) const
{
	if (index < fileSourcePlugins.size())
		return fileSourcePlugins[index];
	else
		return getEmptyFileSourceInfo();
}

Plugin::ProcessorInfo PluginManager::getProcessorInfo(String name, String libName) const
{
	Plugin::ProcessorInfo i = getEmptyProcessorInfo();
	findPlugin<Plugin::ProcessorInfo>(name, libName, processorPlugins, i);
	return i;
}

Plugin::DataThreadInfo PluginManager::getDataThreadInfo(String name, String libName) const
{
	Plugin::DataThreadInfo i = getEmptyDatathreadInfo();
	findPlugin<Plugin::DataThreadInfo>(name, libName, dataThreadPlugins, i);
	return i;
}

Plugin::RecordEngineInfo PluginManager::getRecordEngineInfo(String name, String libName) const
{
	Plugin::RecordEngineInfo i = getEmptyRecordengineInfo();
	findPlugin<Plugin::RecordEngineInfo>(name, libName, recordEnginePlugins, i);
	return i;
}

Plugin::FileSourceInfo PluginManager::getFileSourceInfo(String name, String libName) const
{
	Plugin::FileSourceInfo i = getEmptyFileSourceInfo();
	findPlugin<Plugin::FileSourceInfo>(name, libName, fileSourcePlugins, i);
	return i;
}

String PluginManager::getLibraryName(int index) const
{
	if (index < 0 || index >= libArray.size())
		return String::empty;
	else
		return libArray[index].name;
}

int PluginManager::getLibraryVersion(int index) const
{
	if (index < 0 || index >= libArray.size())
		return -1;
	else
		return libArray[index].libVersion;
}

int PluginManager::getLibraryIndexFromPlugin (Plugin::PluginType type, int index)
{
    switch (type)
    {
        case Plugin::PLUGIN_TYPE_PROCESSOR:
            return processorPlugins[index].libIndex;
        case Plugin::PLUGIN_TYPE_RECORD_ENGINE:
            return recordEnginePlugins[index].libIndex;
        case Plugin::PLUGIN_TYPE_DATA_THREAD:
            return dataThreadPlugins[index].libIndex;
        case Plugin::PLUGIN_TYPE_FILE_SOURCE:
            return fileSourcePlugins[index].libIndex;
        default:
            return -1;
    }
}

Plugin::ProcessorInfo PluginManager::getEmptyProcessorInfo()
{
	Plugin::ProcessorInfo i;
	i.creator = nullptr;
	i.name = nullptr;
	i.type = Plugin::InvalidProcessor;
	return i;
}

Plugin::DataThreadInfo PluginManager::getEmptyDatathreadInfo()
{
	Plugin::DataThreadInfo i;
	i.creator = nullptr;
	i.name = nullptr;
	return i;
}

Plugin::RecordEngineInfo PluginManager::getEmptyRecordengineInfo()
{
	Plugin::RecordEngineInfo i;
	i.creator = nullptr;
	i.name = nullptr;
	return i;
}

Plugin::FileSourceInfo PluginManager::getEmptyFileSourceInfo()
{
	Plugin::FileSourceInfo i;
	i.creator = nullptr;
	i.name = nullptr;
	return i;
}

template<class T>
bool PluginManager::findPlugin(String name, String libName, const Array<LoadedPluginInfo<T>>& pluginArray, T& pluginInfo) const
{
	for (int i = 0; i < pluginArray.size(); i++)
	{
		String pName = String(pluginArray[i].name);
		if (pName == name)
		{
			if ((libName.isEmpty()) || (libName == String(libArray[pluginArray[i].libIndex].name)))
			{
				pluginInfo = pluginArray[i];
				return true;
			}
		}
	}
	return false;
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
		ERROR_MSG("PluginManager::unloadPlugin: Invalid processor");
		return;
	}
#ifdef WIN32
	HINSTANCE handle;
#elif defined(__APPLE__)
    CFBundleRef handle;
#else
	void *handle = 0;
#endif
	handle = processor->processorHandle;
	closeHandle(handle);
	removeListPlugin(processor);
}

void PluginManager::Manager::insertListPlugin(PluginManager::Plugin *processor) {
	LOGD("Size of list before is: ", pluginList.size());
	if(!processor) {
		ERROR_MSG("PluginManager::insertListPlugin: Invalid processor.");
		return;
	}
	pluginList.push_back(processor);
	AccessClass::getProcessorList()->addPluginItem(String("test"), size_t(0x1));
	LOGD("Size of list after is: ", pluginList.size());
}

void PluginManager::Manager::removeListPlugin(PluginManager::Plugin *processor) {	
	LOGD("Size of list before is: ", pluginList.size());
	if(!processor) {
		ERROR_MSG("PluginManager::removeListPlugin: Invalid processor.");
		return;
	}
	pluginList.remove(processor);
	LOGD("Size of list after is: ", pluginList.size());
}

void PluginManager::Manager::removeAllPlugins() {
#ifdef WIN32
	HINSTANCE handle;
#elif defined(__APPLE__)
    CFBundleRef handle;
#else
	void *handle;
#endif
	for(std::list<PluginManager::Plugin *>::iterator i = pluginList.begin(); i != pluginList.end(); i = pluginList.begin()) {
		LOGD("Size of list before all is: ", pluginList.size());
		handle = (*i)->processorHandle;
		removeListPlugin(*i);
		delete *i;
		closeHandle(handle);
		LOGD("Size of list after all is: ", pluginList.size());
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