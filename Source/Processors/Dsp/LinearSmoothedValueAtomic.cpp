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

#include "LinearSmoothedValueAtomic.h"


// Explicit instantiations
template class LinearSmoothedValueAtomic<float>;
template class LinearSmoothedValueAtomic<double>;


template <typename FloatType>
LinearSmoothedValueAtomic<FloatType>::LinearSmoothedValueAtomic() noexcept
    : currentValue  (0)
    , target        (0)
    , step          (0)
    , countdown     (0)
    , stepsToTarget (0)
{
}


template <typename FloatType>
LinearSmoothedValueAtomic<FloatType>::LinearSmoothedValueAtomic (FloatType initialValue) noexcept

    : currentValue  (initialValue)
    , target        (initialValue)
    , step          (0)
    , countdown     (0)
    , stepsToTarget (0)
{
}


template<typename FloatType>
void LinearSmoothedValueAtomic<FloatType>::reset (double sampleRate, double rampLengthInSeconds) noexcept
{
    jassert (sampleRate > 0 && rampLengthInSeconds >= 0);
    stepsToTarget = (int) std::floor (rampLengthInSeconds * sampleRate);
    currentValue = target;
    countdown = 0;
}


template<typename FloatType>
void LinearSmoothedValueAtomic<FloatType>::setValue (FloatType newValue) noexcept
{
    target.store (newValue);
}


template<typename FloatType>
void LinearSmoothedValueAtomic<FloatType>::updateTarget() noexcept
{
    FloatType newTarget = target.load();
    if (newTarget != currentTarget)
    {
        currentTarget = newTarget;
        countdown = stepsToTarget;

        if (countdown <= 0)
            currentValue = currentTarget;
        else
            step = (currentTarget - currentValue) / (FloatType) countdown;
    }
}


template<typename FloatType>
FloatType LinearSmoothedValueAtomic<FloatType>::getNextValue() noexcept
{
    if (countdown <= 0)
        return currentTarget;

    --countdown;
    currentValue += step;
    return currentValue;
}
