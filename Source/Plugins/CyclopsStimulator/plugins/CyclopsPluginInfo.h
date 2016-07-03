#ifndef CYCLOPS_STIMULATOR_PLUGIN_H
#define CYCLOPS_STIMULATOR_PLUGIN_H
/*+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                             CyclopsPluginInfo                                      |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

#include "../../../../JuceLibraryCode/JuceHeader.h"

namespace cyclops{

struct CyclopsPluginInfo
{
    String Name;
    int sourceCount;
    int channelCount;
    StringArray sourceCodeNames;
};

// This definition will move into CyclopsPluginLib.cpp. It shall not be declared
// here
//
// @param      infoStruct  This will be filled.
//
extern void getCyclopsPluginInfo(CyclopsPluginInfo* infoStruct);

} // NAMESPACE cyclops