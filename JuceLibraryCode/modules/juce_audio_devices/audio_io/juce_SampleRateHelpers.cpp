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

namespace juce::SampleRateHelpers
{

static inline const std::vector<double>& getAllSampleRates()
{
    static auto sampleRates = []
    {
        std::vector<double> result;
        constexpr double baseRates[] = { 8000.0, 11025.0, 12000.0 };
        constexpr double maxRate = 768000.0;

        for (auto rate : baseRates)
            for (; rate <= maxRate; rate *= 2)
                result.insert (std::upper_bound (result.begin(), result.end(), rate),
                               rate);

        return result;
    }();

    return sampleRates;
}

} // namespace juce::SampleRateHelpers
