#include <stdio.h>

#include "gtest/gtest.h"

#include <Processors/RecordNode/RecordNode.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>
#include <chrono>
#include <thread>
#include <iterator>
#include <iostream>
#include <filesystem>
#include <algorithm>

class RecordNodeTests :  public ::testing::Test {
protected:
    void SetUp() override {
        num_channels = 8;
        tester = std::make_unique<ProcessorTester>(FakeSourceNodeParams{
            num_channels,
            sample_rate_,
            bitVolts_
        });

        parent_recording_dir = std::filesystem::temp_directory_path() / "record_node_tests";
        if (std::filesystem::exists(parent_recording_dir)) {
            std::filesystem::remove_all(parent_recording_dir);
        }
        std::filesystem::create_directory(parent_recording_dir);

        // Set this before creating the record node
        tester->setRecordingParentDirectory(parent_recording_dir.string());
        processor = tester->Create<RecordNode>(Plugin::Processor::RECORD_NODE);
    }

    void TearDown() override {
        // Swallow errors
        std::error_code ec;
        std::filesystem::remove_all(parent_recording_dir, ec);
    }

    AudioBuffer<float> CreateBuffer(float starting_value, float step, int num_channels, int num_samples) {
        AudioBuffer<float> input_buffer(num_channels, num_samples);

        // in microvolts
        float cur_value = starting_value;
        for (int chidx = 0; chidx < num_channels; chidx++) {
            for (int sample_idx = 0; sample_idx < num_samples; sample_idx++) {
                input_buffer.setSample(chidx, sample_idx, cur_value);
                cur_value += step;
            }
        }
        return input_buffer;
    }

    void WriteBlock(AudioBuffer<float> &buffer) {
        auto output_buffer = tester->ProcessBlock(processor, buffer);
        // Assert the buffer hasn't changed after process()
        ASSERT_EQ(output_buffer.getNumSamples(), buffer.getNumSamples());
        ASSERT_EQ(output_buffer.getNumChannels(), buffer.getNumChannels());
        for (int chidx = 0; chidx < output_buffer.getNumChannels(); chidx++) {
            for (int sample_idx = 0; sample_idx < output_buffer.getNumSamples(); ++sample_idx) {
                ASSERT_EQ(
                    output_buffer.getSample(chidx, sample_idx),
                    buffer.getSample(chidx, sample_idx));
            }
        }
    }

    bool ContinuousPathFor(const std::string& basename, std::filesystem::path* path) {
        // Do verifications:
        auto recording_dir = std::filesystem::directory_iterator(parent_recording_dir)->path();
        std::stringstream ss;
        ss << "Record Node " << processor->getNodeId();
        auto recording_dir2 = recording_dir / ss.str() / "experiment1" / "recording1" / "continuous";
        if (!std::filesystem::exists(recording_dir2)) {
            return false;
        }

        auto recording_dir3 = std::filesystem::directory_iterator(recording_dir2)->path();
        if (!std::filesystem::exists(recording_dir3)) {
            return false;
        }

        auto ret = recording_dir3 / basename;
        if (!std::filesystem::exists(ret)) {
            return false;
        }
        *path = ret;
        return true;
    }

    void MaybeLoadContinuousDatFile(std::vector<int16_t> *output, bool *success) {
        // Do verifications:
        std::filesystem::path continuous_dat_path;
        *success = ContinuousPathFor("continuous.dat", &continuous_dat_path);
        if (!*success) {
            return;
        }

        std::ifstream continuous_ifstream(continuous_dat_path.string(), std::ios::binary | std::ios::in);

        continuous_ifstream.seekg(0, std::ios::end);
        std::streampos fileSize = continuous_ifstream.tellg();
        continuous_ifstream.seekg(0, std::ios::beg);
        if (fileSize % sizeof(int16_t) != 0) {
            *success = false;
            return;
        }

        std::vector<int16_t> persisted_data(fileSize / sizeof(int16_t));
        continuous_ifstream.read((char *) persisted_data.data(), fileSize);
        *success = true;
        *output = persisted_data;
    }

