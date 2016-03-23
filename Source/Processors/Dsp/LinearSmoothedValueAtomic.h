/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_LINEARSMOOTHEDVALUE_H_INCLUDED
#define JUCE_LINEARSMOOTHEDVALUE_H_INCLUDED

#include "../PluginManager/OpenEphysPlugin.h"
#include <atomic>

//==============================================================================
/**
    Utility class for linearly smoothed values like volume etc. that should
    not change abruptly but as a linear ramp, to avoid audio glitches.

    This class was modified by Open Ephys and just an improvement of LinearSmoothedValue
    class from the JUCE 4.0 library, but with some features made that were suggested
    by Timur Doumler during his JUCE Summit 2015 talk, to make it lock-free and guarantee
    that no race conditions will ever occur using it.

    Usage:
    - set value by calling setValue() method;

    - get value by calling updateTarget() and then getNextValue() method.
      We need to call updateTarget() because we should suppose the value could be changed before
      so we need to update it smoothly and then use it.
*/

//==============================================================================
template<typename FloatType>
class PLUGIN_API  LinearSmoothedValueAtomic
{
public:
    /** Constructor. */
    LinearSmoothedValueAtomic() noexcept;

    /** Constructor. */
    LinearSmoothedValueAtomic (FloatType initialValue) noexcept;

    //==========================================================================
    /** Reset to a new sample rate and ramp length. */
    void reset (double sampleRate, double rampLengthInSeconds) noexcept;

    //==========================================================================
    /** Set a new target value. */
    void setValue (FloatType newValue) noexcept;

    //==========================================================================
    void updateTarget() noexcept;

    //==========================================================================
    /** Compute the next value. */
    FloatType getNextValue() noexcept;


private:
    //==========================================================================
    std::atomic<FloatType> target;

    FloatType currentValue;
    FloatType currentTarget;
    FloatType step;

    int countdown;
    int stepsToTarget;
};


#endif   // JUCE_LINEARSMOOTHEDVALUE_H_INCLUDED
