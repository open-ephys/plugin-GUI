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

struct ViewportHelpers
{
    ViewportHelpers() = delete;

    static bool wouldScrollOnEvent (const Viewport* vp, const MouseInputSource& src)
    {
        if (vp != nullptr)
        {
            switch (vp->getScrollOnDragMode())
            {
                case Viewport::ScrollOnDragMode::all:           return true;
                case Viewport::ScrollOnDragMode::nonHover:      return ! src.canHover();
                case Viewport::ScrollOnDragMode::never:         return false;
            }
        }

        return false;
    }
};

} // namespace juce::detail
