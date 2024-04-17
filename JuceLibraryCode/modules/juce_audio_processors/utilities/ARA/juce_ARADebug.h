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

#pragma once

#include <juce_core/system/juce_PlatformDefs.h>

#ifndef JUCE_API
 #define JUCE_API
#endif

#if (JucePlugin_Enable_ARA || (JUCE_PLUGINHOST_ARA && (JUCE_PLUGINHOST_VST3 || JUCE_PLUGINHOST_AU))) && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX)

namespace juce
{

//==============================================================================
 #if (JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS) || JUCE_LOG_ASSERTIONS
  #define ARA_ENABLE_INTERNAL_ASSERTS 1
 #else
  #define ARA_ENABLE_INTERNAL_ASSERTS 0
 #endif // (JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS) || JUCE_LOG_ASSERTIONS

//==============================================================================
 #if ARA_ENABLE_INTERNAL_ASSERTS

JUCE_API void JUCE_CALLTYPE handleARAAssertion (const char* file, const int line, const char* diagnosis) noexcept;

  #if !defined(ARA_HANDLE_ASSERT)
   #define ARA_HANDLE_ASSERT(file, line, diagnosis)    juce::handleARAAssertion (file, line, diagnosis)
  #endif

  #if JUCE_LOG_ASSERTIONS
   #define ARA_ENABLE_DEBUG_OUTPUT 1
  #endif

 #endif

} // namespace juce

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wgnu-zero-variadic-macro-arguments", "-Wmissing-prototypes")
 #include <ARA_Library/Debug/ARADebug.h>
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#endif
