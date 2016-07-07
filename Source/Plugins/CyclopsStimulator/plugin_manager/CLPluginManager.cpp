#include "CLPluginManager.h"
#include <iostream>
#include <cstdio>
#if !defined(WIN32) && !defined(__APPLE__)
#include <dlfcn.h>
#include <execinfo.h>
#endif

#ifdef WIN32
    static inline void closeHandle(HINSTANCE handle)
#elif defined(__APPLE__)
    static inline void closeHandle(CFBundleRef handle)
#else
    static inline void closeHandle(void* handle)
#endif
{
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


namespace cyclops{

void CyclopsPluginManager::loadPlugins(const File &pluginPath) {
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
        std::cout << "Loading Plugin: " << foundDLLs[i].getFileNameWithoutExtension() << "... " << std::flush;
        if (loadPlugin(foundDLLs[i].getFullPathName()))
        {
            std::cout << "Loaded" << std::endl;
        }
        else
        {
            std::cout << " DLL Load FAILED" << std::endl;
        }
        std::cout << std::endl;
    }
}

void CyclopsPluginManager::loadAllPlugins()
{
    Array<File> paths;
    
#ifdef __APPLE__
    paths.add(File::getSpecialLocation(File::currentApplicationFile).getChildFile("Contents/PlugIns"));
    paths.add(File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Application Support/open-ephys/PlugIns"));
#else
    paths.add(File::getSpecialLocation(File::currentApplicationFile).getParentDirectory().getChildFile("cyclops_plugins"));
#endif

    for (auto &pluginPath : paths) {
        if (!pluginPath.isDirectory()) {
            std::cout << "Plugin path not found: " << pluginPath.getFullPathName() << std::endl;
        } else {
            loadPlugins(pluginPath);
        }
    }
}

/**
 * @brief      Takes the user-specified plugin and begins dynamic loading
 *             process.
 * @details    We want to ensure that no step is exectued without a checkpoint
 *             because dynamic loading calls for rellocation of RAM works
 *             inside the same POSIX thread as the GUI.
 *
 * @return     { description_of_the_return_value }
 */
int CyclopsPluginManager::loadPlugin(const String& pluginLoc) {
    /*
    Load in the selected processor. This takes the dynamic object (.so) and
    copies it into RAM Dynamic linker requires a C-style string, so we we have
    to convert first.
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
    Changing this to resolve all variables immediately upon loading. This will
    provide for quicker testing of the custom processor stability and to ensure
    that it doesn't crash due to memory mishaps.
    */
    void *handle = 0;
    handle = dlopen(processorLocCString, RTLD_GLOBAL|RTLD_NOW);
#endif

    if (!handle) {
        ERROR_MSG("Failed to load plugin DLL");
        closeHandle(handle);
        return -1;
    }

//     LibraryInfoFunction infoFunction = 0;
// #ifdef WIN32
//     infoFunction = (LibraryInfoFunction)GetProcAddress(handle, "getLibInfo");
// #elif defined(__APPLE__)
//     infoFunction = (LibraryInfoFunction)CFBundleGetFunctionPointerForName(handle, CFSTR("getLibInfo"));
// #else
//     dlerror();
//     infoFunction = (LibraryInfoFunction)(dlsym(handle, "getLibInfo"));
// #endif

//     if (!infoFunction)
//     {
//         ERROR_MSG("Failed to load function 'getLibInfo'");
//         closeHandle(handle);
//         return -1;
//     }

//     Plugin::LibraryInfo libInfo;
//     infoFunction(&libInfo);

//     if (libInfo.apiVersion != PLUGIN_API_VER)
//     {
//         std::cerr << pluginLoc << " invalid version" << std::endl;
//         closeHandle(handle);
//         return -1;
//     }

    CLPluginInfoFunction piFunction = nullptr;
#ifdef WIN32
    piFunction = (CLPluginInfoFunction)GetProcAddress(handle, "getCyclopsPluginInfo");
#elif defined(__APPLE__ )
    piFunction = (CLPluginInfoFunction)CFBundleGetFunctionPointerForName(handle, CFSTR("getCyclopsPluginInfo"));
#else
    dlerror();
    piFunction = (CLPluginInfoFunction)(dlsym(handle, "getCyclopsPluginInfo"));
#endif

    if (piFunction == nullptr)
    {
        ERROR_MSG("Failed to load function 'getCyclopsPluginInfo'");
        closeHandle(handle);
        return -1;
    }
    // Create an empty PluginInfo struct
    
    // std::map<std::string, CyclopsPluginInfo>::iterator new_map_elem = (pInfoMap[pInfo->name] = new CyclopsPluginInfo);
    // CyclopsPluginInfo& pInfo = new_map_elem->second;
    CyclopsPluginInfo pInfo;
    piFunction(pInfo);
    if (pInfo.sourceCount != (int)pInfo.sourceCodeNames.size()){
        std::cout << "sourceCount doest not match no. of \"Source-Code-Names\".\nFAILED to load plugin" << pInfo.Name << std::endl;
        return -1;
    }
    else if (pInfo.sourceCount < 1){
        std::cout << "No Sources (aka Signals) have been defined for the plugin!\nFAILED to load plugin" << pInfo.Name << std::endl;
        return -1;
    }
    // successful load, plugin seems valid.
    pInfoMap[pInfo.Name] = pInfo;
    return 1;
}

CyclopsPluginInfo* CyclopsPluginManager::getInfo(const std::string& pName)
{
    return &(pInfoMap[pName]);
}

int CyclopsPluginManager::getNumPlugins()
{
    return pInfoMap.size();
}

} // NAMESPACE cyclops