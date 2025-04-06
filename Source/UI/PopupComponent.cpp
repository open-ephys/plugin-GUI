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

#include "PopupComponent.h"
#include "EditorViewport.h"
#include "UIComponent.h"

#include "../CoreServices.h"

void PopupManager::showPopup (std::unique_ptr<Component> popupComponent, Component* anchor)
{
    String componentID = anchor->getComponentID();

    auto& myBox = juce::CallOutBox::launchAsynchronously (std::move (popupComponent),
                                                          anchor->getScreenBounds(),
                                                          nullptr);

    myBox.setDismissalMouseClicksAreAlwaysConsumed (true);

    LOGD ("PopupManager adding: " + componentID);

    popupStack.push_back (componentID);

    juce::ModalComponentManager::getInstance()->attachCallback (&myBox, juce::ModalCallbackFunction::create ([this] (int result)
                                                                                                             { onPopupDismissed (result); }));
}

void PopupManager::onPopupDismissed (int result)
{
    if (popupStack.size() > 0)
    {
        String componentID = popupStack.back();
        popupStack.pop_back();
        LOGD ("PopupManager closed: " + componentID);
    }
    else
    {
        LOGD ("PopupManager: no popups to remove.");
    }
}

PopupComponent::PopupComponent (Component* parent_) : parent (parent_)
{
    undoManager = CoreServices::getUndoManager();

    setWantsKeyboardFocus (true);
    parent->addComponentListener (this);
}

PopupComponent::~PopupComponent()
{
    if (parent != nullptr)
    {
        parent->removeComponentListener (this);
    }
}

bool PopupComponent::keyPressed (const KeyPress& key)
{
    if (key == KeyPress ('z', ModifierKeys::commandModifier, 0))
    {
        LOGD (" [PopupComponent] Undoing action");

        if (! undoManager->canUndo())
            return false;

        if (Component::getNumCurrentlyModalComponents() > 1)
        {
            findParentComponentOfClass<CallOutBox>()->exitModalState (0);
            return false;
        }

        if (CoreServices::getAcquisitionStatus()
            && undoManager->getUndoDescription().contains ("Disabled during acquisition"))
            return false;

        if (AccessClass::getEditorViewport()->isSignalChainLocked())
            return false;

        undoManager->undo();

        if (parent != nullptr)
            updatePopup();

        return true;
    }
    else if (key == KeyPress ('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0))
    {
        LOGD (" [PopupComponent] Redoing action");

        if (! undoManager->canRedo())
            return false;

        if (Component::getNumCurrentlyModalComponents() > 1)
        {
            findParentComponentOfClass<CallOutBox>()->exitModalState (0);
            return false;
        }

        if (CoreServices::getAcquisitionStatus()
            && undoManager->getRedoDescription().contains ("Disabled during acquisition"))
            return false;

        if (AccessClass::getEditorViewport()->isSignalChainLocked())
            return false;

        undoManager->redo();

        if (parent != nullptr)
            updatePopup();

        return true;
    }
    else if (key == KeyPress (KeyPress::escapeKey, 0, 0))
    {
        findParentComponentOfClass<CallOutBox>()->exitModalState (0);
        return true;
    }

    return false;
}

Component* PopupComponent::findComponentByIDRecursive (Component* parent, const String& componentID)
{
    if (! parent)
        return nullptr;

    // Check if the current component matches the ID
    if (parent->getComponentID() == componentID)
    {
        return parent;
    }

    // Recursively search in child components
    for (auto* child : parent->getChildren())
    {
        Component* found = findComponentByIDRecursive (child, componentID);
        if (found)
        {
            return found;
        }
    }

    // Not found in this branch of the hierarchy
    return nullptr;
}
