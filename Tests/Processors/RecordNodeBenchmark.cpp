/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/**
 * RecordNode Performance Benchmarks
 * 
 * This file contains benchmarks to measure the baseline performance of
 * the RecordNode's disk writing operations. These benchmarks establish
 * baseline metrics for optimization work.
 * 
 * Key metrics measured:
 * - Float-to-int16 conversion throughput (MB/s)
 * - Interleaved write throughput (MB/s)
 * - End-to-end recording throughput (samples/sec, MB/s)
 * - Per-channel vs batch operation overhead
 * 
 * Test configurations:
 * - 384 channels @ 30kHz (Neuropixels 1.0 single probe)
 * - 768 channels @ 30kHz (2x probes)
 * - 1536 channels @ 30kHz (4x probes)
 */

#include <stdio.h>
#include "gtest/gtest.h"

#include <Processors/RecordNode/RecordNode.h>
#include <Processors/RecordNode/BinaryFormat/BinaryRecording.h>
#include <Processors/RecordNode/BinaryFormat/SequentialBlockFile.h>
#include <Processors/RecordNode/BinaryFormat/SIMDConverter.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>

#include <chrono>
#include <thread>
#include <filesystem>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <fstream>

using namespace std::chrono;

/**
 * Benchmark result structure for collecting and reporting performance metrics
 */
struct BenchmarkResult
{
    std::string testName;
    int numChannels;
    int numSamples;
    int iterations;
    double totalTimeMs;
    double avgTimeMs;
    double samplesPerSecond;
    double megabytesPerSecond;
    double channelSamplesPerSecond;
    
    void print() const
    {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "\n========================================\n";
        std::cout << "Benchmark: " << testName << "\n";
        std::cout << "----------------------------------------\n";
        std::cout << "Configuration:\n";
        std::cout << "  Channels:    " << numChannels << "\n";
        std::cout << "  Samples:     " << numSamples << "\n";
        std::cout << "  Iterations:  " << iterations << "\n";
        std::cout << "Results:\n";
        std::cout << "  Total time:  " << totalTimeMs << " ms\n";
        std::cout << "  Avg time:    " << avgTimeMs << " ms/iteration\n";
        std::cout << "  Throughput:  " << samplesPerSecond / 1e6 << " M samples/sec\n";
        std::cout << "  Throughput:  " << megabytesPerSecond << " MB/sec\n";
        std::cout << "  Per-channel: " << channelSamplesPerSecond / 1e6 << " M ch-samples/sec\n";
        std::cout << "========================================\n";
    }
    
    void writeToCSV(std::ofstream& file) const
    {
        file << testName << ","
             << numChannels << ","
             << numSamples << ","
             << iterations << ","
             << totalTimeMs << ","
             << avgTimeMs << ","
             << samplesPerSecond << ","
             << megabytesPerSecond << ","
             << channelSamplesPerSecond << "\n";
    }
};

/**
 * Benchmark fixture for RecordNode performance testing
 */
class RecordNodeBenchmark : public testing::Test
{
protected:
    void SetUp() override
    {
        // Default configuration - can be overridden in specific tests
        numChannels = 384;
        sampleRate = 30000.0f;
        bitVolts = 0.195f; // Neuropixels default
        
        parentRecordingDir = std::filesystem::temp_directory_path() / "record_node_benchmark";
        if (std::filesystem::exists(parentRecordingDir))
        {
            std::filesystem::remove_all(parentRecordingDir);
        }
        std::filesystem::create_directory(parentRecordingDir);
    }

    void TearDown() override
    {
        std::error_code ec;
        std::filesystem::remove_all(parentRecordingDir, ec);
    }

    /**
     * Initialize the ProcessorTester with specified channel count
     */
    void initializeTester(int channels, float rate = 30000.0f, float bv = 0.195f)
    {
        numChannels = channels;
        sampleRate = rate;
        bitVolts = bv;
        
        tester = std::make_unique<ProcessorTester>(TestSourceNodeBuilder(
            FakeSourceNodeParams{
                numChannels,
                sampleRate,
                bitVolts
            }));
        
        tester->setRecordingParentDirectory(parentRecordingDir.string());
        processor = tester->createProcessor<RecordNode>(Plugin::Processor::RECORD_NODE);
    }

    /**
     * Create a buffer with random-ish data that's representative of neural signals
     */
    AudioBuffer<float> createSignalBuffer(int channels, int samples)
    {
        AudioBuffer<float> buffer(channels, samples);
        
        // Fill with pseudo-random values in typical neural signal range
        // Using a simple pattern rather than true random for reproducibility
        for (int ch = 0; ch < channels; ch++)
        {
            float* data = buffer.getWritePointer(ch);
            for (int s = 0; s < samples; s++)
            {
                // Simulate neural signal: small oscillations with occasional spikes
                float baseSignal = 50.0f * std::sin(2.0f * 3.14159f * s / 100.0f + ch * 0.1f);
                float noise = ((s * 17 + ch * 31) % 100 - 50) * 0.5f;
                data[s] = baseSignal + noise;
            }
        }
        return buffer;
    }

    /**
     * Process a block through the RecordNode
     */
    void writeBlock(AudioBuffer<float>& buffer)
    {
        auto outBuffer = tester->processBlock(processor, buffer);
    }

    /**
     * Benchmark the float-to-int16 conversion in isolation
     * This directly tests the conversion used in BinaryRecording::writeContinuousData
     */
    BenchmarkResult benchmarkFloatToInt16Conversion(int channels, int samples, int iterations)
    {
        // Allocate buffers
        HeapBlock<float> inputBuffer(samples);
        HeapBlock<float> scaledBuffer(samples);
        HeapBlock<int16> outputBuffer(samples);
        
        // Fill input with test data
        for (int i = 0; i < samples; i++)
        {
            inputBuffer[i] = 100.0f * std::sin(2.0f * 3.14159f * i / 100.0f);
        }
        
        float multFactor = 1.0f / (float(0x7fff) * bitVolts);
        
        // Warm-up
        for (int i = 0; i < 10; i++)
        {
            FloatVectorOperations::copyWithMultiply(scaledBuffer.getData(), inputBuffer.getData(), multFactor, samples);
            AudioDataConverters::convertFloatToInt16LE(scaledBuffer.getData(), outputBuffer.getData(), samples);
        }
        
        // Benchmark
        auto startTime = high_resolution_clock::now();
        
        for (int iter = 0; iter < iterations; iter++)
        {
            for (int ch = 0; ch < channels; ch++)
            {
                // This mimics what BinaryRecording::writeContinuousData does
                FloatVectorOperations::copyWithMultiply(scaledBuffer.getData(), inputBuffer.getData(), multFactor, samples);
                AudioDataConverters::convertFloatToInt16LE(scaledBuffer.getData(), outputBuffer.getData(), samples);
            }
        }
        
        auto endTime = high_resolution_clock::now();
        double totalTimeMs = duration_cast<microseconds>(endTime - startTime).count() / 1000.0;
        
        BenchmarkResult result;
        result.testName = "Float-to-Int16 Conversion";
        result.numChannels = channels;
        result.numSamples = samples;
        result.iterations = iterations;
        result.totalTimeMs = totalTimeMs;
        result.avgTimeMs = totalTimeMs / iterations;
        
        int64_t totalSamples = (int64_t)channels * samples * iterations;
        result.samplesPerSecond = totalSamples / (totalTimeMs / 1000.0);
        result.megabytesPerSecond = (totalSamples * sizeof(int16)) / (totalTimeMs / 1000.0) / 1e6;
        result.channelSamplesPerSecond = result.samplesPerSecond;
        
        return result;
    }

