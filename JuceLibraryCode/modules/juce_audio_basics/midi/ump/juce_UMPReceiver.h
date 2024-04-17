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

/**
    A base class for classes which receive Universal MIDI Packets from an input.

    @tags{Audio}
*/
struct Receiver
{
    virtual ~Receiver() noexcept = default;

    /** This will be called each time a new packet is ready for processing. */
    virtual void packetReceived (const View& packet, double time) = 0;
};

} // namespace juce::universal_midi_packets

#endif
