#ifndef __POPOVERCOMPONENT_H__
#define __POPOVERCOMPONENT_H__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/PluginManager/OpenEphysPlugin.h"
#include "../Utils/Utils.h"
#include "../AccessClass.h"

class UIComponent;
class PopoverComponent;

/**
 * 
 *  A manager class that keeps track of all the popovers in the application
 * 
 */

class PopoverManager {
public:
    void showPopover(std::unique_ptr<Component> popoverComponent, Component* anchor) {
        auto& myBox = juce::CallOutBox::launchAsynchronously(std::move(popoverComponent),
                                                             anchor->getScreenBounds(),
                                                             nullptr);

        juce::ModalComponentManager::getInstance()->attachCallback(&myBox, juce::ModalCallbackFunction::create([this](int result) {
            onPopoverDismissed(result);
        }));
    }

protected:
    void onPopoverDismissed(int result) {
        LOGD("*** PopupoverConfigurationWindow closed");
    }
};


/**
 * 
 *  A popover component that can be used to display additional components in a callout box
 * 
 */

class PLUGIN_API PopoverComponent : public Component
{

public:
    PopoverComponent(Component* parent);
    virtual ~PopoverComponent();

    Component* findComponentByIDRecursive(Component* parent, const String& componentID);

    bool keyPressed(const KeyPress &key) override;

    void setUndoManager(UndoManager* manager)
    {
        undoManager = manager;
    }

    UndoManager* getUndoManager()
    {
        return undoManager;
    }

    void setParent(Component* parent_)
    {
        parent = parent_;
    }

    Component* getParent()
    {
        return parent;
    }

    void setOpen(bool open)
    {
        isOpen = open;
    }

    bool getOpen()
    {
        return isOpen;
    }

private:

    bool isOpen;

    Component* parent;

    UndoManager* undoManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PopoverComponent)

};

#endif  // __POPOVERCOMPONENT_H__
