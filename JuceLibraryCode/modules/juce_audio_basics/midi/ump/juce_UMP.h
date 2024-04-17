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

#include "juce_UMPProtocols.h"
#include "juce_UMPUtils.h"
#include "juce_UMPacket.h"
#include "juce_UMPSysEx7.h"
#include "juce_UMPView.h"
#include "juce_UMPIterator.h"
#include "juce_UMPackets.h"
#include "juce_UMPFactory.h"
#include "juce_UMPConversion.h"
#include "juce_UMPMidi1ToBytestreamTranslator.h"
#include "juce_UMPMidi1ToMidi2DefaultTranslator.h"
#include "juce_UMPConverters.h"
#include "juce_UMPDispatcher.h"
#include "juce_UMPReceiver.h"

#ifndef DOXYGEN

namespace juce
{
namespace ump = universal_midi_packets;
}

#endif
