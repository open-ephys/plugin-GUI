#include "PopoverComponent.h"
#include "UIComponent.h"

#include "../CoreServices.h"

void PopoverManager::showPopover(std::unique_ptr<Component> popoverComponent, Component* anchor)
{

    String componentID = anchor->getComponentID();

    auto &myBox = juce::CallOutBox::launchAsynchronously(std::move(popoverComponent),
                                                            anchor->getScreenBounds(),
                                                            nullptr);

    LOGD("***Adding popover: " + componentID);

    popoverStack.push_back(componentID);

    juce::ModalComponentManager::getInstance()->attachCallback(&myBox, juce::ModalCallbackFunction::create([this](int result) {
        onPopoverDismissed(result);
    }));

}

void PopoverManager::onPopoverDismissed(int result)
{
    if (popoverStack.size() > 0)
    {
        String componentID = popoverStack.back();
        popoverStack.pop_back();
        LOGD("***Closed popover " + componentID);
    }
    else
    {
        LOGD("***No popovers to remove");
    }
}

PopoverComponent::PopoverComponent(Component* parent_) : parent(parent_)
{
    undoManager = CoreServices::getUndoManager();

    setWantsKeyboardFocus(true);
    parent->addComponentListener(this);
}

PopoverComponent::~PopoverComponent()
{
    if (parent != nullptr)
    {
        parent->removeComponentListener(this);
    }
}

bool PopoverComponent::keyPressed(const KeyPress &key)
{
    if (key == KeyPress('z', ModifierKeys::commandModifier, 0))
    {
        LOGD(" [PopupComponent] Undoing action");
        
        if (!undoManager->canUndo()) return false;

        if (Component::getNumCurrentlyModalComponents() > 1)
        {
            findParentComponentOfClass<CallOutBox>()->exitModalState(0);
            return false;
        }
    
        undoManager->undo();
        updatePopup();
        return true;
    }
    else if (key == KeyPress('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0))
    {
        LOGD(" [PopupComponent] Redoing action");

        if (!undoManager->canRedo()) return false;

        undoManager->redo();
        updatePopup();
        return true;
    }
    else if (key == KeyPress(KeyPress::escapeKey, 0, 0))
    {
        findParentComponentOfClass<CallOutBox>()->exitModalState(0);
        return true;
    }

    return false;
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
