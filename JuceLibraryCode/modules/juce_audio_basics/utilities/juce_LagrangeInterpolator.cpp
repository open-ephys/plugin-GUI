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

template <int k>
struct LagrangeResampleHelper
{
    static forcedinline void calc (float& a, float b) noexcept   { a *= b * (1.0f / k); }
};

template <>
struct LagrangeResampleHelper<0>
{
    static forcedinline void calc (float&, float) noexcept {}
};

template <int k>
static float calcCoefficient (float input, float offset) noexcept
{
    LagrangeResampleHelper<0 - k>::calc (input, -2.0f - offset);
    LagrangeResampleHelper<1 - k>::calc (input, -1.0f - offset);
    LagrangeResampleHelper<2 - k>::calc (input,  0.0f - offset);
    LagrangeResampleHelper<3 - k>::calc (input,  1.0f - offset);
    LagrangeResampleHelper<4 - k>::calc (input,  2.0f - offset);
    return input;
}

float Interpolators::LagrangeTraits::valueAtOffset (const float* inputs, float offset, int index) noexcept
{
    float result = 0.0f;

    result += calcCoefficient<0> (inputs[index], offset); if (++index == 5) index = 0;
    result += calcCoefficient<1> (inputs[index], offset); if (++index == 5) index = 0;
    result += calcCoefficient<2> (inputs[index], offset); if (++index == 5) index = 0;
    result += calcCoefficient<3> (inputs[index], offset); if (++index == 5) index = 0;
    result += calcCoefficient<4> (inputs[index], offset);

    return result;
}

} // namespace juce
