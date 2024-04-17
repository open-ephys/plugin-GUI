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

namespace juce::detail
{

struct AccessibilityHelpers
{
    AccessibilityHelpers() = delete;

    enum class Event
    {
        elementCreated,
        elementDestroyed,
        elementMovedOrResized,
        focusChanged,
        windowOpened,
        windowClosed
    };

    static void notifyAccessibilityEvent (const AccessibilityHandler&, Event);

    static String getApplicationOrPluginName()
    {
       #if defined (JucePlugin_Name)
        return JucePlugin_Name;
       #else
        if (auto* app = JUCEApplicationBase::getInstance())
            return app->getApplicationName();

        return "JUCE Application";
       #endif
    }

    template <typename MemberFn>
    static const AccessibilityHandler* getEnclosingHandlerWithInterface (const AccessibilityHandler* handler, MemberFn fn)
    {
        if (handler == nullptr)
            return nullptr;

        if ((handler->*fn)() != nullptr)
            return handler;

        return getEnclosingHandlerWithInterface (handler->getParent(), fn);
    }
};

} // namespace juce::detail
