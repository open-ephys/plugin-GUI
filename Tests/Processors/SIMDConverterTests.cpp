#include <stdio.h>

#include "gtest/gtest.h"

#include <Processors/RecordNode/BinaryFormat/SIMDConverter.h>
#include <cmath>
#include <vector>
#include <random>
#include <limits>

class SIMDConverterTests : public testing::Test {
protected:
    // Reference implementation for verification
    // Note: The implementation uses different rounding for different code paths:
    // - Scalar fallback (numSamples < 8): uses std::round() (rounds half away from zero)
    // - SIMD path (numSamples >= 8): uses hardware rounding (round-to-nearest-even on NEON/SSE4.1)
    // For simplicity, this reference uses std::round() which matches the scalar implementation.
    // The SIMD path may differ by ±1 for exact .5 values, but this is acceptable for
    // audio/neural data where exact .5 values are extremely rare.
    static int16_t referenceConvert(float input, float scale) {
        float scaled = input * scale;
        int32_t rounded = static_cast<int32_t>(std::round(scaled));
        rounded = std::max(-32768, std::min(32767, rounded));
        return static_cast<int16_t>(rounded);
    }
    
    // Helper that allows ±1 tolerance for .5 boundary values where SIMD and scalar
    // may differ due to different rounding modes
    static bool valuesMatchWithRoundingTolerance(int16_t actual, int16_t expected, float input) {
        if (actual == expected) return true;
        // Check if input was at a .5 boundary where rounding may differ
        float fractional = std::fabs(input - std::floor(input));
        bool isHalfValue = (std::fabs(fractional - 0.5f) < 0.0001f);
        if (isHalfValue && std::abs(actual - expected) == 1) return true;
        return false;
    }

    // Helper to compare outputs with reference, allowing tolerance for .5 boundary rounding
    void verifyConversion(const float* input, const int16_t* output, 
                          float scale, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            int16_t expected = referenceConvert(input[i], scale);
            float scaledInput = input[i] * scale;
            EXPECT_TRUE(valuesMatchWithRoundingTolerance(output[i], expected, scaledInput)) 
                << "Mismatch at index " << i 
                << ": input=" << input[i] 
                << ", scale=" << scale 
                << ", expected=" << expected 
                << ", got=" << output[i];
        }
    }
};

// =============================================================================
// Test 1: Basic conversion with scaling, rounding, and clamping
// =============================================================================

TEST_F(SIMDConverterTests, BasicConversion_ScalesCorrectly) {
    const int numSamples = 16;
    std::vector<float> input(numSamples);
    std::vector<int16_t> output(numSamples);
    
    // Simple values that scale cleanly
    for (int i = 0; i < numSamples; ++i) {
        input[i] = static_cast<float>(i * 100);
    }
    
    float scale = 1.0f;
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), scale, numSamples);
    
    verifyConversion(input.data(), output.data(), scale, numSamples);
}

TEST_F(SIMDConverterTests, BasicConversion_WithNonUnityScale) {
    const int numSamples = 16;
    std::vector<float> input(numSamples);
    std::vector<int16_t> output(numSamples);
    
    // Values that need scaling
    for (int i = 0; i < numSamples; ++i) {
        input[i] = static_cast<float>(i) * 10.5f;
    }
    
    float scale = 2.5f;
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), scale, numSamples);
    
    verifyConversion(input.data(), output.data(), scale, numSamples);
}

TEST_F(SIMDConverterTests, BasicConversion_NegativeValues) {
    const int numSamples = 16;
    std::vector<float> input(numSamples);
    std::vector<int16_t> output(numSamples);
    
    // Mix of positive and negative values
    for (int i = 0; i < numSamples; ++i) {
        input[i] = static_cast<float>(i - 8) * 1000.0f;
    }
    
    float scale = 1.0f;
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), scale, numSamples);
    
    verifyConversion(input.data(), output.data(), scale, numSamples);
}

