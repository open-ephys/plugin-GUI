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

namespace juce
{

//==============================================================================
/**
    A class that measures various statistics about a series of floating point
    values that it is given.

    @tags{Core}
*/
template <typename FloatType>
class StatisticsAccumulator
{
public:
    //==============================================================================
    /** Constructs a new StatisticsAccumulator. */
    StatisticsAccumulator() = default;

    //==============================================================================
    /** Add a new value to the accumulator.
        This will update all running statistics accordingly.
    */
    void addValue (FloatType v) noexcept
    {
        jassert (juce_isfinite (v));

        sum += v;
        sumSquares += v * v;
        ++count;

        if (v > maximum) maximum = v;
        if (v < minimum) minimum = v;
    }

    /** Reset the accumulator.
        This will reset all currently saved statistcs.
    */
    void reset() noexcept               { *this = StatisticsAccumulator<FloatType>(); }

    //==============================================================================
    /** Returns the average (arithmetic mean) of all previously added values.
        If no values have been added yet, this will return zero.
    */
    FloatType getAverage() const noexcept
    {
        return count > 0 ? sum / (FloatType) count
                         : FloatType();
    }

    /** Returns the variance of all previously added values.
        If no values have been added yet, this will return zero.
    */
    FloatType getVariance() const noexcept
    {
        return count > 0 ? (sumSquares - sum * sum / (FloatType) count) / (FloatType) count
                         : FloatType();
    }

    /** Returns the standard deviation of all previously added values.
        If no values have been added yet, this will return zero.
    */
    FloatType getStandardDeviation() const noexcept
    {
        return std::sqrt (getVariance());
    }

    /** Returns the smallest of all previously added values.
        If no values have been added yet, this will return positive infinity.
    */
    FloatType getMinValue() const noexcept
    {
        return minimum;
    }

    /** Returns the largest of all previously added values.
        If no values have been added yet, this will return negative infinity.
    */
    FloatType getMaxValue() const noexcept
    {
        return maximum;
    }

    /** Returns how many values have been added to this accumulator. */
    size_t getCount() const noexcept
    {
        return count;
    }

private:
    //==============================================================================
    struct KahanSum
    {
        KahanSum() = default;
        operator FloatType() const noexcept             { return sum; }

        void JUCE_NO_ASSOCIATIVE_MATH_OPTIMISATIONS operator+= (FloatType value) noexcept
        {
            FloatType correctedValue = value - error;
            FloatType newSum = sum + correctedValue;
            error = (newSum - sum) - correctedValue;
            sum = newSum;
        }

        FloatType sum{}, error{};
    };

    //==============================================================================
    size_t count { 0 };
    KahanSum sum, sumSquares;
    FloatType minimum {  std::numeric_limits<FloatType>::infinity() },
              maximum { -std::numeric_limits<FloatType>::infinity() };
};

} // namespace juce
