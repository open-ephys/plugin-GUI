/*
  ==============================================================================

   This file is part of the JUCE 8 technical preview.
   Copyright (c) Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
class UIAExpandCollapseProvider  : public UIAProviderBase,
                                   public ComBaseClassHelper<ComTypes::IExpandCollapseProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    JUCE_COMRESULT Expand() override
    {
        return invokeShowMenu();
    }

    JUCE_COMRESULT Collapse() override
    {
        return invokeShowMenu();
    }

    JUCE_COMRESULT get_ExpandCollapseState (ComTypes::ExpandCollapseState* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = getHandler().getCurrentState().isExpanded()
                           ? ComTypes::ExpandCollapseState_Expanded
                           : ComTypes::ExpandCollapseState_Collapsed;

            return S_OK;
        });
    }

private:
    JUCE_COMRESULT invokeShowMenu()
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        const auto& handler = getHandler();

        if (handler.getActions().invoke (AccessibilityActionType::showMenu))
        {
            using namespace ComTypes::Constants;

            sendAccessibilityAutomationEvent (handler, handler.getCurrentState().isExpanded()
                                                           ? UIA_MenuOpenedEventId
                                                           : UIA_MenuClosedEventId);

            return S_OK;
        }

        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAExpandCollapseProvider)
};

} // namespace juce