    /**
     * Benchmark the interleaved write operation in SequentialBlockFile
     */
    BenchmarkResult benchmarkInterleavedWrite(int channels, int samples, int iterations)
    {
        // Create temporary file for benchmark
        auto tempFile = parentRecordingDir / "benchmark_interleave.dat";
        
        // Allocate test data
        std::vector<HeapBlock<int16>> channelData(channels);
        for (int ch = 0; ch < channels; ch++)
        {
            channelData[ch].malloc(samples);
            for (int s = 0; s < samples; s++)
            {
                channelData[ch][s] = static_cast<int16>((ch * 100 + s) % 32767);
            }
        }
        
        double totalTimeMs = 0.0;
        
        for (int iter = 0; iter < iterations; iter++)
        {
            // Create fresh file for each iteration
            if (std::filesystem::exists(tempFile))
            {
                std::filesystem::remove(tempFile);
            }
            
            SequentialBlockFile blockFile(channels, 4096);
            bool opened = blockFile.openFile(tempFile.string());
            EXPECT_TRUE(opened);
            
            auto startTime = high_resolution_clock::now();
            
            // Write all channels (mimics RecordThread behavior)
            for (int ch = 0; ch < channels; ch++)
            {
                blockFile.writeChannel(0, ch, channelData[ch].getData(), samples);
            }
            
            auto endTime = high_resolution_clock::now();
            totalTimeMs += duration_cast<microseconds>(endTime - startTime).count() / 1000.0;
        }
        
        // Cleanup
        if (std::filesystem::exists(tempFile))
        {
            std::filesystem::remove(tempFile);
        }
        
        BenchmarkResult result;
        result.testName = "Interleaved Write (SequentialBlockFile)";
        result.numChannels = channels;
        result.numSamples = samples;
        result.iterations = iterations;
        result.totalTimeMs = totalTimeMs;
        result.avgTimeMs = totalTimeMs / iterations;
        
        int64_t totalSamples = (int64_t)channels * samples * iterations;
        result.samplesPerSecond = totalSamples / (totalTimeMs / 1000.0);
        result.megabytesPerSecond = (totalSamples * sizeof(int16)) / (totalTimeMs / 1000.0) / 1e6;
        result.channelSamplesPerSecond = result.samplesPerSecond;
        
        return result;
    }

    /**
     * Benchmark end-to-end recording throughput
     */
    BenchmarkResult benchmarkEndToEndRecording(int channels, int samplesPerBlock, int numBlocks)
    {
        initializeTester(channels, sampleRate, bitVolts);
        
        // Pre-create all buffers to avoid allocation during timing
        std::vector<AudioBuffer<float>> buffers;
        for (int i = 0; i < numBlocks; i++)
        {
            buffers.push_back(createSignalBuffer(channels, samplesPerBlock));
        }
        
        // Start acquisition and recording
        tester->startAcquisition(true);
        
        // Small delay to ensure everything is initialized
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        auto startTime = high_resolution_clock::now();
        
        // Process all blocks
        for (int i = 0; i < numBlocks; i++)
        {
            writeBlock(buffers[i]);
        }
        
        auto endTime = high_resolution_clock::now();
        
        // Stop recording
        tester->stopAcquisition();
        
        double totalTimeMs = duration_cast<microseconds>(endTime - startTime).count() / 1000.0;
        
        BenchmarkResult result;
        result.testName = "End-to-End Recording";
        result.numChannels = channels;
        result.numSamples = samplesPerBlock * numBlocks;
        result.iterations = numBlocks;
        result.totalTimeMs = totalTimeMs;
        result.avgTimeMs = totalTimeMs / numBlocks;
        
        int64_t totalSamples = (int64_t)channels * samplesPerBlock * numBlocks;
        result.samplesPerSecond = totalSamples / (totalTimeMs / 1000.0);
        result.megabytesPerSecond = (totalSamples * sizeof(int16)) / (totalTimeMs / 1000.0) / 1e6;
        result.channelSamplesPerSecond = result.samplesPerSecond;
        
        // Reset tester for next test
        tester.reset();
        
        return result;
    }

    /**
     * Calculate the theoretical maximum throughput based on sample rate
     */
    static double calculateTheoreticalThroughput(int channels, float sampleRate)
    {
        // Samples per second * bytes per sample
        return channels * sampleRate * sizeof(int16);
    }

    /**
     * Check if throughput meets real-time requirements
     */
    static bool meetsRealTimeRequirement(const BenchmarkResult& result, float sampleRate)
    {
        double requiredMBps = (result.numChannels * sampleRate * sizeof(int16)) / 1e6;
        // Need at least 2x margin for safety
        return result.megabytesPerSecond >= requiredMBps * 2.0;
    }

protected:
    RecordNode* processor = nullptr;
    int numChannels;
    float sampleRate;
    float bitVolts;
    std::unique_ptr<ProcessorTester> tester;
    std::filesystem::path parentRecordingDir;
};

// ============================================================================
// FLOAT-TO-INT16 CONVERSION BENCHMARKS
// ============================================================================

TEST_F(RecordNodeBenchmark, FloatToInt16_384ch_4096samples)
{
    auto result = benchmarkFloatToInt16Conversion(384, 4096, 100);
    result.print();
    
    // Baseline expectation: should be at least 200 MB/s
    EXPECT_GT(result.megabytesPerSecond, 100.0) 
        << "Float-to-int16 conversion is slower than expected";
}

TEST_F(RecordNodeBenchmark, FloatToInt16_768ch_4096samples)
{
    auto result = benchmarkFloatToInt16Conversion(768, 4096, 50);
    result.print();
    
    EXPECT_GT(result.megabytesPerSecond, 100.0);
}

TEST_F(RecordNodeBenchmark, FloatToInt16_1536ch_4096samples)
{
    auto result = benchmarkFloatToInt16Conversion(1536, 4096, 25);
    result.print();
    
    EXPECT_GT(result.megabytesPerSecond, 100.0);
}

// ============================================================================
// INTERLEAVED WRITE BENCHMARKS
// ============================================================================

TEST_F(RecordNodeBenchmark, InterleavedWrite_384ch_4096samples)
{
    auto result = benchmarkInterleavedWrite(384, 4096, 50);
    result.print();
    
    // Baseline expectation: should be at least 150 MB/s
    EXPECT_GT(result.megabytesPerSecond, 50.0)
        << "Interleaved write is slower than expected";
}

TEST_F(RecordNodeBenchmark, InterleavedWrite_768ch_4096samples)
{
    auto result = benchmarkInterleavedWrite(768, 4096, 25);
    result.print();
    
    EXPECT_GT(result.megabytesPerSecond, 50.0);
}

TEST_F(RecordNodeBenchmark, InterleavedWrite_1536ch_4096samples)
{
    auto result = benchmarkInterleavedWrite(1536, 4096, 10);
    result.print();
    
    EXPECT_GT(result.megabytesPerSecond, 50.0);
}

// ============================================================================
// END-TO-END RECORDING BENCHMARKS
// ============================================================================

TEST_F(RecordNodeBenchmark, EndToEnd_384ch_30kHz_10sec)
{
    int channels = 384;
    int samplesPerBlock = 1024;
    float sampleRate = 30000.0f;
    int numBlocks = static_cast<int>(10.0 * sampleRate / samplesPerBlock);
    
    auto result = benchmarkEndToEndRecording(channels, samplesPerBlock, numBlocks);
    result.print();
    
    // Check if we can sustain real-time recording
    double requiredMBps = (channels * sampleRate * sizeof(int16)) / 1e6;
    std::cout << "Required throughput for real-time: " << requiredMBps << " MB/s\n";
    std::cout << "Achieved throughput: " << result.megabytesPerSecond << " MB/s\n";
    std::cout << "Margin: " << (result.megabytesPerSecond / requiredMBps) << "x\n";
    
    EXPECT_TRUE(meetsRealTimeRequirement(result, sampleRate))
        << "Cannot sustain real-time recording for " << channels << " channels at " << sampleRate << " Hz";
}

TEST_F(RecordNodeBenchmark, EndToEnd_768ch_30kHz_5sec)
{
    int channels = 768;
    int samplesPerBlock = 1024;
    float sampleRate = 30000.0f;
    int numBlocks = static_cast<int>(5.0 * sampleRate / samplesPerBlock);
    
    auto result = benchmarkEndToEndRecording(channels, samplesPerBlock, numBlocks);
    result.print();
    
    double requiredMBps = (channels * sampleRate * sizeof(int16)) / 1e6;
    std::cout << "Required throughput for real-time: " << requiredMBps << " MB/s\n";
    std::cout << "Achieved throughput: " << result.megabytesPerSecond << " MB/s\n";
    std::cout << "Margin: " << (result.megabytesPerSecond / requiredMBps) << "x\n";
}

