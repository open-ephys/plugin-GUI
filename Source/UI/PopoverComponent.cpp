#include "PopoverComponent.h"
#include "UIComponent.h"

#include "../CoreServices.h"

PopoverComponent::PopoverComponent(Component* parent_) : parent(parent_)
{
    undoManager = CoreServices::getUndoManager();

    setWantsKeyboardFocus(true);
}

PopoverComponent::~PopoverComponent()
{
}

bool PopoverComponent::keyPressed(const KeyPress &key)
{
    if (key == KeyPress('z', ModifierKeys::commandModifier, 0))
    {
        if (!undoManager->canUndo()) return false;

        String desc = undoManager->getUndoDescription();
        if (desc == "")
        {
            //findParentComponentOfClass<CallOutBox>()->exitModalState(0);
            juce::ModalComponentManager::getInstance()->cancelAllModalComponents();
            undoManager->undo();
            return false;
        }
        else if (desc != parent->getComponentID())
        {
            String parentID = parent->getComponentID();
            juce::ModalComponentManager::getInstance()->cancelAllModalComponents();
            Component* foundComponent = AccessClass::getUIComponent()->findComponentByIDRecursive(AccessClass::getUIComponent(), desc);
            ((Button*)foundComponent)->triggerClick();
            undoManager->undo();
            return false;
        } 
        else
        {
            undoManager->undo();
            return true;
        }
    }
    else if (key == KeyPress('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0))
    {
        //Get descripion of next undo action, if it's not from this popover, close the popover
        if (!undoManager->canRedo()) return false;

        String desc = undoManager->getRedoDescription();
        if (desc == "")
        {
            findParentComponentOfClass<CallOutBox>()->exitModalState(0);
            undoManager->redo();
            return false;
        }
        else if (desc != parent->getComponentID())
        {
            findParentComponentOfClass<CallOutBox>()->exitModalState(0);
            Component* foundComponent = AccessClass::getUIComponent()->findComponentByIDRecursive(AccessClass::getUIComponent(), desc);
            ((Button*)foundComponent)->triggerClick();
            undoManager->redo();
            return false;
        } 
        else
        {
            undoManager->redo();
            return true;
        }
    }
    else if (key == KeyPress(KeyPress::escapeKey, 0, 0))
    {
        findParentComponentOfClass<CallOutBox>()->exitModalState(0);
        return true;
    }
}

Component* PopoverComponent::findComponentByIDRecursive(Component* parent, const String& componentID) {
    if (!parent) return nullptr;

    // Check if the current component matches the ID
    if (parent->getComponentID() == componentID) {
        return parent;
    }

    // Recursively search in child components
    for (auto* child : parent->getChildren()) {
        Component* found = findComponentByIDRecursive(child, componentID);
        if (found) {
            return found;
        }
    }

    // Not found in this branch of the hierarchy
    return nullptr;
}
