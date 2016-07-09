#include "CyclopsPlugin.h"
#include "CyclopsPluginInfo.h"

namespace cyclops
{

CyclopsPlugin::CyclopsPlugin() : readiness(0)
{
    std::cout<<"Made";
}

void CyclopsPlugin::setChannels(const int channelIDs[]){
    Channels.clearQuick();
    Channels.addArray(channelIDs, info->channelCount);
}

bool isReady()
{
    return true;
}

bool enable()
{
    return true;
}

bool disable()
{
    return true;
}

} // NAMESPACE cyclops
