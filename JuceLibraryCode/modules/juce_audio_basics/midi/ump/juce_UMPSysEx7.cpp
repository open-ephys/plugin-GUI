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

uint32_t SysEx7::getNumPacketsRequiredForDataSize (uint32_t size)
{
    constexpr auto denom = 6;
    return (size / denom) + ((size % denom) != 0);
}

SysEx7::PacketBytes SysEx7::getDataBytes (const PacketX2& packet)
{
    const auto numBytes = Utils::getChannel (packet[0]);
    constexpr uint8_t maxBytes = 6;
    jassert (numBytes <= maxBytes);

    return
    {
        { { std::byte { packet.getU8<2>() },
            std::byte { packet.getU8<3>() },
            std::byte { packet.getU8<4>() },
            std::byte { packet.getU8<5>() },
            std::byte { packet.getU8<6>() },
            std::byte { packet.getU8<7>() } } },
        jmin (numBytes, maxBytes)
    };
}

} // namespace juce::universal_midi_packets
