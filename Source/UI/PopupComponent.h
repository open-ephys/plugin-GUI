#ifndef __POPUPCOMPONENT_H__
#define __POPUPCOMPONENT_H__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/PluginManager/OpenEphysPlugin.h"
#include "../Utils/Utils.h"
#include "../AccessClass.h"

class UIComponent;
class PopupComponent;

/**
 * 
 *  A manager class that keeps track of all the popups in the application
 * 
 */

class PopupManager {
public:

    PopupManager() {};

    ~PopupManager() {};

    void showPopup(std::unique_ptr<Component> popupComponent, Component* anchor);

    int getPopupStackSize() { return popupStack.size(); }

    String getActivePopup() { return popupStack.back(); }

protected:
    void onPopupDismissed(int result);

    std::vector<String> popupStack;
};


/**
 * 
 *  A popup component that can be used to display additional components in a callout box
 * 
 */

class PLUGIN_API PopupComponent : public Component, public ComponentListener
{

public:
    PopupComponent(Component* parent);
    virtual ~PopupComponent();

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

    void focusOfChildComponentChanged(FocusChangeType cause) override
    {
        if (isShowing())
            this->grabKeyboardFocus();
    }

    void componentBeingDeleted(Component& component) override
    {
        findParentComponentOfClass<CallOutBox>()->exitModalState(0);
        parent = nullptr;
    }

    virtual void updatePopup() = 0;

private:

    bool isOpen;

    Component* parent;

    UndoManager* undoManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PopupComponent)

};

#endif  // __POPUPCOMPONENT_H__
