#include "PopoverComponent.h"

#include "../CoreServices.h"

PopoverComponent::PopoverComponent()
{
    setWantsKeyboardFocus(true);
}

PopoverComponent::~PopoverComponent()
{
}

bool PopoverComponent::keyPressed(const KeyPress &key)
{
    if (key == KeyPress('z', ModifierKeys::commandModifier, 0))
    {
        CoreServices::getUndoManager()->undo();
        return true;
    }
    else if (key == KeyPress('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0))
    {
        CoreServices::getUndoManager()->redo();
        return true;
    }
}