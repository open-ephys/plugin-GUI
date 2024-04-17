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

struct WindowingHelpers
{
    WindowingHelpers() = delete;

    static Image createIconForFile (const File& file);

    #if JUCE_WINDOWS
     static bool isEmbeddedInForegroundProcess (Component* c);
     static bool isWindowOnCurrentVirtualDesktop (void*);
    #else
     static bool isEmbeddedInForegroundProcess (Component*) { return false; }
     static bool isWindowOnCurrentVirtualDesktop (void*) { return true; }
    #endif

    /*  Returns true if this process is in the foreground, or if the viewComponent
        is embedded into a window owned by the foreground process.
    */
    static bool isForegroundOrEmbeddedProcess (Component* viewComponent)
    {
        return Process::isForegroundProcess() || isEmbeddedInForegroundProcess (viewComponent);
    }

    template <typename Value>
    static BorderSize<int> roundToInt (BorderSize<Value> border)
    {
        return { ::juce::roundToInt (border.getTop()),
                 ::juce::roundToInt (border.getLeft()),
                 ::juce::roundToInt (border.getBottom()),
                 ::juce::roundToInt (border.getRight()) };
    }
};

} // namespace juce::detail
