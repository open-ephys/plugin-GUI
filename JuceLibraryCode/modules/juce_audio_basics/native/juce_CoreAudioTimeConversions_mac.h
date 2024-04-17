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


// This file will be included directly by macOS/iOS-specific .cpps
#pragma once

#if ! DOXYGEN

#include <mach/mach_time.h>

namespace juce
{

struct CoreAudioTimeConversions
{
public:
    CoreAudioTimeConversions()
    {
        mach_timebase_info_data_t info{};
        mach_timebase_info (&info);
        numerator   = info.numer;
        denominator = info.denom;
    }

    uint64_t hostTimeToNanos (uint64_t hostTime) const
    {
        return multiplyByRatio (hostTime, numerator, denominator);
    }

    uint64_t nanosToHostTime (uint64_t nanos) const
    {
        return multiplyByRatio (nanos, denominator, numerator);
    }

private:
    // Adapted from CAHostTimeBase.h in the Core Audio Utility Classes
    static uint64_t multiplyByRatio (uint64_t toMultiply, uint64_t numerator, uint64_t denominator)
    {
       #if defined (__SIZEOF_INT128__)
        unsigned __int128
       #else
        long double
       #endif
            result = toMultiply;

        if (numerator != denominator)
        {
            result *= numerator;
            result /= denominator;
        }

        return (uint64_t) result;
    }

    uint64_t numerator = 0, denominator = 0;
};

} // namespace juce

#endif