TEST_F(RecordNodeBenchmark, EndToEnd_1536ch_30kHz_5sec)
{
    int channels = 1536;
    int samplesPerBlock = 1024;
    float sampleRate = 30000.0f;
    int numBlocks = static_cast<int>(5.0 * sampleRate / samplesPerBlock);
    
    auto result = benchmarkEndToEndRecording(channels, samplesPerBlock, numBlocks);
    result.print();
    
    double requiredMBps = (channels * sampleRate * sizeof(int16)) / 1e6;
    std::cout << "Required throughput for real-time: " << requiredMBps << " MB/s\n";
    std::cout << "Achieved throughput: " << result.megabytesPerSecond << " MB/s\n";
    std::cout << "Margin: " << (result.megabytesPerSecond / requiredMBps) << "x\n";
}

// ============================================================================
// BUFFER SIZE IMPACT BENCHMARKS
// ============================================================================

TEST_F(RecordNodeBenchmark, BufferSizeImpact_384ch)
{
    std::cout << "\n========================================\n";
    std::cout << "Buffer Size Impact Analysis (384 channels)\n";
    std::cout << "========================================\n";
    
    std::vector<int> bufferSizes = {256, 512, 1024, 2048, 4096, 8192};
    
    for (int bufSize : bufferSizes)
    {
        auto result = benchmarkFloatToInt16Conversion(384, bufSize, 100);
        std::cout << "Buffer size " << bufSize << ": " 
                  << result.megabytesPerSecond << " MB/s\n";
    }
}

// ============================================================================
// SIMD CONVERTER CORRECTNESS TESTS
// ============================================================================

/**
 * Test fixture for SIMD converter correctness testing
 */
class SIMDConverterTest : public testing::Test
{
protected:
    void SetUp() override
    {
        bitVolts = 0.195f; // Neuropixels default
    }

    /**
     * Performs the original two-pass conversion (JUCE methods)
     */
    void originalConversion(const float* input, int16* output, float scale, int numSamples)
    {
        HeapBlock<float> scaledBuffer(numSamples);
        FloatVectorOperations::copyWithMultiply(scaledBuffer.getData(), input, scale, numSamples);
        AudioDataConverters::convertFloatToInt16LE(scaledBuffer.getData(), output, numSamples);
    }

    /**
     * Compare two int16 arrays, allowing for small rounding differences
     */
    bool compareOutputs(const int16* expected, const int16* actual, int numSamples, int maxDiff = 1)
    {
        int numDifferences = 0;
        int maxObservedDiff = 0;
        
        for (int i = 0; i < numSamples; i++)
        {
            int diff = std::abs(expected[i] - actual[i]);
            if (diff > maxDiff)
            {
                numDifferences++;
                if (numDifferences <= 10)
                {
                    std::cout << "  Sample " << i << ": expected=" << expected[i] 
                              << ", actual=" << actual[i] << ", diff=" << diff << "\n";
                }
            }
            maxObservedDiff = std::max(maxObservedDiff, diff);
        }
        
        if (numDifferences > 0)
        {
            std::cout << "Total differences > " << maxDiff << ": " << numDifferences 
                      << " / " << numSamples << "\n";
            std::cout << "Max observed difference: " << maxObservedDiff << "\n";
        }
        
        return numDifferences == 0;
    }

    float bitVolts;
};

TEST_F(SIMDConverterTest, DetectsAvailableSIMD)
{
    auto simdType = SIMDConverter::getAvailableSIMD();
    std::cout << "Detected SIMD type: " << SIMDConverter::getSIMDTypeString() << "\n";
    
    // At minimum, we should have scalar support
    // On modern systems, we expect NEON or SSE2
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    EXPECT_EQ(simdType, SIMDConverter::SIMDType::NEON);
#elif defined(__SSE2__) || defined(_M_X64) || defined(_M_IX86)
    EXPECT_TRUE(simdType == SIMDConverter::SIMDType::SSE2 || 
                simdType == SIMDConverter::SIMDType::SSE4_1);
#endif
}