    void LoadContinuousDatFile(std::vector<int16_t> *output) {
        bool success = false;
        MaybeLoadContinuousDatFile(output, &success);
        ASSERT_TRUE(success);
    }

    void LoadNpyFileBinary(const std::string& basename, std::vector<char> *output, bool *success) {
        // Do verifications:
        std::filesystem::path npy_file_path;
        *success = ContinuousPathFor(basename, &npy_file_path);
        if (!*success) {
            return;
        }

        std::ifstream data_ifstream(npy_file_path.string(), std::ios::binary | std::ios::in);

        data_ifstream.seekg(0, std::ios::end);
        std::streampos fileSize = data_ifstream.tellg();
        data_ifstream.seekg(0, std::ios::beg);

        std::vector<char> persisted_data(fileSize);
        data_ifstream.read(persisted_data.data(), fileSize);
        *success = true;
        *output = persisted_data;
    }

    void CompareBinaryFilesHex(const std::string& filename, const std::vector<char> bin_data, const std::string& expected_bin_data_hex) {
        std::vector<char> expected_bin_data;
        for (int i = 0; i < expected_bin_data_hex.length(); i += 2) {
            std::string byteString = expected_bin_data_hex.substr(i, 2);
            char byte = (char) strtol(byteString.c_str(), nullptr, 16);
            expected_bin_data.push_back(byte);
        }

        // Create a string rep of the actual sample numbers bin in case it fails, to help debugging
        std::stringstream bin_data_hex_ss;
        bin_data_hex_ss << "Expected data for " << filename << " in hex to be=" << expected_bin_data_hex
                        << " but received=";
        bin_data_hex_ss << std::hex;
        for (int i = 0; i < bin_data.size(); i++) {
            bin_data_hex_ss << std::setw(2) << std::setfill('0') << (int)bin_data[i];
        }
        std::string err_msg = bin_data_hex_ss.str();

        ASSERT_EQ(bin_data.size(), expected_bin_data.size()) << err_msg;
        for (int i = 0; i < bin_data.size(); i++) {
            ASSERT_EQ(bin_data[i], expected_bin_data[i])
                                << err_msg
                                << " (error on index " << i << ")";
        }
    }

    static int16_t min_val_possible() {
        // The min value is actually -32767 in the math in RecordNode, not -32768 like the "true" min for int16_t
        return (std::numeric_limits<int16_t>::min)() + 1;
    }

    static int16_t max_val_possible() {
        return (std::numeric_limits<int16_t>::max)();
    }

    RecordNode *processor;
    int num_channels;
    float bitVolts_ = 1.0;
    std::unique_ptr<ProcessorTester> tester;
    std::filesystem::path parent_recording_dir;
    float sample_rate_ = 1.0;
};

TEST_F(RecordNodeTests, TestInputOutput_Continuous_Single) {
    int num_samples = 100;
    tester->startAcquisition(true);

    auto input_buffer = CreateBuffer(1000.0, 20.0, num_channels, num_samples);
    WriteBlock(input_buffer);

    // The record node always flushes its pending writes when stopping acquisition, so we don't need to sleep before
    // stopping.
    tester->stopAcquisition();

    std::vector<int16_t> persisted_data;
    LoadContinuousDatFile(&persisted_data);
    ASSERT_EQ(persisted_data.size(), num_channels * num_samples);

    int persisted_data_idx = 0;
    // File is channel-interleaved, so ensure we iterate in the correct order:
    for (int sample_idx = 0; sample_idx < num_samples; sample_idx++) {
        for (int chidx = 0; chidx < num_channels; chidx++) {
            auto expected_microvolts = input_buffer.getSample(chidx, sample_idx);
            ASSERT_EQ(persisted_data[persisted_data_idx], expected_microvolts);
            persisted_data_idx++;
        }
    }
}

