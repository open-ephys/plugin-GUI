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

namespace juce::universal_midi_packets
{

/**
    Holds a UMP group, and a span of bytes that were received or are to be
    sent on that group. Helpful when working with sysex messages.

    @tags{Audio}
*/
struct BytesOnGroup
{
    uint8_t group{};
    Span<const std::byte> bytes;
};

} // namespace juce::universal_midi_packets
