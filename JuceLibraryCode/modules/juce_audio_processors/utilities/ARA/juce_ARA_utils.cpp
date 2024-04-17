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

#if (JucePlugin_Enable_ARA || (JUCE_PLUGINHOST_ARA && (JUCE_PLUGINHOST_VST3 || JUCE_PLUGINHOST_AU))) && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX)
namespace juce
{
 #if ARA_ENABLE_INTERNAL_ASSERTS
JUCE_API void JUCE_CALLTYPE handleARAAssertion (const char* file, const int line, const char* diagnosis) noexcept
{
  #if (JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS)
    DBG (diagnosis);
  #endif

    logAssertion (file, line);

  #if (JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS)
    if (juce_isRunningUnderDebugger())
        JUCE_BREAK_IN_DEBUGGER;
    JUCE_ANALYZER_NORETURN
  #endif
}
 #endif
}
#endif

#if JucePlugin_Enable_ARA
#include "juce_ARADocumentControllerCommon.cpp"
#include "juce_ARADocumentController.cpp"
#include "juce_ARAModelObjects.cpp"
#include "juce_ARAPlugInInstanceRoles.cpp"
#include "juce_AudioProcessor_ARAExtensions.cpp"

ARA_SETUP_DEBUG_MESSAGE_PREFIX (JucePlugin_Name);
#endif