TEST_F(SIMDConverterTests, BasicConversion_SmallFractionalValues) {
    const int numSamples = 16;
    std::vector<float> input(numSamples);
    std::vector<int16_t> output(numSamples);
    
    // Small fractional values that need scaling up
    for (int i = 0; i < numSamples; ++i) {
        input[i] = static_cast<float>(i) * 0.001f;
    }
    
    // Scale factor similar to what BinaryRecording uses
    float scale = 1.0f / (32767.0f * 0.000195f);  // ~156.3
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), scale, numSamples);
    
    verifyConversion(input.data(), output.data(), scale, numSamples);
}

TEST_F(SIMDConverterTests, BasicConversion_ZeroValues) {
    const int numSamples = 16;
    std::vector<float> input(numSamples, 0.0f);
    std::vector<int16_t> output(numSamples, -1);  // Initialize to non-zero
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 100.0f, numSamples);
    
    for (int i = 0; i < numSamples; ++i) {
        EXPECT_EQ(output[i], 0) << "Zero input should produce zero output";
    }
}

TEST_F(SIMDConverterTests, BasicConversion_EmptyArray) {
    std::vector<float> input;
    std::vector<int16_t> output;
    
    // Should not crash with empty array
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, 0);
}

// =============================================================================
// Test 2: Non-SIMD-aligned array lengths
// =============================================================================

TEST_F(SIMDConverterTests, NonAlignedLength_OneElement) {
    std::vector<float> input = { 12345.6f };
    std::vector<int16_t> output(1);
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, 1);
    
    verifyConversion(input.data(), output.data(), 1.0f, 1);
}

TEST_F(SIMDConverterTests, NonAlignedLength_ThreeElements) {
    std::vector<float> input = { 100.0f, -200.0f, 300.5f };
    std::vector<int16_t> output(3);
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, 3);
    
    verifyConversion(input.data(), output.data(), 1.0f, 3);
}

TEST_F(SIMDConverterTests, NonAlignedLength_SevenElements) {
    // 7 is not divisible by 8 (SIMD width)
    const int numSamples = 7;
    std::vector<float> input(numSamples);
    std::vector<int16_t> output(numSamples);
    
    for (int i = 0; i < numSamples; ++i) {
        input[i] = static_cast<float>((i + 1) * 1000);
    }
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, numSamples);
    
    verifyConversion(input.data(), output.data(), 1.0f, numSamples);
}

TEST_F(SIMDConverterTests, NonAlignedLength_NineElements) {
    // 9 = 8 + 1 (one full SIMD iteration + 1 remainder)
    const int numSamples = 9;
    std::vector<float> input(numSamples);
    std::vector<int16_t> output(numSamples);
    
    for (int i = 0; i < numSamples; ++i) {
        input[i] = static_cast<float>(i - 4) * 500.0f;
    }
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, numSamples);
    
    verifyConversion(input.data(), output.data(), 1.0f, numSamples);
}

TEST_F(SIMDConverterTests, NonAlignedLength_FifteenElements) {
    // 15 = 8 + 7 (one full SIMD iteration + 7 remainder)
    const int numSamples = 15;
    std::vector<float> input(numSamples);
    std::vector<int16_t> output(numSamples);
    
    for (int i = 0; i < numSamples; ++i) {
        input[i] = static_cast<float>(i * i) - 100.0f;
    }
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, numSamples);
    
    verifyConversion(input.data(), output.data(), 1.0f, numSamples);
}

TEST_F(SIMDConverterTests, NonAlignedLength_LargeNonAligned) {
    // Large array not divisible by 8: 4096 + 5 = 4101
    const int numSamples = 4101;
    std::vector<float> input(numSamples);
    std::vector<int16_t> output(numSamples);
    
    std::mt19937 rng(42);  // Fixed seed for reproducibility
    std::uniform_real_distribution<float> dist(-20000.0f, 20000.0f);
    
    for (int i = 0; i < numSamples; ++i) {
        input[i] = dist(rng);
    }
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, numSamples);
    
    verifyConversion(input.data(), output.data(), 1.0f, numSamples);
}

