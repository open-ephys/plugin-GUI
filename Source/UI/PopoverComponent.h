#ifndef __POPOVERCOMPONENT_H__
#define __POPOVERCOMPONENT_H__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/PluginManager/OpenEphysPlugin.h"
#include "../Utils/Utils.h"

class PLUGIN_API PopoverComponent : public Component
{

public:
    PopoverComponent();
    virtual ~PopoverComponent();

    bool keyPressed(const KeyPress &key) override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PopoverComponent)

};

#endif  // __POPOVERCOMPONENT_H__
