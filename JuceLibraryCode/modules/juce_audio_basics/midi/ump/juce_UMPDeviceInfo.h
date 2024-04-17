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
    Holds MIDI device info that may be required by certain UMP messages and
    MIDI-CI messages.

    @tags{Audio}
*/
struct DeviceInfo
{
    std::array<std::byte, 3> manufacturer;  ///< LSB first
    std::array<std::byte, 2> family;        ///< LSB first
    std::array<std::byte, 2> modelNumber;   ///< LSB first
    std::array<std::byte, 4> revision;

private:
    auto tie() const { return std::tie (manufacturer, family, modelNumber, revision); }

public:
    bool operator== (const DeviceInfo& other) const { return tie() == other.tie(); }
    bool operator!= (const DeviceInfo& other) const { return tie() != other.tie(); }

    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename This>
    static auto serialise (Archive& archive, This& t)
    {
        return archive (named ("manufacturer", t.manufacturer),
                        named ("family", t.family),
                        named ("modelNumber", t.modelNumber),
                        named ("revision", t.revision));
    }
};

} // namespace juce::universal_midi_packets
