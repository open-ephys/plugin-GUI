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
/**
    Disables low-power-mode for the duration of an instance's lifetime.

    Currently this is only implemented on macOS, where it will disable the
    "App Nap" power-saving feature.

    @tags{Events}
*/
class ScopedLowPowerModeDisabler
{
public:
    ScopedLowPowerModeDisabler();
    ~ScopedLowPowerModeDisabler();

private:
    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE (ScopedLowPowerModeDisabler)
    JUCE_DECLARE_NON_MOVEABLE (ScopedLowPowerModeDisabler)
};

} // namespace juce