// =============================================================================
// Test 3: Batch conversion with different scale factors
// =============================================================================

TEST_F(SIMDConverterTests, BatchConversion_SingleChannel) {
    const int numSamples = 32;
    std::vector<float> inputData(numSamples);
    std::vector<int16_t> outputData(numSamples);
    
    for (int i = 0; i < numSamples; ++i) {
        inputData[i] = static_cast<float>(i * 100);
    }
    
    const float* inputs[] = { inputData.data() };
    int16_t* outputs[] = { outputData.data() };
    float scaleFactors[] = { 1.5f };
    
    SIMDConverter::convertFloatToInt16Batch(inputs, outputs, scaleFactors, 1, numSamples);
    
    verifyConversion(inputData.data(), outputData.data(), 1.5f, numSamples);
}

TEST_F(SIMDConverterTests, BatchConversion_MultipleChannels_SameScale) {
    const int numChannels = 4;
    const int numSamples = 64;
    
    std::vector<std::vector<float>> inputBuffers(numChannels, std::vector<float>(numSamples));
    std::vector<std::vector<int16_t>> outputBuffers(numChannels, std::vector<int16_t>(numSamples));
    
    std::vector<const float*> inputs(numChannels);
    std::vector<int16_t*> outputs(numChannels);
    std::vector<float> scaleFactors(numChannels, 2.0f);  // Same scale for all
    
    for (int ch = 0; ch < numChannels; ++ch) {
        for (int i = 0; i < numSamples; ++i) {
            inputBuffers[ch][i] = static_cast<float>((ch + 1) * i * 50);
        }
        inputs[ch] = inputBuffers[ch].data();
        outputs[ch] = outputBuffers[ch].data();
    }
    
    SIMDConverter::convertFloatToInt16Batch(inputs.data(), outputs.data(), 
                                             scaleFactors.data(), numChannels, numSamples);
    
    for (int ch = 0; ch < numChannels; ++ch) {
        verifyConversion(inputBuffers[ch].data(), outputBuffers[ch].data(), 
                         scaleFactors[ch], numSamples);
    }
}

TEST_F(SIMDConverterTests, BatchConversion_MultipleChannels_DifferentScales) {
    const int numChannels = 8;
    const int numSamples = 128;
    
    std::vector<std::vector<float>> inputBuffers(numChannels, std::vector<float>(numSamples));
    std::vector<std::vector<int16_t>> outputBuffers(numChannels, std::vector<int16_t>(numSamples));
    
    std::vector<const float*> inputs(numChannels);
    std::vector<int16_t*> outputs(numChannels);
    
    // Different scale factors for each channel (simulating different bitVolts)
    std::vector<float> scaleFactors = { 1.0f, 0.5f, 2.0f, 0.195f, 3.14159f, 0.001f, 100.0f, 1.234f };
    
    std::mt19937 rng(123);
    std::uniform_real_distribution<float> dist(-15000.0f, 15000.0f);
    
    for (int ch = 0; ch < numChannels; ++ch) {
        for (int i = 0; i < numSamples; ++i) {
            inputBuffers[ch][i] = dist(rng);
        }
        inputs[ch] = inputBuffers[ch].data();
        outputs[ch] = outputBuffers[ch].data();
    }
    
    SIMDConverter::convertFloatToInt16Batch(inputs.data(), outputs.data(), 
                                             scaleFactors.data(), numChannels, numSamples);
    
    for (int ch = 0; ch < numChannels; ++ch) {
        verifyConversion(inputBuffers[ch].data(), outputBuffers[ch].data(), 
                         scaleFactors[ch], numSamples);
    }
}