TEST_F(SIMDConverterTest, DebugSimpleConversion)
{
    // Super simple test: convert 8 values
    const int numSamples = 8;
    
    float input[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    int16_t output[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    
    // Scale factor of 1.0 should just convert floats to ints
    SIMDConverter::convertFloatToInt16(input, output, 1.0f, numSamples);
    
    std::cout << "Debug output after conversion with scale=1.0:\n";
    for (int i = 0; i < 8; i++)
    {
        std::cout << "  output[" << i << "] = " << output[i] << " (expected ~" << static_cast<int>(input[i]) << ")\n";
    }
    
    // Check that we got reasonable values
    EXPECT_EQ(output[0], 1);
    EXPECT_EQ(output[1], 2);
    EXPECT_EQ(output[2], 3);
    EXPECT_EQ(output[3], 4);
    EXPECT_EQ(output[4], 5);
    EXPECT_EQ(output[5], 6);
    EXPECT_EQ(output[6], 7);
    EXPECT_EQ(output[7], 8);
    
    // Now test with the typical scale factor used in the original conversion tests
    float bitVolts = 0.195f;
    float scaleFactor = 1.0f / (float(0x7fff) * bitVolts);
    std::cout << "\nScale factor: " << scaleFactor << " (bitVolts=" << bitVolts << ")\n";
    
    // Use the same input values as the original SmallBuffer test
    float input2[8];
    for (int i = 0; i < 8; i++)
    {
        input2[i] = 100.0f * std::sin(2.0f * 3.14159f * i / 30.0f);
    }
    
    int16_t output2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    SIMDConverter::convertFloatToInt16(input2, output2, scaleFactor, numSamples);
    
    std::cout << "Debug output after conversion with typical scale factor:\n";
    for (int i = 0; i < 8; i++)
    {
        float expected = input2[i] * scaleFactor;
        std::cout << "  input[" << i << "] = " << input2[i] 
                  << ", scaled = " << expected
                  << ", output = " << output2[i] << "\n";
    }
}

TEST_F(SIMDConverterTest, MatchesOriginalConversion_SmallBuffer)
{
    const int numSamples = 64;
    
    // The original workflow is:
    // 1. FloatVectorOperations::copyWithMultiply(scaled, input, 1/(32767*bitVolts), size)
    // 2. AudioDataConverters::convertFloatToInt16LE(scaled, output, size) which does: output = round(32767 * scaled)
    // Net effect: output = round(input / bitVolts)
    //
    // For our SIMD converter, we use scaleFactor = 1/bitVolts directly
    float simdScaleFactor = 1.0f / bitVolts;
    
    // For the original method, we still use the two-pass approach with its scale factor
    float originalScaleFactor = 1.0f / (float(0x7fff) * bitVolts);
    
    // Create test input: typical neural signal range
    std::vector<float> input(numSamples);
    for (int i = 0; i < numSamples; i++)
    {
        input[i] = 100.0f * std::sin(2.0f * 3.14159f * i / 30.0f);
    }
    
    // Convert using original method (with its scale factor)
    std::vector<int16> expected(numSamples);
    originalConversion(input.data(), expected.data(), originalScaleFactor, numSamples);
    
    // Convert using SIMD converter (with the correct scale factor for single-pass)
    std::vector<int16_t> actual(numSamples);
    SIMDConverter::convertFloatToInt16(input.data(), actual.data(), simdScaleFactor, numSamples);
    
    // Compare results
    EXPECT_TRUE(compareOutputs(expected.data(), reinterpret_cast<const int16*>(actual.data()), numSamples))
        << "SIMD conversion output differs from original conversion";
}

TEST_F(SIMDConverterTest, MatchesOriginalConversion_LargeBuffer)
{
    const int numSamples = 4096;
    float simdScaleFactor = 1.0f / bitVolts;
    float originalScaleFactor = 1.0f / (float(0x7fff) * bitVolts);
    
    // Create test input with varied data
    std::vector<float> input(numSamples);
    for (int i = 0; i < numSamples; i++)
    {
        // Mix of signals at different frequencies and amplitudes
        input[i] = 50.0f * std::sin(2.0f * 3.14159f * i / 100.0f)
                 + 30.0f * std::sin(2.0f * 3.14159f * i / 17.0f)
                 + ((i * 31 + 17) % 100 - 50) * 0.3f; // pseudo-random noise
    }
    
    // Convert using original method
    std::vector<int16> expected(numSamples);
    originalConversion(input.data(), expected.data(), originalScaleFactor, numSamples);
    
    // Convert using SIMD converter
    std::vector<int16_t> actual(numSamples);
    SIMDConverter::convertFloatToInt16(input.data(), actual.data(), simdScaleFactor, numSamples);
    
    // Compare results (allow 1 LSB difference due to rounding)
    EXPECT_TRUE(compareOutputs(expected.data(), reinterpret_cast<const int16*>(actual.data()), numSamples, 1))
        << "SIMD conversion output differs from original conversion";
}

TEST_F(SIMDConverterTest, MatchesOriginalConversion_EdgeCases)
{
    float simdScaleFactor = 1.0f / bitVolts;
    float originalScaleFactor = 1.0f / (float(0x7fff) * bitVolts);
    
    // Test edge cases: values near clipping boundaries
    std::vector<float> input = {
        0.0f,                               // Zero
        1.0f, -1.0f,                        // Small values
        5000.0f, -5000.0f,                  // Mid-range values
        32767.0f * bitVolts,                // Near positive max
        -32768.0f * bitVolts,               // Near negative max
        40000.0f * bitVolts,                // Should clip to +32767
        -40000.0f * bitVolts,               // Should clip to -32768
        0.5f, -0.5f,                        // Values that need rounding
        100.0f * bitVolts + 0.5f,           // Rounding up
        100.0f * bitVolts - 0.5f,           // Rounding down
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f  // Padding to make it SIMD-friendly size
    };
    int numSamples = static_cast<int>(input.size());
    
    // Convert using original method
    std::vector<int16> expected(numSamples);
    originalConversion(input.data(), expected.data(), originalScaleFactor, numSamples);
    
    // Convert using SIMD converter
    std::vector<int16_t> actual(numSamples);
    SIMDConverter::convertFloatToInt16(input.data(), actual.data(), simdScaleFactor, numSamples);
    
    // Compare (allow 1 LSB difference for rounding)
    EXPECT_TRUE(compareOutputs(expected.data(), reinterpret_cast<const int16*>(actual.data()), numSamples, 1))
        << "SIMD conversion output differs from original on edge cases";
}

TEST_F(SIMDConverterTest, MatchesOriginalConversion_NonAlignedSizes)
{
    float simdScaleFactor = 1.0f / bitVolts;
    float originalScaleFactor = 1.0f / (float(0x7fff) * bitVolts);
    
    // Test non-SIMD-aligned sizes (not multiples of 8)
    std::vector<int> testSizes = {1, 3, 7, 15, 17, 31, 33, 63, 65, 100, 127, 129, 255, 257};
    
    for (int numSamples : testSizes)
    {
        std::vector<float> input(numSamples);
        for (int i = 0; i < numSamples; i++)
        {
            input[i] = 80.0f * std::sin(2.0f * 3.14159f * i / 50.0f);
        }
        
        std::vector<int16> expected(numSamples);
        originalConversion(input.data(), expected.data(), originalScaleFactor, numSamples);
        
        std::vector<int16_t> actual(numSamples);
        SIMDConverter::convertFloatToInt16(input.data(), actual.data(), simdScaleFactor, numSamples);
        
        EXPECT_TRUE(compareOutputs(expected.data(), reinterpret_cast<const int16*>(actual.data()), numSamples, 1))
            << "SIMD conversion failed for size " << numSamples;
    }
}

TEST_F(SIMDConverterTest, BatchConversionCorrectness)
{
    const int numChannels = 4;
    const int numSamples = 256;
    float bitVoltsValues[] = {0.195f, 0.195f, 0.1f, 0.3f};
    
    // Allocate input/output arrays
    std::vector<std::vector<float>> inputData(numChannels, std::vector<float>(numSamples));
    std::vector<std::vector<int16_t>> outputData(numChannels, std::vector<int16_t>(numSamples));
    std::vector<std::vector<int16>> expectedData(numChannels, std::vector<int16>(numSamples));
    
    std::vector<const float*> inputs(numChannels);
    std::vector<int16_t*> outputs(numChannels);
    std::vector<float> simdScaleFactors(numChannels);
    
    // Fill input data and compute expected results
    for (int ch = 0; ch < numChannels; ch++)
    {
        for (int i = 0; i < numSamples; i++)
        {
            inputData[ch][i] = 60.0f * std::sin(2.0f * 3.14159f * i / 40.0f + ch * 0.5f);
        }
        
        float originalScaleFactor = 1.0f / (float(0x7fff) * bitVoltsValues[ch]);
        simdScaleFactors[ch] = 1.0f / bitVoltsValues[ch];
        inputs[ch] = inputData[ch].data();
        outputs[ch] = outputData[ch].data();
        
        // Compute expected output using original method
        originalConversion(inputData[ch].data(), expectedData[ch].data(), originalScaleFactor, numSamples);
    }
    
    // Run batch conversion with SIMD scale factors
    SIMDConverter::convertFloatToInt16Batch(inputs.data(), outputs.data(), simdScaleFactors.data(), numChannels, numSamples);
    
    // Compare results for each channel
    for (int ch = 0; ch < numChannels; ch++)
    {
        EXPECT_TRUE(compareOutputs(expectedData[ch].data(), reinterpret_cast<const int16*>(outputData[ch].data()), numSamples, 1))
            << "Batch conversion failed for channel " << ch;
    }
}

// ============================================================================
// SIMD CONVERTER PERFORMANCE BENCHMARKS
// ============================================================================

TEST_F(RecordNodeBenchmark, SIMD_vs_Original_384ch_4096samples)
{
    const int channels = 384;
    const int samples = 4096;
    const int iterations = 100;
    float originalScaleFactor = 1.0f / (float(0x7fff) * bitVolts);
    float simdScaleFactor = 1.0f / bitVolts;
    
    // Allocate buffers
    HeapBlock<float> inputBuffer(samples);
    HeapBlock<float> scaledBuffer(samples);
    HeapBlock<int16> originalOutput(samples);
    HeapBlock<int16_t> simdOutput(samples);
    
    // Fill input
    for (int i = 0; i < samples; i++)
    {
        inputBuffer[i] = 100.0f * std::sin(2.0f * 3.14159f * i / 100.0f);
    }
    
    // Warm-up both implementations
    for (int i = 0; i < 10; i++)
    {
        FloatVectorOperations::copyWithMultiply(scaledBuffer.getData(), inputBuffer.getData(), originalScaleFactor, samples);
        AudioDataConverters::convertFloatToInt16LE(scaledBuffer.getData(), originalOutput.getData(), samples);
        SIMDConverter::convertFloatToInt16(inputBuffer.getData(), simdOutput.getData(), simdScaleFactor, samples);
    }
    
    // Benchmark original method
    auto startOriginal = high_resolution_clock::now();
    for (int iter = 0; iter < iterations; iter++)
    {
        for (int ch = 0; ch < channels; ch++)
        {
            FloatVectorOperations::copyWithMultiply(scaledBuffer.getData(), inputBuffer.getData(), originalScaleFactor, samples);
            AudioDataConverters::convertFloatToInt16LE(scaledBuffer.getData(), originalOutput.getData(), samples);
        }
    }
    auto endOriginal = high_resolution_clock::now();
    double originalTimeMs = duration_cast<microseconds>(endOriginal - startOriginal).count() / 1000.0;
    
    // Benchmark SIMD method
    auto startSIMD = high_resolution_clock::now();
    for (int iter = 0; iter < iterations; iter++)
    {
        for (int ch = 0; ch < channels; ch++)
        {
            SIMDConverter::convertFloatToInt16(inputBuffer.getData(), simdOutput.getData(), simdScaleFactor, samples);
        }
    }
    auto endSIMD = high_resolution_clock::now();
    double simdTimeMs = duration_cast<microseconds>(endSIMD - startSIMD).count() / 1000.0;
    
    // Calculate throughput
    int64_t totalSamples = (int64_t)channels * samples * iterations;
    double originalMBps = (totalSamples * sizeof(int16)) / (originalTimeMs / 1000.0) / 1e6;
    double simdMBps = (totalSamples * sizeof(int16)) / (simdTimeMs / 1000.0) / 1e6;
    double speedup = originalTimeMs / simdTimeMs;
    
    std::cout << "\n========================================\n";
    std::cout << "SIMD vs Original Comparison (" << channels << " ch, " << samples << " samples)\n";
    std::cout << "SIMD type: " << SIMDConverter::getSIMDTypeString() << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Original (JUCE):  " << std::fixed << std::setprecision(1) 
              << originalMBps << " MB/s (" << originalTimeMs << " ms)\n";
    std::cout << "SIMD Converter:   " << simdMBps << " MB/s (" << simdTimeMs << " ms)\n";
    std::cout << "Speedup:          " << std::setprecision(2) << speedup << "x\n";
    std::cout << "========================================\n";
    
    // We expect at least 1.5x improvement
    EXPECT_GT(speedup, 1.0) << "SIMD converter should be faster than original";
}

TEST_F(RecordNodeBenchmark, SIMD_vs_Original_768ch_4096samples)
{
    const int channels = 768;
    const int samples = 4096;
    const int iterations = 50;
    float originalScaleFactor = 1.0f / (float(0x7fff) * bitVolts);
    float simdScaleFactor = 1.0f / bitVolts;
    
    HeapBlock<float> inputBuffer(samples);
    HeapBlock<float> scaledBuffer(samples);
    HeapBlock<int16> originalOutput(samples);
    HeapBlock<int16_t> simdOutput(samples);
    
    for (int i = 0; i < samples; i++)
    {
        inputBuffer[i] = 100.0f * std::sin(2.0f * 3.14159f * i / 100.0f);
    }
    
    // Warm-up
    for (int i = 0; i < 10; i++)
    {
        FloatVectorOperations::copyWithMultiply(scaledBuffer.getData(), inputBuffer.getData(), originalScaleFactor, samples);
        AudioDataConverters::convertFloatToInt16LE(scaledBuffer.getData(), originalOutput.getData(), samples);
        SIMDConverter::convertFloatToInt16(inputBuffer.getData(), simdOutput.getData(), simdScaleFactor, samples);
    }
    
    // Benchmark original
    auto startOriginal = high_resolution_clock::now();
    for (int iter = 0; iter < iterations; iter++)
    {
        for (int ch = 0; ch < channels; ch++)
        {
            FloatVectorOperations::copyWithMultiply(scaledBuffer.getData(), inputBuffer.getData(), originalScaleFactor, samples);
            AudioDataConverters::convertFloatToInt16LE(scaledBuffer.getData(), originalOutput.getData(), samples);
        }
    }
    auto endOriginal = high_resolution_clock::now();
    double originalTimeMs = duration_cast<microseconds>(endOriginal - startOriginal).count() / 1000.0;
    
    // Benchmark SIMD
    auto startSIMD = high_resolution_clock::now();
    for (int iter = 0; iter < iterations; iter++)
    {
        for (int ch = 0; ch < channels; ch++)
        {
            SIMDConverter::convertFloatToInt16(inputBuffer.getData(), simdOutput.getData(), simdScaleFactor, samples);
        }
    }
    auto endSIMD = high_resolution_clock::now();
    double simdTimeMs = duration_cast<microseconds>(endSIMD - startSIMD).count() / 1000.0;
    
    int64_t totalSamples = (int64_t)channels * samples * iterations;
    double originalMBps = (totalSamples * sizeof(int16)) / (originalTimeMs / 1000.0) / 1e6;
    double simdMBps = (totalSamples * sizeof(int16)) / (simdTimeMs / 1000.0) / 1e6;
    double speedup = originalTimeMs / simdTimeMs;
    
    std::cout << "\n========================================\n";
    std::cout << "SIMD vs Original Comparison (" << channels << " ch, " << samples << " samples)\n";
    std::cout << "SIMD type: " << SIMDConverter::getSIMDTypeString() << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Original (JUCE):  " << std::fixed << std::setprecision(1) 
              << originalMBps << " MB/s (" << originalTimeMs << " ms)\n";
    std::cout << "SIMD Converter:   " << simdMBps << " MB/s (" << simdTimeMs << " ms)\n";
    std::cout << "Speedup:          " << std::setprecision(2) << speedup << "x\n";
    std::cout << "========================================\n";
    
    EXPECT_GT(speedup, 1.0);
}

TEST_F(RecordNodeBenchmark, SIMD_vs_Original_1536ch_4096samples)
{
    const int channels = 1536;
    const int samples = 4096;
    const int iterations = 25;
    float originalScaleFactor = 1.0f / (float(0x7fff) * bitVolts);
    float simdScaleFactor = 1.0f / bitVolts;
    
    HeapBlock<float> inputBuffer(samples);
    HeapBlock<float> scaledBuffer(samples);
    HeapBlock<int16> originalOutput(samples);
    HeapBlock<int16_t> simdOutput(samples);
    
    for (int i = 0; i < samples; i++)
    {
        inputBuffer[i] = 100.0f * std::sin(2.0f * 3.14159f * i / 100.0f);
    }
    
    // Warm-up
    for (int i = 0; i < 10; i++)
    {
        FloatVectorOperations::copyWithMultiply(scaledBuffer.getData(), inputBuffer.getData(), originalScaleFactor, samples);
        AudioDataConverters::convertFloatToInt16LE(scaledBuffer.getData(), originalOutput.getData(), samples);
        SIMDConverter::convertFloatToInt16(inputBuffer.getData(), simdOutput.getData(), simdScaleFactor, samples);
    }
    
    // Benchmark original
    auto startOriginal = high_resolution_clock::now();
    for (int iter = 0; iter < iterations; iter++)
    {
        for (int ch = 0; ch < channels; ch++)
        {
            FloatVectorOperations::copyWithMultiply(scaledBuffer.getData(), inputBuffer.getData(), originalScaleFactor, samples);
            AudioDataConverters::convertFloatToInt16LE(scaledBuffer.getData(), originalOutput.getData(), samples);
        }
    }
    auto endOriginal = high_resolution_clock::now();
    double originalTimeMs = duration_cast<microseconds>(endOriginal - startOriginal).count() / 1000.0;
    
    // Benchmark SIMD
    auto startSIMD = high_resolution_clock::now();
    for (int iter = 0; iter < iterations; iter++)
    {
        for (int ch = 0; ch < channels; ch++)
        {
            SIMDConverter::convertFloatToInt16(inputBuffer.getData(), simdOutput.getData(), simdScaleFactor, samples);
        }
    }
    auto endSIMD = high_resolution_clock::now();
    double simdTimeMs = duration_cast<microseconds>(endSIMD - startSIMD).count() / 1000.0;
    
    int64_t totalSamples = (int64_t)channels * samples * iterations;
    double originalMBps = (totalSamples * sizeof(int16)) / (originalTimeMs / 1000.0) / 1e6;
    double simdMBps = (totalSamples * sizeof(int16)) / (simdTimeMs / 1000.0) / 1e6;
    double speedup = originalTimeMs / simdTimeMs;
    
    std::cout << "\n========================================\n";
    std::cout << "SIMD vs Original Comparison (" << channels << " ch, " << samples << " samples)\n";
    std::cout << "SIMD type: " << SIMDConverter::getSIMDTypeString() << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Original (JUCE):  " << std::fixed << std::setprecision(1) 
              << originalMBps << " MB/s (" << originalTimeMs << " ms)\n";
    std::cout << "SIMD Converter:   " << simdMBps << " MB/s (" << simdTimeMs << " ms)\n";
    std::cout << "Speedup:          " << std::setprecision(2) << speedup << "x\n";
    std::cout << "========================================\n";
    
    EXPECT_GT(speedup, 1.0);
}

TEST_F(RecordNodeBenchmark, SIMD_GeneratePerformanceReport)
{
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "         SIMD CONVERTER PERFORMANCE REPORT\n";
    std::cout << "================================================================\n";
    std::cout << "SIMD implementation: " << SIMDConverter::getSIMDTypeString() << "\n";
    std::cout << "\n";
    
    std::vector<int> channelCounts = {384, 768, 1536};
    std::vector<int> sampleCounts = {1024, 2048, 4096, 8192};
    
    std::cout << "----------------------------------------------------------------\n";
    std::cout << "Throughput Comparison (MB/s):\n";
    std::cout << "----------------------------------------------------------------\n";
    std::cout << std::setw(10) << "Channels" << std::setw(10) << "Samples"
              << std::setw(12) << "Original" << std::setw(12) << "SIMD"
              << std::setw(10) << "Speedup" << "\n";
    
    for (int channels : channelCounts)
    {
        for (int samples : sampleCounts)
        {
            float originalScaleFactor = 1.0f / (float(0x7fff) * bitVolts);
            float simdScaleFactor = 1.0f / bitVolts;
            int iterations = std::max(10, 1000000 / (channels * samples));
            
            HeapBlock<float> inputBuffer(samples);
            HeapBlock<float> scaledBuffer(samples);
            HeapBlock<int16> originalOutput(samples);
            HeapBlock<int16_t> simdOutput(samples);
            
            for (int i = 0; i < samples; i++)
            {
                inputBuffer[i] = 100.0f * std::sin(2.0f * 3.14159f * i / 100.0f);
            }
            
            // Benchmark original
            auto startOriginal = high_resolution_clock::now();
            for (int iter = 0; iter < iterations; iter++)
            {
                for (int ch = 0; ch < channels; ch++)
                {
                    FloatVectorOperations::copyWithMultiply(scaledBuffer.getData(), inputBuffer.getData(), originalScaleFactor, samples);
                    AudioDataConverters::convertFloatToInt16LE(scaledBuffer.getData(), originalOutput.getData(), samples);
                }
            }
            auto endOriginal = high_resolution_clock::now();
            double originalTimeMs = duration_cast<microseconds>(endOriginal - startOriginal).count() / 1000.0;
            
            // Benchmark SIMD
            auto startSIMD = high_resolution_clock::now();
            for (int iter = 0; iter < iterations; iter++)
            {
                for (int ch = 0; ch < channels; ch++)
                {
                    SIMDConverter::convertFloatToInt16(inputBuffer.getData(), simdOutput.getData(), simdScaleFactor, samples);
                }
            }
            auto endSIMD = high_resolution_clock::now();
            double simdTimeMs = duration_cast<microseconds>(endSIMD - startSIMD).count() / 1000.0;
            
            int64_t totalSamples = (int64_t)channels * samples * iterations;
            double originalMBps = (totalSamples * sizeof(int16)) / (originalTimeMs / 1000.0) / 1e6;
            double simdMBps = (totalSamples * sizeof(int16)) / (simdTimeMs / 1000.0) / 1e6;
            double speedup = originalTimeMs / simdTimeMs;
            
            std::cout << std::setw(10) << channels << std::setw(10) << samples
                      << std::setw(12) << std::fixed << std::setprecision(1) << originalMBps
                      << std::setw(12) << simdMBps
                      << std::setw(10) << std::setprecision(2) << speedup << "x\n";
        }
    }
    
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "                    END OF SIMD REPORT\n";
    std::cout << "================================================================\n";
}

// ============================================================================
// BATCH WRITE BENCHMARKS
// ============================================================================

/**
 * Benchmark comparing per-channel vs batch interleaved writes in SequentialBlockFile
 */
TEST_F(RecordNodeBenchmark, BatchWrite_vs_PerChannel_Interleave_384ch)
{
    const int channels = 384;
    const int samples = 4096;
    const int iterations = 50;
    
    // Allocate test data - one buffer per channel
    std::vector<HeapBlock<int16>> channelData(channels);
    std::vector<int16*> channelPtrs(channels);
    
    for (int ch = 0; ch < channels; ch++)
    {
        channelData[ch].malloc(samples);
        channelPtrs[ch] = channelData[ch].getData();
        for (int s = 0; s < samples; s++)
        {
            channelData[ch][s] = static_cast<int16>((ch * 100 + s) % 32767);
        }
    }
    
    double perChannelTimeMs = 0.0;
    double batchTimeMs = 0.0;
    
    // Per-channel benchmark
    for (int iter = 0; iter < iterations; iter++)
    {
        auto tempFile = parentRecordingDir / ("bench_perchan_" + std::to_string(iter) + ".dat");
        
        {
            SequentialBlockFile blockFile(channels, 4096);
            blockFile.openFile(tempFile.string());
            
            auto startTime = high_resolution_clock::now();
            
            for (int ch = 0; ch < channels; ch++)
            {
                blockFile.writeChannel(0, ch, channelPtrs[ch], samples);
            }
            
            auto endTime = high_resolution_clock::now();
            perChannelTimeMs += duration_cast<microseconds>(endTime - startTime).count() / 1000.0;
        }

        std::filesystem::remove(tempFile);
    }
    
    // Batch benchmark
    for (int iter = 0; iter < iterations; iter++)
    {
        auto tempFile = parentRecordingDir / ("bench_batch_" + std::to_string(iter) + ".dat");
        {
            SequentialBlockFile blockFile(channels, 4096);
            blockFile.openFile(tempFile.string());
            
            auto startTime = high_resolution_clock::now();
            
            blockFile.writeChannelBatch(0, channelPtrs.data(), channels, samples);
            
            auto endTime = high_resolution_clock::now();
            batchTimeMs += duration_cast<microseconds>(endTime - startTime).count() / 1000.0;
        }

        std::filesystem::remove(tempFile);
    }
    
    // Calculate throughput
    int64_t totalSamples = (int64_t)channels * samples * iterations;
    double perChannelMBps = (totalSamples * sizeof(int16)) / (perChannelTimeMs / 1000.0) / 1e6;
    double batchMBps = (totalSamples * sizeof(int16)) / (batchTimeMs / 1000.0) / 1e6;
    double speedup = perChannelTimeMs / batchTimeMs;
    
    std::cout << "\n========================================\n";
    std::cout << "Batch vs Per-Channel Interleaved Write (" << channels << " ch, " << samples << " samples)\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Per-channel:  " << std::fixed << std::setprecision(1) 
              << perChannelMBps << " MB/s (" << perChannelTimeMs << " ms)\n";
    std::cout << "Batch:        " << batchMBps << " MB/s (" << batchTimeMs << " ms)\n";
    std::cout << "Speedup:      " << std::setprecision(2) << speedup << "x\n";
    std::cout << "========================================\n";
    
    // Batch write should be at least as fast (speedup >= 1.0)
    EXPECT_GE(speedup, 0.8) << "Batch write should not be significantly slower than per-channel";
}

/**
 * Benchmark comparing per-channel vs batch interleaved writes with 768 channels
 */
TEST_F(RecordNodeBenchmark, BatchWrite_vs_PerChannel_Interleave_768ch)
{
    const int channels = 768;
    const int samples = 4096;
    const int iterations = 25;
    
    std::vector<HeapBlock<int16>> channelData(channels);
    std::vector<int16*> channelPtrs(channels);
    
    for (int ch = 0; ch < channels; ch++)
    {
        channelData[ch].malloc(samples);
        channelPtrs[ch] = channelData[ch].getData();
        for (int s = 0; s < samples; s++)
        {
            channelData[ch][s] = static_cast<int16>((ch * 100 + s) % 32767);
        }
    }
    
    double perChannelTimeMs = 0.0;
    double batchTimeMs = 0.0;
    
    for (int iter = 0; iter < iterations; iter++)
    {
        auto tempFile = parentRecordingDir / ("bench_perchan_768_" + std::to_string(iter) + ".dat");
        {
            SequentialBlockFile blockFile(channels, 4096);
            blockFile.openFile(tempFile.string());
            
            auto startTime = high_resolution_clock::now();
            
            for (int ch = 0; ch < channels; ch++)
            {
                blockFile.writeChannel(0, ch, channelPtrs[ch], samples);
            }
            
            auto endTime = high_resolution_clock::now();
            perChannelTimeMs += duration_cast<microseconds>(endTime - startTime).count() / 1000.0;
        }

        std::filesystem::remove(tempFile);
    }
    
    for (int iter = 0; iter < iterations; iter++)
    {
        auto tempFile = parentRecordingDir / ("bench_batch_768_" + std::to_string(iter) + ".dat");
        {
            SequentialBlockFile blockFile(channels, 4096);
            blockFile.openFile(tempFile.string());
            
            auto startTime = high_resolution_clock::now();
            
            blockFile.writeChannelBatch(0, channelPtrs.data(), channels, samples);
            
            auto endTime = high_resolution_clock::now();
            batchTimeMs += duration_cast<microseconds>(endTime - startTime).count() / 1000.0;
        }

        std::filesystem::remove(tempFile);
    }
    
    int64_t totalSamples = (int64_t)channels * samples * iterations;
    double perChannelMBps = (totalSamples * sizeof(int16)) / (perChannelTimeMs / 1000.0) / 1e6;
    double batchMBps = (totalSamples * sizeof(int16)) / (batchTimeMs / 1000.0) / 1e6;
    double speedup = perChannelTimeMs / batchTimeMs;
    
    std::cout << "\n========================================\n";
    std::cout << "Batch vs Per-Channel Interleaved Write (" << channels << " ch, " << samples << " samples)\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Per-channel:  " << std::fixed << std::setprecision(1) 
              << perChannelMBps << " MB/s (" << perChannelTimeMs << " ms)\n";
    std::cout << "Batch:        " << batchMBps << " MB/s (" << batchTimeMs << " ms)\n";
    std::cout << "Speedup:      " << std::setprecision(2) << speedup << "x\n";
    std::cout << "========================================\n";
    
    EXPECT_GE(speedup, 0.8);
}

/**
 * Comprehensive batch write performance report
 */
TEST_F(RecordNodeBenchmark, BatchWrite_PerformanceReport)
{
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "         BATCH WRITE PERFORMANCE REPORT\n";
    std::cout << "================================================================\n";
    std::cout << "\n";
    
    std::vector<int> channelCounts = {384, 768, 1536};
    std::vector<int> sampleCounts = {1024, 2048, 4096};
    
    std::cout << "----------------------------------------------------------------\n";
    std::cout << "SequentialBlockFile Interleaved Write Comparison:\n";
    std::cout << "----------------------------------------------------------------\n";
    std::cout << std::setw(10) << "Channels" << std::setw(10) << "Samples"
              << std::setw(14) << "Per-ch MB/s" << std::setw(12) << "Batch MB/s"
              << std::setw(10) << "Speedup" << "\n";
    
    for (int channels : channelCounts)
    {
        std::vector<HeapBlock<int16>> channelData(channels);
        std::vector<int16*> channelPtrs(channels);
        
        for (int ch = 0; ch < channels; ch++)
        {
            channelData[ch].malloc(8192);  // Max sample size
            channelPtrs[ch] = channelData[ch].getData();
            for (int s = 0; s < 8192; s++)
            {
                channelData[ch][s] = static_cast<int16>((ch * 100 + s) % 32767);
            }
        }
        
        for (int samples : sampleCounts)
        {
            int iterations = std::max(5, 500000 / (channels * samples));
            
            double perChannelTimeMs = 0.0;
            double batchTimeMs = 0.0;
            
            for (int iter = 0; iter < iterations; iter++)
            {
                auto tempFile = parentRecordingDir / "bench_temp.dat";
                
                // Per-channel write
                {
                    SequentialBlockFile blockFile(channels, 4096);
                    blockFile.openFile(tempFile.string());
                    
                    auto startTime = high_resolution_clock::now();
                    for (int ch = 0; ch < channels; ch++)
                    {
                        blockFile.writeChannel(0, ch, channelPtrs[ch], samples);
                    }
                    auto endTime = high_resolution_clock::now();
                    perChannelTimeMs += duration_cast<microseconds>(endTime - startTime).count() / 1000.0;
                }
                std::filesystem::remove(tempFile);
                
                // Batch write
                {
                    SequentialBlockFile blockFile(channels, 4096);
                    blockFile.openFile(tempFile.string());
                    
                    auto startTime = high_resolution_clock::now();
                    blockFile.writeChannelBatch(0, channelPtrs.data(), channels, samples);
                    auto endTime = high_resolution_clock::now();
                    batchTimeMs += duration_cast<microseconds>(endTime - startTime).count() / 1000.0;
                }
                std::filesystem::remove(tempFile);
            }
            
            int64_t totalSamples = (int64_t)channels * samples * iterations;
            double perChannelMBps = (totalSamples * sizeof(int16)) / (perChannelTimeMs / 1000.0) / 1e6;
            double batchMBps = (totalSamples * sizeof(int16)) / (batchTimeMs / 1000.0) / 1e6;
            double speedup = perChannelTimeMs / batchTimeMs;
            
            std::cout << std::setw(10) << channels << std::setw(10) << samples
                      << std::setw(14) << std::fixed << std::setprecision(1) << perChannelMBps
                      << std::setw(12) << batchMBps
                      << std::setw(10) << std::setprecision(2) << speedup << "x\n";
        }
    }
    
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "                 END OF BATCH WRITE REPORT\n";
    std::cout << "================================================================\n";
}

// ============================================================================
// COMPREHENSIVE BASELINE REPORT
// ============================================================================

TEST_F(RecordNodeBenchmark, GenerateBaselineReport)
{
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "         RECORDNODE BASELINE PERFORMANCE REPORT\n";
    std::cout << "================================================================\n";
    std::cout << "\n";
    
    // Configuration
    std::vector<int> channelCounts = {384, 768, 1536};
    int samples = 4096;
    float rate = 30000.0f;
    
    std::cout << "Test Configuration:\n";
    std::cout << "  Sample rate: " << rate << " Hz\n";
    std::cout << "  Samples per block: " << samples << "\n";
    std::cout << "  Bit volts: " << bitVolts << "\n";
    std::cout << "\n";
    
    std::cout << "----------------------------------------------------------------\n";
    std::cout << "Float-to-Int16 Conversion Performance:\n";
    std::cout << "----------------------------------------------------------------\n";
    std::cout << std::setw(12) << "Channels" 
              << std::setw(15) << "MB/s"
              << std::setw(20) << "M samples/s" << "\n";
    
    for (int ch : channelCounts)
    {
        auto result = benchmarkFloatToInt16Conversion(ch, samples, 50);
        std::cout << std::setw(12) << ch
                  << std::setw(15) << std::fixed << std::setprecision(1) << result.megabytesPerSecond
                  << std::setw(20) << std::fixed << std::setprecision(1) << result.samplesPerSecond / 1e6
                  << "\n";
    }
    
    std::cout << "\n";
    std::cout << "----------------------------------------------------------------\n";
    std::cout << "Interleaved Write Performance:\n";
    std::cout << "----------------------------------------------------------------\n";
    std::cout << std::setw(12) << "Channels" 
              << std::setw(15) << "MB/s"
              << std::setw(20) << "M samples/s" << "\n";
    
    for (int ch : channelCounts)
    {
        auto result = benchmarkInterleavedWrite(ch, samples, 20);
        std::cout << std::setw(12) << ch
                  << std::setw(15) << std::fixed << std::setprecision(1) << result.megabytesPerSecond
                  << std::setw(20) << std::fixed << std::setprecision(1) << result.samplesPerSecond / 1e6
                  << "\n";
    }
    
    std::cout << "\n";
    std::cout << "----------------------------------------------------------------\n";
    std::cout << "Real-time Recording Requirements:\n";
    std::cout << "----------------------------------------------------------------\n";
    std::cout << std::setw(12) << "Channels"
              << std::setw(15) << "Required MB/s"
              << std::setw(15) << "@ 30kHz" << "\n";
    
    for (int ch : channelCounts)
    {
        double required = (ch * rate * sizeof(int16)) / 1e6;
        std::cout << std::setw(12) << ch
                  << std::setw(15) << std::fixed << std::setprecision(1) << required
                  << std::setw(15) << "(2x = " << required * 2 << ")"
                  << "\n";
    }
    
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "                    END OF BASELINE REPORT\n";
    std::cout << "================================================================\n";
}

// ============================================================================
// TILE SIZE TUNING BENCHMARKS
// ============================================================================

/**
 * Benchmark to test different tile sizes for the SIMD interleaving.
 * This helps find the optimal tile configuration for cache efficiency.
 */
TEST_F(RecordNodeBenchmark, TileSizeTuning_Interleaving)
{
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "         TILE SIZE TUNING FOR INTERLEAVING\n";
    std::cout << "================================================================\n";
    std::cout << "SIMD type: " << SIMDConverter::getSIMDTypeString() << "\n";
    std::cout << "\n";
    
    // Test different tile configurations
    std::vector<std::pair<int, int>> tileConfigs = {
        {64, 32},
        {128, 32},
        {128, 64},
        {256, 32},
        {256, 64},
        {256, 128},
        {512, 32},
        {512, 64},
        {1024, 64},
        {2048, 64}
    };
    
    std::vector<int> channelCounts = {384, 768, 1536};
    const int samples = 4096;
    const int iterations = 30;
    
    for (int channels : channelCounts)
    {
        std::cout << "\n----------------------------------------------------------------\n";
        std::cout << "Channel count: " << channels << "\n";
        std::cout << "----------------------------------------------------------------\n";
        std::cout << std::setw(12) << "Tile Samp" << std::setw(12) << "Tile Ch"
                  << std::setw(14) << "MB/s" << std::setw(12) << "Time (ms)" << "\n";
        
        // Allocate test data
        std::vector<HeapBlock<int16>> channelData(channels);
        std::vector<int16*> channelPtrs(channels);
        HeapBlock<int16> outputBuffer(channels * samples);
        
        for (int ch = 0; ch < channels; ch++)
        {
            channelData[ch].malloc(samples);
            channelPtrs[ch] = channelData[ch].getData();
            for (int s = 0; s < samples; s++)
            {
                channelData[ch][s] = static_cast<int16>((ch * 100 + s) % 32767);
            }
        }
        
        // Test default configuration first
        auto defaultConfig = SIMDConverter::getRecommendedTileConfig(channels);
        
        double defaultTimeMs = 0.0;
        for (int iter = 0; iter < iterations; iter++)
        {
            auto startTime = high_resolution_clock::now();
            
            // Cast to const int16_t* const*
            std::vector<const int16_t*> constPtrs(channels);
            for (int ch = 0; ch < channels; ch++)
                constPtrs[ch] = channelPtrs[ch];
            
            SIMDConverter::interleaveInt16(
                constPtrs.data(),
                outputBuffer.getData(),
                channels,
                samples,
                defaultConfig.tileSamples,
                defaultConfig.tileChannels);
            
            auto endTime = high_resolution_clock::now();
            defaultTimeMs += duration_cast<microseconds>(endTime - startTime).count() / 1000.0;
        }
        
        int64_t totalSamples = (int64_t)channels * samples * iterations;
        double defaultMBps = (totalSamples * sizeof(int16)) / (defaultTimeMs / 1000.0) / 1e6;
        
        std::cout << std::setw(12) << defaultConfig.tileSamples 
                  << std::setw(12) << defaultConfig.tileChannels
                  << std::setw(14) << std::fixed << std::setprecision(1) << defaultMBps
                  << std::setw(12) << std::setprecision(2) << defaultTimeMs
                  << " (default)\n";
        
        // Test each tile configuration
        for (const auto& config : tileConfigs)
        {
            int tileSamples = config.first;
            int tileChannels = config.second;
            
            // Skip if tile channels exceeds actual channels
            if (tileChannels > channels)
                continue;
            
            double timeMs = 0.0;
            for (int iter = 0; iter < iterations; iter++)
            {
                auto startTime = high_resolution_clock::now();
                
                std::vector<const int16_t*> constPtrs(channels);
                for (int ch = 0; ch < channels; ch++)
                    constPtrs[ch] = channelPtrs[ch];
                
                SIMDConverter::interleaveInt16(
                    constPtrs.data(),
                    outputBuffer.getData(),
                    channels,
                    samples,
                    tileSamples,
                    tileChannels);
                
                auto endTime = high_resolution_clock::now();
                timeMs += duration_cast<microseconds>(endTime - startTime).count() / 1000.0;
            }
            
            double mbps = (totalSamples * sizeof(int16)) / (timeMs / 1000.0) / 1e6;
            
            std::cout << std::setw(12) << tileSamples << std::setw(12) << tileChannels
                      << std::setw(14) << std::fixed << std::setprecision(1) << mbps
                      << std::setw(12) << std::setprecision(2) << timeMs;
            
            if (mbps > defaultMBps * 1.05)
                std::cout << " *BETTER*";
            else if (mbps < defaultMBps * 0.95)
                std::cout << " (slower)";
            
            std::cout << "\n";
        }
    }
    
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "                 END OF TILE SIZE TUNING\n";
    std::cout << "================================================================\n";
}

/**
 * Benchmark comparing SIMD interleaving with the original scalar approach.
 */
TEST_F(RecordNodeBenchmark, SIMDInterleaving_vs_Scalar)
{
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "         SIMD INTERLEAVING VS SCALAR\n";
    std::cout << "================================================================\n";
    std::cout << "SIMD type: " << SIMDConverter::getSIMDTypeString() << "\n";
    std::cout << "\n";
    
    std::vector<int> channelCounts = {384, 768, 1536};
    const int samples = 4096;
    const int iterations = 50;
    
    std::cout << std::setw(10) << "Channels" << std::setw(14) << "Scalar MB/s"
              << std::setw(14) << "SIMD MB/s" << std::setw(12) << "Speedup" << "\n";
    std::cout << "----------------------------------------------------------------\n";
    
    for (int channels : channelCounts)
    {
        // Allocate test data
        std::vector<HeapBlock<int16>> channelData(channels);
        std::vector<int16*> channelPtrs(channels);
        HeapBlock<int16> outputBuffer(channels * samples);
        
        for (int ch = 0; ch < channels; ch++)
        {
            channelData[ch].malloc(samples);
            channelPtrs[ch] = channelData[ch].getData();
            for (int s = 0; s < samples; s++)
            {
                channelData[ch][s] = static_cast<int16>((ch * 100 + s) % 32767);
            }
        }
        
        // Benchmark scalar (simple nested loop)
        double scalarTimeMs = 0.0;
        for (int iter = 0; iter < iterations; iter++)
        {
            auto startTime = high_resolution_clock::now();
            
            // Simple scalar interleaving (reference implementation)
            int16* output = outputBuffer.getData();
            for (int s = 0; s < samples; s++)
            {
                for (int ch = 0; ch < channels; ch++)
                {
                    *output++ = channelPtrs[ch][s];
                }
            }
            
            auto endTime = high_resolution_clock::now();
            scalarTimeMs += duration_cast<microseconds>(endTime - startTime).count() / 1000.0;
        }
        
        // Benchmark SIMD with recommended tile config
        auto config = SIMDConverter::getRecommendedTileConfig(channels);
        double simdTimeMs = 0.0;
        for (int iter = 0; iter < iterations; iter++)
        {
            auto startTime = high_resolution_clock::now();
            
            std::vector<const int16_t*> constPtrs(channels);
            for (int ch = 0; ch < channels; ch++)
                constPtrs[ch] = channelPtrs[ch];
            
            SIMDConverter::interleaveInt16(
                constPtrs.data(),
                outputBuffer.getData(),
                channels,
                samples,
                config.tileSamples,
                config.tileChannels);
            
            auto endTime = high_resolution_clock::now();
            simdTimeMs += duration_cast<microseconds>(endTime - startTime).count() / 1000.0;
        }
        
        int64_t totalSamples = (int64_t)channels * samples * iterations;
        double scalarMBps = (totalSamples * sizeof(int16)) / (scalarTimeMs / 1000.0) / 1e6;
        double simdMBps = (totalSamples * sizeof(int16)) / (simdTimeMs / 1000.0) / 1e6;
        double speedup = scalarTimeMs / simdTimeMs;
        
        std::cout << std::setw(10) << channels
                  << std::setw(14) << std::fixed << std::setprecision(1) << scalarMBps
                  << std::setw(14) << simdMBps
                  << std::setw(12) << std::setprecision(2) << speedup << "x\n";
    }
    
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "             END OF SIMD VS SCALAR COMPARISON\n";
    std::cout << "================================================================\n";
}
