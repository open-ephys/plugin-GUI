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

#if JUCE_UNIT_TESTS

static CommonSmoothedValueTests <SmoothedValue<float, ValueSmoothingTypes::Linear>> commonLinearSmoothedValueTests;
static CommonSmoothedValueTests <SmoothedValue<float, ValueSmoothingTypes::Multiplicative>> commonMultiplicativeSmoothedValueTests;

class SmoothedValueTests final : public UnitTest
{
public:
    SmoothedValueTests()
        : UnitTest ("SmoothedValueTests", UnitTestCategories::smoothedValues)
    {}

    void runTest() override
    {
        beginTest ("Linear moving target");
        {
            SmoothedValue<float, ValueSmoothingTypes::Linear> sv;

            sv.reset (12);
            float initialValue = 0.0f;
            sv.setCurrentAndTargetValue (initialValue);
            sv.setTargetValue (1.0f);

            auto delta = sv.getNextValue() - initialValue;

            sv.skip (6);

            auto newInitialValue = sv.getCurrentValue();
            sv.setTargetValue (newInitialValue + 2.0f);
            auto doubleDelta = sv.getNextValue() - newInitialValue;

            expectWithinAbsoluteError (doubleDelta, delta * 2.0f, 1.0e-7f);
        }

        beginTest ("Multiplicative curve");
        {
            SmoothedValue<double, ValueSmoothingTypes::Multiplicative> sv;

            auto numSamples = 12;
            AudioBuffer<double> values (2, numSamples + 1);

            sv.reset (numSamples);
            sv.setCurrentAndTargetValue (1.0);
            sv.setTargetValue (2.0f);

            values.setSample (0, 0, sv.getCurrentValue());

            for (int i = 1; i < values.getNumSamples(); ++i)
                values.setSample (0, i, sv.getNextValue());

            sv.setTargetValue (1.0f);
            values.setSample (1, values.getNumSamples() - 1, sv.getCurrentValue());

            for (int i = values.getNumSamples() - 2; i >= 0 ; --i)
                values.setSample (1, i, sv.getNextValue());

            for (int i = 0; i < values.getNumSamples(); ++i)
                expectWithinAbsoluteError (values.getSample (0, i), values.getSample (1, i), 1.0e-9);
        }
    }
};

static SmoothedValueTests smoothedValueTests;

#endif

} // namespace juce
