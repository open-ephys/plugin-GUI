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

#ifndef DOXYGEN

namespace juce::universal_midi_packets
{

/** The kinds of MIDI protocol that can be formatted into Universal MIDI Packets. */
enum class PacketProtocol
{
    MIDI_1_0,
    MIDI_2_0,
};

/** All kinds of MIDI protocol understood by JUCE. */
enum class MidiProtocol
{
    bytestream,
    UMP_MIDI_1_0,
    UMP_MIDI_2_0,
};

} // namespace juce::universal_midi_packets

#endif
