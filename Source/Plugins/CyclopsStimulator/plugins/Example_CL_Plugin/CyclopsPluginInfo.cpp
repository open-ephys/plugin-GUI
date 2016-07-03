#include "../CyclopsPluginInfo.h"
#include "Example_CL_Plugin.h"

//using namespace cyclops;

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
void cyclops::getCyclopsPluginInfo(CyclopsPluginInfo* infoStruct){
    infoStruct->Name = "Example_CL_Plugin"; // Names of your Cyclops Plugin. This will appear on the GUI.
    infoStruct->channelCounts = 1;  // The no. of LED channels that will be controlled.
    infoStruct->sourceCounts = 4; // The no. of Sources needed on the Teensy.

    StringArray source_code_names = {"FastSquare", "SlowSquare", "Triangle", "Sawtooth"};
    infoStruct->sourceCodeNames = source_code_names; // this will copy, so it's safe.
}

/*
 * Now list the same names here in this enum, prepended with an underscore, if
 * possible. The underscore is just to make sure that your words have not
 * already been used in some part of this program. It will also serve as an
 * indication that this is an enum keyword.
 */
enum cyclops::sourceAlias // defined from the (empty) declaration in CyclopsPlugin.h
{
    _FastSquare,
    _SlowSquare,
    _Triangle,
    _Sawtooth
}
/*
 * You must keep the names in same order as that in source_code_names. Then you
 * can use the enum to refer to the correct object instead of using interger
 * index literals, making your code easier to read and modify.
 *
 * I could set up a std::map<string, int> or something like that, but it would
 * make everthing slower, and the current approach is faster.
 */