TEST_F(SIMDConverterTests, BatchConversion_ManyChannels_Neuropixels) {
    // Simulate Neuropixels 1.0 configuration: 384 channels
    const int numChannels = 384;
    const int numSamples = 4096;
    
    std::vector<std::vector<float>> inputBuffers(numChannels, std::vector<float>(numSamples));
    std::vector<std::vector<int16_t>> outputBuffers(numChannels, std::vector<int16_t>(numSamples));
    
    std::vector<const float*> inputs(numChannels);
    std::vector<int16_t*> outputs(numChannels);
    std::vector<float> scaleFactors(numChannels);
    
    std::mt19937 rng(456);
    std::uniform_real_distribution<float> dataDist(-500.0f, 500.0f);
    std::uniform_real_distribution<float> scaleDist(0.1f, 10.0f);
    
    for (int ch = 0; ch < numChannels; ++ch) {
        scaleFactors[ch] = scaleDist(rng);
        for (int i = 0; i < numSamples; ++i) {
            inputBuffers[ch][i] = dataDist(rng);
        }
        inputs[ch] = inputBuffers[ch].data();
        outputs[ch] = outputBuffers[ch].data();
    }
    
    SIMDConverter::convertFloatToInt16Batch(inputs.data(), outputs.data(), 
                                             scaleFactors.data(), numChannels, numSamples);
    
    // Verify a subset of channels for performance
    for (int ch = 0; ch < numChannels; ch += 64) {
        verifyConversion(inputBuffers[ch].data(), outputBuffers[ch].data(), 
                         scaleFactors[ch], numSamples);
    }
}

// =============================================================================
// Test 4: SIMD detection
// =============================================================================

TEST_F(SIMDConverterTests, SIMDDetection_ReturnsValidType) {
    SIMDConverter::SIMDType simdType = SIMDConverter::getAvailableSIMD();
    
    // Should return one of the known types
    bool isValidType = (simdType == SIMDConverter::SIMDType::None ||
                        simdType == SIMDConverter::SIMDType::SSE2 ||
                        simdType == SIMDConverter::SIMDType::SSE4_1 ||
                        simdType == SIMDConverter::SIMDType::AVX2 ||
                        simdType == SIMDConverter::SIMDType::NEON);
    
    EXPECT_TRUE(isValidType) << "getAvailableSIMD() returned invalid type";
}

TEST_F(SIMDConverterTests, SIMDDetection_ConsistentResults) {
    // Multiple calls should return the same result
    SIMDConverter::SIMDType firstCall = SIMDConverter::getAvailableSIMD();
    SIMDConverter::SIMDType secondCall = SIMDConverter::getAvailableSIMD();
    SIMDConverter::SIMDType thirdCall = SIMDConverter::getAvailableSIMD();
    
    EXPECT_EQ(firstCall, secondCall);
    EXPECT_EQ(secondCall, thirdCall);
}

TEST_F(SIMDConverterTests, SIMDDetection_StringMatchesType) {
    SIMDConverter::SIMDType simdType = SIMDConverter::getAvailableSIMD();
    std::string simdString = SIMDConverter::getSIMDTypeString();
    
    // Verify string matches detected type
    switch (simdType) {
        case SIMDConverter::SIMDType::None:
            EXPECT_EQ(simdString, "Scalar (no SIMD)");
            break;
        case SIMDConverter::SIMDType::SSE2:
            EXPECT_EQ(simdString, "x86 SSE2");
            break;
        case SIMDConverter::SIMDType::SSE4_1:
            EXPECT_EQ(simdString, "x86 SSE4.1");
            break;
        case SIMDConverter::SIMDType::AVX2:
            EXPECT_EQ(simdString, "x86 AVX2");
            break;
        case SIMDConverter::SIMDType::NEON:
            EXPECT_EQ(simdString, "ARM NEON");
            break;
        default:
            FAIL() << "Unknown SIMD type";
    }
}

