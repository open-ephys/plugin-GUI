#include "PopoverComponent.h"

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

        //Get descripion of next undo action, if it's not from this popover, close the popover
        String desc = undoManager->getUndoDescription();
        if (desc == "")
        {
            findParentComponentOfClass<CallOutBox>()->exitModalState(0);
            undoManager->undo();
            return false;
        }
        else //TODO: make sure the right popup is showing for the next action
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
        else //TODO: make sure the right popup is showing for the next action
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