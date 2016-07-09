#include "../plugin_manager/CyclopsPluginInfo.h"
#include "Example_CL_Plugin.h"
#include <string>
#include <vector>
#ifdef WIN32
#include <Windows.h>
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
//using namespace cyclops;

cyclops::CyclopsPlugin* maker_function()
{
    return new Example_CL_Plugin;
}

/**
 * @brief      Gets the cyclops plugin information.
 * @details    Here you have to specify the no. of LED channels that this plugin
 *             would control, the no. of "source" objects that need to be
 *             pre-loaded onto the Teensy (for this plugin).
 *
 *             Also specify the "names" of the Sources, this will be shown on
 *             the Cyclops-Stimulator canvas, where you can map these "names"
 *             with _actual_ Signals.
 *
 * @param      infoStruct  The information structure which needs to be filled.
 */
extern "C" EXPORT void getCyclopsPluginInfo(cyclops::CyclopsPluginInfo& infoStruct) {
    infoStruct.Name = "Example_CL_Plugin"; // Names of your Cyclops Plugin. This will appear on the GUI.
    infoStruct.channelCount = 1;  // The no. of LED channels that will be controlled.
    infoStruct.sourceCount = 4; // The no. of Sources needed on the Teensy, should be same as length of the vector below.
    infoStruct.sourceCodeNames = {"FastSquare", "SlowSquare", "Triangle", "Sawtooth"}; // this will copy, so it's safe.
    infoStruct.CyclopsPluginFactory = maker_function;
}

