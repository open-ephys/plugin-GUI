/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __POPUPCOMPONENT_H__
#define __POPUPCOMPONENT_H__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../AccessClass.h"
#include "../Processors/PluginManager/OpenEphysPlugin.h"
#include "../Utils/Utils.h"

class UIComponent;
class PopupComponent;

/**
 * 
 *  A manager class that keeps track of all the popups in the application
 * 
 */

class PLUGIN_API PopupManager
{
public:
    PopupManager() {};

    ~PopupManager() {};

    void showPopup (std::unique_ptr<Component> popupComponent, Component* anchor);

    int getPopupStackSize() { return int(popupStack.size()); }

    String getActivePopup() { return popupStack.back(); }

protected:
    void onPopupDismissed (int result);

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
    PopupComponent (Component* parent);
    virtual ~PopupComponent();

    Component* findComponentByIDRecursive (Component* parent, const String& componentID);

    bool keyPressed (const KeyPress& key) override;

    void setUndoManager (UndoManager* manager)
    {
        undoManager = manager;
    }

    UndoManager* getUndoManager()
    {
        return undoManager;
    }

    void setParent (Component* parent_)
    {
        parent = parent_;
    }

    Component* getParent()
    {
        return parent;
    }

    void setOpen (bool open)
    {
        isOpen = open;
    }

    bool getOpen()
    {
        return isOpen;
    }

    void focusOfChildComponentChanged (FocusChangeType cause) override
    {
        if (isShowing())
            this->grabKeyboardFocus();
    }

    void componentBeingDeleted (Component& component) override
    {
        findParentComponentOfClass<CallOutBox>()->exitModalState (0);
        parent = nullptr;
    }

    virtual void updatePopup() = 0;

private:
    bool isOpen;

    Component* parent;

    UndoManager* undoManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PopupComponent)
};

#endif // __POPUPCOMPONENT_H__
