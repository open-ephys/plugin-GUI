#ifndef EXAMPLE_CL_PLUGIN_H
#define EXAMPLE_CL_PLUGIN_H

#include "../../plugin_manager/CyclopsPluginInfo.h"

/*
 * Now list the same names here in this enum, prepended with an underscore, if
 * possible. To use the enum, you must scope it like
 * sourceAlias::FastSquare
 * see documentation on enum classes for more clarity.
 */
enum class sourceAlias : int
{
    FastSquare,
    SlowSquare,
    Triangle,
    Sawtooth
};
/*
 * You must keep the names in same order as that in source_code_names. Then you
 * can use the enum to refer to the correct object instead of using interger
 * index literals, making your code easier to read and modify.
 * You can keep this enum, rename it, or remove it altogether, it doesn't matter.
 */

class Example_CL_Plugin : public cyclops::CyclopsPlugin
{
public:
    Example_CL_Plugin();
    void handleEvent(int eventType, MidiMessage& event, int samplePosition = 0);
};

#endif