TEST_F(RecordNodeTests, TestInputOutput_Continuous_Multiple) {
    tester->startAcquisition(true);

    int num_samples_per_block = 100;
    int num_blocks = 8;
    std::vector<AudioBuffer<float>> input_buffers;
    for (int i = 0; i < num_blocks; i++) {
        auto input_buffer = CreateBuffer(1000.0f * i, 20.0, num_channels, num_samples_per_block);
        WriteBlock(input_buffer);
        input_buffers.push_back(input_buffer);
    }

    tester->stopAcquisition();

    std::vector<int16_t> persisted_data;
    LoadContinuousDatFile(&persisted_data);
    ASSERT_EQ(persisted_data.size(), num_channels * num_samples_per_block * num_blocks);

    int persisted_data_idx = 0;
    // File is channel-interleaved, so ensure we iterate in the correct order:
    for (int block_idx = 0; block_idx < num_blocks; block_idx++) {
        const auto& input_buffer = input_buffers[block_idx];
        for (int sample_idx = 0; sample_idx < num_samples_per_block; sample_idx++) {
            for (int chidx = 0; chidx < num_channels; chidx++) {
                auto expected_microvolts = input_buffer.getSample(chidx, sample_idx);
                ASSERT_EQ(persisted_data[persisted_data_idx], expected_microvolts);
                persisted_data_idx++;
            }
        }
    }
}

TEST_F(RecordNodeTests, TestEmpty) {
    tester->startAcquisition(true);
    tester->stopAcquisition();

    std::vector<int16_t> persisted_data;
    LoadContinuousDatFile(&persisted_data);
    ASSERT_EQ(persisted_data.size(), 0);
}

TEST_F(RecordNodeTests, TestClipsProperly) {
    int num_samples = 100;
    tester->startAcquisition(true);

    // The min value is actually -32767, not -32768 like the "true" min
    std::vector<AudioBuffer<float>> input_buffers;

    // Write numbers both underflowing and overflowing
    auto input_buffer = CreateBuffer((float) min_val_possible() + 1, -1, num_channels, num_samples);
    WriteBlock(input_buffer);
    input_buffers.push_back(input_buffer);

    input_buffer = CreateBuffer((float) max_val_possible() - 1, 1, num_channels, num_samples);
    WriteBlock(input_buffer);
    input_buffers.push_back(input_buffer);

    tester->stopAcquisition();

    std::vector<int16_t> persisted_data;
    LoadContinuousDatFile(&persisted_data);
    ASSERT_EQ(persisted_data.size(), num_channels * num_samples * 2);

    int persisted_data_idx = 0;
    for (int block_idx = 0; block_idx < 2; block_idx++) {
        auto input_buffer = input_buffers[block_idx];
        for (int sample_idx = 0; sample_idx < num_samples; sample_idx++) {
            for (int chidx = 0; chidx < num_channels; chidx++) {
                auto expected_microvolts = input_buffer.getSample(chidx, sample_idx);

                // Per the logic above, only the very first sample/channel is within the normal bounds; the rest will
                // be clipped at the upper or lower possible values.
                int16_t expected_persisted;
                if (sample_idx == 0 && chidx == 0) {
                    expected_persisted = expected_microvolts;
                } else if (expected_microvolts > 0) {
                    expected_persisted = max_val_possible();
                } else {
                    expected_persisted = min_val_possible();
                }

                ASSERT_EQ(persisted_data[persisted_data_idx], expected_persisted);
                persisted_data_idx++;
            }
        }
    }
}

class CustomBitVolts_RecordNodeTests : public RecordNodeTests {
    void SetUp() override {
        bitVolts_ = 0.195;
        RecordNodeTests::SetUp();
    }
};