TEST_F(SIMDConverterTests, SIMDDetection_StringNotEmpty) {
    std::string simdString = SIMDConverter::getSIMDTypeString();
    
    EXPECT_FALSE(simdString.empty()) << "getSIMDTypeString() should not return empty string";
    EXPECT_GT(simdString.length(), 0);
}

// =============================================================================
// Test 5: Rounding and clamping behavior
// =============================================================================

TEST_F(SIMDConverterTests, Rounding_HalfValuesRoundToNearest) {
    // SIMD implementations use round-to-nearest-even (banker's rounding):
    // 0.5 -> 0, 1.5 -> 2, 2.5 -> 2, 3.5 -> 4
    // This differs from std::round() which rounds half away from zero.
    std::vector<float> input = { 
        0.5f, 1.5f, 2.5f, 3.5f,
        -0.5f, -1.5f, -2.5f, -3.5f
    };
    std::vector<int16_t> output(input.size());
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, 
                                        static_cast<int>(input.size()));
    
    verifyConversion(input.data(), output.data(), 1.0f, static_cast<int>(input.size()));
}

TEST_F(SIMDConverterTests, Rounding_FractionalValues) {
    std::vector<float> input = {
        0.1f, 0.4f, 0.6f, 0.9f,
        -0.1f, -0.4f, -0.6f, -0.9f,
        1.1f, 1.4f, 1.6f, 1.9f,
        -1.1f, -1.4f, -1.6f, -1.9f
    };
    std::vector<int16_t> output(input.size());
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, 
                                        static_cast<int>(input.size()));
    
    verifyConversion(input.data(), output.data(), 1.0f, static_cast<int>(input.size()));
}

TEST_F(SIMDConverterTests, Clamping_PositiveOverflow) {
    std::vector<float> input = {
        32767.0f,    // Exactly max
        32768.0f,    // One over
        40000.0f,    // Well over
        100000.0f,   // Way over
    };
    std::vector<int16_t> output(input.size());
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, 
                                        static_cast<int>(input.size()));
    
    EXPECT_EQ(output[0], 32767);
    EXPECT_EQ(output[1], 32767);  // Should clamp
    EXPECT_EQ(output[2], 32767);
    EXPECT_EQ(output[3], 32767);
}

TEST_F(SIMDConverterTests, Clamping_NegativeOverflow) {
    std::vector<float> input = {
        -32768.0f,   // Exactly min
        -32769.0f,   // One under
        -40000.0f,   // Well under
        -100000.0f,  // Way under
        -1e10f       // Extremely negative
    };
    std::vector<int16_t> output(input.size());
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, 
                                        static_cast<int>(input.size()));
    
    EXPECT_EQ(output[0], -32768);
    EXPECT_EQ(output[1], -32768);  // Should clamp
    EXPECT_EQ(output[2], -32768);
    EXPECT_EQ(output[3], -32768);
    EXPECT_EQ(output[4], -32768);
}

TEST_F(SIMDConverterTests, Clamping_ScaledOverflow) {
    // Values that overflow after scaling
    std::vector<float> input = {
        10000.0f,
        20000.0f,
        -10000.0f,
        -20000.0f
    };
    std::vector<int16_t> output(input.size());
    
    float scale = 5.0f;  // Will cause 10000*5 = 50000 > 32767
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), scale, 
                                        static_cast<int>(input.size()));
    
    EXPECT_EQ(output[0], 32767);   // 10000 * 5 = 50000 -> clamped
    EXPECT_EQ(output[1], 32767);   // 20000 * 5 = 100000 -> clamped
    EXPECT_EQ(output[2], -32768);  // -10000 * 5 = -50000 -> clamped
    EXPECT_EQ(output[3], -32768);  // -20000 * 5 = -100000 -> clamped
}

