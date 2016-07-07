#ifndef CL_PLUGIN_MANAGER_H
#define CL_PLUGIN_MANAGER_H

#include "CyclopsPlugin.h" // includes JUCE headers
#include "CyclopsPluginInfo.h"
#include <map>
#include <string>

namespace cyclops{

class CyclopsPluginManager
{
public:
    void loadAllPlugins();
    void loadPlugins(const File &pluginPath);
    int loadPlugin(const String&);
    void removeAllPlugins();
    CyclopsPluginInfo* getInfo(const std::string& pName); // this is not same as getCLPluginInfo()
    int getNumPlugins();
private:
    std::map<std::string, CyclopsPluginInfo> pInfoMap;
};

} // NAMESPACE cyclops

#endif