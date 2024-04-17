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

pointer_sized_int VST2ClientExtensions::handleVstPluginCanDo ([[maybe_unused]] int32 index,
                                                              [[maybe_unused]] pointer_sized_int value,
                                                              [[maybe_unused]] void* ptr,
                                                              [[maybe_unused]] float opt)
{
    return 0;
}

void VST2ClientExtensions::handleVstHostCallbackAvailable ([[maybe_unused]] std::function<VstHostCallbackType>&& callback) {}

} // namespace juce