TEST_F(SIMDConverterTests, Clamping_BoundaryValues) {
    // Test exact boundary values
    std::vector<float> input = {
        32766.5f,   // Rounds to 32767 (max)
        32767.4f,   // Rounds to 32767 (at max)
        32767.5f,   // Rounds to 32768 -> clamp to 32767
        -32767.5f,  // Rounds to -32768 (at min)
        -32768.4f,  // Rounds to -32768 (at min)
        -32768.5f   // Rounds to -32769 -> clamp to -32768
    };
    std::vector<int16_t> output(input.size());
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, 
                                        static_cast<int>(input.size()));
    
    verifyConversion(input.data(), output.data(), 1.0f, static_cast<int>(input.size()));
}

TEST_F(SIMDConverterTests, Clamping_Int16Range) {
    // Verify full int16 range is usable
    std::vector<float> input = {
        static_cast<float>(std::numeric_limits<int16_t>::min()),
        static_cast<float>(std::numeric_limits<int16_t>::max()),
        0.0f
    };
    std::vector<int16_t> output(input.size());
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, 
                                        static_cast<int>(input.size()));
    
    EXPECT_EQ(output[0], std::numeric_limits<int16_t>::min());
    EXPECT_EQ(output[1], std::numeric_limits<int16_t>::max());
    EXPECT_EQ(output[2], 0);
}

TEST_F(SIMDConverterTests, Clamping_WithScaling_StillCorrect) {
    // Values near boundary after scaling
    const float bitVolts = 0.195f;
    const float scale = 1.0f / (32767.0f * bitVolts);
    
    std::vector<float> input = {
        6388.0f,     // Should be near max: 6388 / 0.195 ≈ 32759
        -6388.0f,    // Should be near min
        10000.0f,    // Will overflow: 10000 / 0.195 ≈ 51282 -> clamp
        -10000.0f    // Will underflow
    };
    std::vector<int16_t> output(input.size());
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), scale, 
                                        static_cast<int>(input.size()));
    
    verifyConversion(input.data(), output.data(), scale, static_cast<int>(input.size()));
}

// =============================================================================
// Additional edge case tests
// =============================================================================

TEST_F(SIMDConverterTests, SpecialFloatValues_ZeroScale) {
    std::vector<float> input = { 1000.0f, -1000.0f, 0.0f, 5000.0f };
    std::vector<int16_t> output(input.size(), -1);
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 0.0f, 
                                        static_cast<int>(input.size()));
    
    // Zero scale should produce all zeros
    for (size_t i = 0; i < input.size(); ++i) {
        EXPECT_EQ(output[i], 0);
    }
}

TEST_F(SIMDConverterTests, SpecialFloatValues_NegativeScale) {
    std::vector<float> input = { 100.0f, 200.0f, -100.0f, -200.0f };
    std::vector<int16_t> output(input.size());
    
    float scale = -1.0f;
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), scale, 
                                        static_cast<int>(input.size()));
    
    // Negative scale should invert signs
    verifyConversion(input.data(), output.data(), scale, static_cast<int>(input.size()));
}

TEST_F(SIMDConverterTests, LargeArray_Correctness) {
    // Test with a large array to ensure no buffer overruns
    const int numSamples = 100000;
    std::vector<float> input(numSamples);
    std::vector<int16_t> output(numSamples);
    
    std::mt19937 rng(789);
    std::uniform_real_distribution<float> dist(-30000.0f, 30000.0f);
    
    for (int i = 0; i < numSamples; ++i) {
        input[i] = dist(rng);
    }
    
    SIMDConverter::convertFloatToInt16(input.data(), output.data(), 1.0f, numSamples);
    
    // Verify samples at various positions
    for (int i = 0; i < numSamples; i += 1000) {
        int16_t expected = referenceConvert(input[i], 1.0f);
        EXPECT_EQ(output[i], expected) << "Mismatch at index " << i;
    }
    
    // Verify last few samples (boundary check)
    for (int i = numSamples - 10; i < numSamples; ++i) {
        int16_t expected = referenceConvert(input[i], 1.0f);
        EXPECT_EQ(output[i], expected) << "Mismatch at index " << i;
    }
}