TEST_F(CustomBitVolts_RecordNodeTests, Test_RespectsBitVolts) {
    int num_samples = 100;
    tester->startAcquisition(true);
    auto input_buffer = CreateBuffer(1000.0, 20.0, num_channels, num_samples);
    WriteBlock(input_buffer);
    tester->stopAcquisition();

    std::vector<int16_t> persisted_data;
    LoadContinuousDatFile(&persisted_data);
    ASSERT_EQ(persisted_data.size(), num_channels * num_samples);

    int persisted_data_idx = 0;
    // File is channel-interleaved, so ensure we iterate in the correct order:
    for (int sample_idx = 0; sample_idx < num_samples; sample_idx++) {
        for (int chidx = 0; chidx < num_channels; chidx++) {
            auto expected_microvolts = input_buffer.getSample(chidx, sample_idx);
            auto expected_converted = expected_microvolts / bitVolts_;

            // Rounds to nearest int, like BinaryRecording does, and clamp within bounds
            int expected_rounded = juce::roundToInt(expected_converted);
            int16_t expected_persisted = (int16_t) std::clamp(
                expected_rounded,
                (int) min_val_possible(),
                (int) max_val_possible());
            ASSERT_EQ(persisted_data[persisted_data_idx], expected_persisted);
            persisted_data_idx++;
        }
    }
}

TEST_F(RecordNodeTests, Test_PersistsSampleNumbersAndTimestamps) {
    tester->startAcquisition(true);

    int num_samples = 5;
    for (int i = 0; i < 3; i++) {
        auto input_buffer = CreateBuffer(1000.0, 20.0, num_channels, num_samples);
        WriteBlock(input_buffer);
    }
    tester->stopAcquisition();

    bool success = false;
    std::vector<char> sample_numbers_bin;
    LoadNpyFileBinary("sample_numbers.npy", &sample_numbers_bin, &success);
    ASSERT_TRUE(success);

    /**
     * Since NpyFile in OpenEphys doesn't include any facility to read .npy files, we've generated the expected
     * .npy file output in Python directly, and hardcode its binary value in this test. That python command was:
     * # For sample_numbers:
     *      import numpy as np, io, binascii; b = io.BytesIO(); np.save(b, np.arange(15, dtype=np.int64)); b.seek(0); print(binascii.hexlify(b.read()))
     */
    std::string expected_sample_numbers_hex =
        "934e554d5059010076007b276465736372273a20273c6938272c2027666f727472616e5f6f72646572273a2046616c73652c2027736861"
        "7065273a202831352c292c207d202020202020202020202020202020202020202020202020202020202020202020202020202020202020"
        "20202020202020202020202020202020200a00000000000000000100000000000000020000000000000003000000000000000400000000"
        "000000050000000000000006000000000000000700000000000000080000000000000009000000000000000a000000000000000b000000"
        "000000000c000000000000000d000000000000000e00000000000000";
    CompareBinaryFilesHex("sample_numbers.npy", sample_numbers_bin, expected_sample_numbers_hex);

    success = false;
    std::vector<char> timestamps_bin;
    LoadNpyFileBinary("timestamps.npy", &timestamps_bin, &success);
    ASSERT_TRUE(success);

    /**
     * Same logic as above (note the timestamps are just converted from the sample numbers according to sampling rate)
     *      import numpy as np, io, binascii; b = io.BytesIO(); np.save(b, np.arange(15, dtype=np.double)); b.seek(0); print(binascii.hexlify(b.read()))
     */
    std::string expected_timestamps_hex =
        "934e554d5059010076007b276465736372273a20273c6638272c2027666f727472616e5f6f72646572273a2046616c73652c2027736861"
        "7065273a202831352c292c207d202020202020202020202020202020202020202020202020202020202020202020202020202020202020"
        "20202020202020202020202020202020200a0000000000000000000000000000f03f000000000000004000000000000008400000000000"
        "001040000000000000144000000000000018400000000000001c4000000000000020400000000000002240000000000000244000000000"
        "0000264000000000000028400000000000002a400000000000002c40";
    CompareBinaryFilesHex("timestamps.npy", timestamps_bin, expected_timestamps_hex);
}

TEST_F(RecordNodeTests, Test_PersistsStructureOeBin) {
    GTEST_SKIP();
}

TEST_F(RecordNodeTests, Test_PersistsEvents) {
    GTEST_SKIP();
}
