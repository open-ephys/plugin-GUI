#include "CyclopsPlugin.h"

namespace cyclops {

CyclopsPlugin::CyclopsPlugin(CyclopsPluginInfo* info_struct)
{
    info = info_struct; // this is owned by cyclops-stimulator processor, so it's safe
}

CyclopsPlugin::setChannels(const int channelIDs[]){
    Channels.clearQuick();
    Channels.addArray(channelIDs, info->channelCount);
}

//bool isReady();
//bool enable();
//bool disable();

} // NAMESPACE cyclops
