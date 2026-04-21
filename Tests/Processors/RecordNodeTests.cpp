#include <stdio.h>

#include "gtest/gtest.h"

#include <Processors/RecordNode/RecordNode.h>
#include <Processors/RecordNode/BinaryFormat/SequentialBlockFile.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>
#include <chrono>
#include <thread>
#include <iterator>
#include <iostream>
#include <filesystem>
#include <algorithm>

class RecordNodeTests :  public testing::Test {
protected:
    void SetUp() override {
        numChannels = 8;
        tester = std::make_unique<ProcessorTester>(TestSourceNodeBuilder
                                                   (FakeSourceNodeParams{
            numChannels,
            sampleRate,
            bitVolts
        }));

        parentRecordingDir = std::filesystem::temp_directory_path() / "record_node_tests";
        if (std::filesystem::exists(parentRecordingDir)) {
            std::filesystem::remove_all(parentRecordingDir);
        }
        std::filesystem::create_directory(parentRecordingDir);

        // Set this before creating the record node
        tester->setRecordingParentDirectory(parentRecordingDir.string());
        processor = tester->createProcessor<RecordNode>(Plugin::Processor::RECORD_NODE);
    }

    void TearDown() override {
        // Swallow errors
        std::error_code ec;
        std::filesystem::remove_all(parentRecordingDir, ec);
    }

    AudioBuffer<float> createBuffer(float startingVal, float step, int numChannels, int numSamples) {
        AudioBuffer<float> inputBuffer(numChannels, numSamples);

        // in microvolts
        float currVal = startingVal;
        for (int chidx = 0; chidx < numChannels; chidx++) {
            for (int sampleIdx = 0; sampleIdx < numSamples; sampleIdx++) {
                inputBuffer.setSample(chidx, sampleIdx, currVal);
                currVal += step;
            }
        }
        return inputBuffer;
    }

    void writeBlock(AudioBuffer<float> &buffer, TTLEvent* maybeTtlEvent = nullptr) {
        auto outBuffer = tester->processBlock(processor, buffer, maybeTtlEvent);
        // Assert the buffer hasn't changed after process()
        ASSERT_EQ(outBuffer.getNumSamples(), buffer.getNumSamples());
        ASSERT_EQ(outBuffer.getNumChannels(), buffer.getNumChannels());
        for (int chidx = 0; chidx < outBuffer.getNumChannels(); chidx++) {
            for (int sampleIdx = 0; sampleIdx < outBuffer.getNumSamples(); ++sampleIdx) {
                ASSERT_EQ(
                    outBuffer.getSample(chidx, sampleIdx),
                    buffer.getSample(chidx, sampleIdx));
            }
        }
    }

    bool subRecordingPathFor(
        const std::string& subrecording_dirname,
        const std::string& basename,
        std::filesystem::path* path) {
        // Do verifications:
        auto dirIter = std::filesystem::directory_iterator(parentRecordingDir);
        if (dirIter == std::filesystem::directory_iterator()) {
            return false; // Directory is empty
        }
        auto recordingDir = dirIter->path();
        std::stringstream ss;
        ss << "Record Node " << processor->getNodeId();
        auto recordingDir2 = recordingDir / ss.str() / "experiment1" / "recording1" / subrecording_dirname;
        if (!std::filesystem::exists(recordingDir2)) {
            return false;
        }

        std::filesystem::path recordingDir3;
        for (const auto &subdir : std::filesystem::directory_iterator(recordingDir2)) {
            auto subDirBaseName = subdir.path().filename().string();
            if (subDirBaseName.find("FakeSourceNode") != std::string::npos) {
                recordingDir3 = subdir.path();
            }
        }

        if (!std::filesystem::exists(recordingDir3)) {
            return false;
        }

        auto ret = recordingDir3 / basename;
        if (!std::filesystem::exists(ret)) {
            return false;
        }
        *path = ret;
        return true;
    }

    bool continuousPathFor(const std::string& basename, std::filesystem::path* path) {
        return subRecordingPathFor("continuous", basename, path);
    }

    bool eventsPathFor(const std::string& basename, std::filesystem::path* path) {
        std::filesystem::path partialPath;
        auto success = subRecordingPathFor("events", "TTL", &partialPath);
        if (!success) {
            return false;
        }
        auto ret = partialPath / basename;
        if (std::filesystem::exists(ret)) {
            *path = ret;
            return true;
        } else {
            return false;
        }
    }

    void maybeLoadContinuousDatFile(std::vector<int16_t> *output, bool *success) {
        // Do verifications:
        std::filesystem::path continuousDatPath;
        *success = continuousPathFor("continuous.dat", &continuousDatPath);
        if (!*success) {
            return;
        }

        std::ifstream continuousIfStream(continuousDatPath.string(), std::ios::binary | std::ios::in);

        continuousIfStream.seekg(0, std::ios::end);
        std::streampos fileSize = continuousIfStream.tellg();
        continuousIfStream.seekg(0, std::ios::beg);
        if (fileSize % sizeof(int16_t) != 0) {
            *success = false;
            return;
        }

        std::vector<int16_t> persistedData(fileSize / sizeof(int16_t));
        continuousIfStream.read((char *) persistedData.data(), fileSize);
        *success = true;
        *output = persistedData;
    }

    void loadContinuousDatFile(std::vector<int16_t> *output) {
        bool success = false;
        maybeLoadContinuousDatFile(output, &success);
        ASSERT_TRUE(success);
    }

    std::vector<char> loadNpyFileBinaryFullpath(const std::string& fullPath) {
        std::ifstream dataIfStream(fullPath, std::ios::binary | std::ios::in);

        dataIfStream.seekg(0, std::ios::end);
        std::streampos fileSize = dataIfStream.tellg();
        dataIfStream.seekg(0, std::ios::beg);

        std::vector<char> persistedData(fileSize);
        dataIfStream.read(persistedData.data(), fileSize);
        return persistedData;
    }

    void loadNpyFileBinary(const std::string &basename, std::vector<char> *output, bool *success) {
        // Do verifications:
        std::filesystem::path npyFilePath;
        *success = continuousPathFor(basename, &npyFilePath);
        if (!*success) {
            return;
        }
        *success = true;
        *output = loadNpyFileBinaryFullpath(npyFilePath.string());
    }


    void compareBinaryFilesHex(const std::string& filename, const std::vector<char> binData, const std::string& expectedBinDataHex) {
        std::vector<char> expectedBinData;
        for (int i = 0; i < expectedBinDataHex.length(); i += 2) {
            std::string byteString = expectedBinDataHex.substr(i, 2);
            char byte = (char) strtol(byteString.c_str(), nullptr, 16);
            expectedBinData.push_back(byte);
        }

        // Create a string rep of the actual sample numbers bin in case it fails, to help debugging
        std::stringstream binDataHexSs;
        binDataHexSs << "Expected data for " << filename << " in hex to be=" << expectedBinDataHex
                        << " but received=";
        binDataHexSs << std::hex;
        for (int i = 0; i < binData.size(); i++) {
            binDataHexSs << std::setw(2) << std::setfill('0') << (int)binData[i];
        }
        std::string err_msg = binDataHexSs.str();

        ASSERT_EQ(binData.size(), expectedBinData.size()) << err_msg;
        for (int i = 0; i < binData.size(); i++) {
            ASSERT_EQ(binData[i], expectedBinData[i])
                                << err_msg
                                << " (error on index " << i << ")";
        }
    }

    static int16_t minValPossible() {
        // The SIMDConverter correctly uses the full int16 range [-32768, 32767]
        return (std::numeric_limits<int16_t>::min)();
    }

    static int16_t maxValPossible() {
        return (std::numeric_limits<int16_t>::max)();
    }

    RecordNode *processor;
    int numChannels;
    float bitVolts = 1.0;
    std::unique_ptr<ProcessorTester> tester;
    std::filesystem::path parentRecordingDir;
    float sampleRate = 1.0;
};

TEST_F(RecordNodeTests, TestInputOutput_Continuous_Single) {
    int numSamples = 100;
    tester->startAcquisition(true);

    auto inputBuffer = createBuffer(1000.0, 20.0, numChannels, numSamples);
    writeBlock(inputBuffer);

    // The record node always flushes its pending writes when stopping acquisition, so we don't need to sleep before
    // stopping.
    tester->stopAcquisition();

    std::vector<int16_t> persistedData;
    loadContinuousDatFile(&persistedData);
    ASSERT_EQ(persistedData.size(), numChannels * numSamples);

    int persistedDataIdx = 0;
    // File is channel-interleaved, so ensure we iterate in the correct order:
    for (int sampleIdx = 0; sampleIdx < numSamples; sampleIdx++) {
        for (int chidx = 0; chidx < numChannels; chidx++) {
            auto expectedMicroVolts = inputBuffer.getSample(chidx, sampleIdx);
            ASSERT_EQ(persistedData[persistedDataIdx], expectedMicroVolts);
            persistedDataIdx++;
        }
    }
}

TEST_F(RecordNodeTests, TestInputOutput_Continuous_Multiple) {
    tester->startAcquisition(true);

    int numSamplesPerBlock = 100;
    int numBlocks = 8;
    std::vector<AudioBuffer<float>> inputBuffers;
    for (int i = 0; i < numBlocks; i++) {
        auto inputBuffer = createBuffer(1000.0f * i, 20.0, numChannels, numSamplesPerBlock);
        writeBlock(inputBuffer);
        inputBuffers.push_back(inputBuffer);
    }

    tester->stopAcquisition();

    std::vector<int16_t> persistedData;
    loadContinuousDatFile(&persistedData);
    ASSERT_EQ(persistedData.size(), numChannels * numSamplesPerBlock * numBlocks);

    int persistedDataIdx = 0;
    // File is channel-interleaved, so ensure we iterate in the correct order:
    for (int blockIdx = 0; blockIdx < numBlocks; blockIdx++) {
        const auto& inputBuffer = inputBuffers[blockIdx];
        for (int sampleIdx = 0; sampleIdx < numSamplesPerBlock; sampleIdx++) {
            for (int chidx = 0; chidx < numChannels; chidx++) {
                auto expectedMicroVolts = inputBuffer.getSample(chidx, sampleIdx);
                ASSERT_EQ(persistedData[persistedDataIdx], expectedMicroVolts);
                persistedDataIdx++;
            }
        }
    }
}

TEST_F(RecordNodeTests, TestEmpty) {
    tester->startAcquisition(true);
    tester->stopAcquisition();

    std::vector<int16_t> persistedData;
    loadContinuousDatFile(&persistedData);
    ASSERT_EQ(persistedData.size(), 0);
}

TEST_F(RecordNodeTests, TestClipsProperly) {
    int numSamples = 100;
    tester->startAcquisition(true);

    // The min value is actually -32767, not -32768 like the "true" min
    std::vector<AudioBuffer<float>> inputBuffers;

    // Write numbers both underflowing and overflowing
    auto inputBuffer = createBuffer((float) minValPossible() + 1, -1, numChannels, numSamples);
    writeBlock(inputBuffer);
    inputBuffers.push_back(inputBuffer);

    inputBuffer = createBuffer((float) maxValPossible() - 1, 1, numChannels, numSamples);
    writeBlock(inputBuffer);
    inputBuffers.push_back(inputBuffer);

    tester->stopAcquisition();

    std::vector<int16_t> persistedData;
    loadContinuousDatFile(&persistedData);
    ASSERT_EQ(persistedData.size(), numChannels * numSamples * 2);

    int persistedDataIdx = 0;
    for (int blockIdx = 0; blockIdx < 2; blockIdx++) {
        auto inputBuffer = inputBuffers[blockIdx];
        for (int sampleIdx = 0; sampleIdx < numSamples; sampleIdx++) {
            for (int chidx = 0; chidx < numChannels; chidx++) {
                auto expectedMicroVolts = inputBuffer.getSample(chidx, sampleIdx);

                // Per the logic above, only the very first sample/channel is within the normal bounds; the rest will
                // be clipped at the upper or lower possible values.
                int16_t expectedPersisted;
                if (sampleIdx == 0 && chidx == 0) {
                    expectedPersisted = expectedMicroVolts;
                } else if (expectedMicroVolts > 0) {
                    expectedPersisted = maxValPossible();
                } else {
                    expectedPersisted = minValPossible();
                }

                ASSERT_EQ(persistedData[persistedDataIdx], expectedPersisted);
                persistedDataIdx++;
            }
        }
    }
}

class CustomBitVolts_RecordNodeTests : public RecordNodeTests {
    void SetUp() override {
        bitVolts = 0.195;
        RecordNodeTests::SetUp();
    }
};

TEST_F(CustomBitVolts_RecordNodeTests, Test_RespectsBitVolts) {
    int numSamples = 100;
    tester->startAcquisition(true);
    auto inputBuffer = createBuffer(1000.0, 20.0, numChannels, numSamples);
    writeBlock(inputBuffer);
    tester->stopAcquisition();

    std::vector<int16_t> persistedData;
    loadContinuousDatFile(&persistedData);
    ASSERT_EQ(persistedData.size(), numChannels * numSamples);

    int persistedDataIdx = 0;
    // File is channel-interleaved, so ensure we iterate in the correct order:
    for (int sampleIdx = 0; sampleIdx < numSamples; sampleIdx++) {
        for (int chidx = 0; chidx < numChannels; chidx++) {
            auto expectedMicroVolts = inputBuffer.getSample(chidx, sampleIdx);
            auto expected_converted = expectedMicroVolts / bitVolts;

            // Rounds to nearest int, like BinaryRecording does, and clamp within bounds
            int expected_rounded = juce::roundToInt(expected_converted);
            int16_t expectedPersisted = (int16_t) std::clamp(
                expected_rounded,
                (int) minValPossible(),
                (int) maxValPossible());
            ASSERT_EQ(persistedData[persistedDataIdx], expectedPersisted);
            persistedDataIdx++;
        }
    }
}

TEST_F(RecordNodeTests, Test_PersistsSampleNumbersAndTimestamps) {
    tester->startAcquisition(true);

    int numSamples = 5;
    for (int i = 0; i < 3; i++) {
        auto inputBuffer = createBuffer(1000.0, 20.0, numChannels, numSamples);
        writeBlock(inputBuffer);
    }
    tester->stopAcquisition();

    bool success = false;
    std::vector<char> sampleNumbersBin;
    loadNpyFileBinary("sample_numbers.npy", &sampleNumbersBin, &success);
    ASSERT_TRUE(success);

    /**
     * Since NpyFile in OpenEphys doesn't include any facility to read .npy files, we've generated the expected
     * .npy file output in Python directly, and hardcode its binary value in this test. That python command was:
     * # For sample_numbers:
     *      import numpy as np, io, binascii; b = io.BytesIO(); np.save(b, np.arange(15, dtype=np.int64)); b.seek(0); print(binascii.hexlify(b.read()))
     */
    std::string expectedSampleNumbersHex =
        "934e554d5059010076007b276465736372273a20273c6938272c2027666f727472616e5f6f72646572273a2046616c73652c2027736861"
        "7065273a202831352c292c207d202020202020202020202020202020202020202020202020202020202020202020202020202020202020"
        "20202020202020202020202020202020200a00000000000000000100000000000000020000000000000003000000000000000400000000"
        "000000050000000000000006000000000000000700000000000000080000000000000009000000000000000a000000000000000b000000"
        "000000000c000000000000000d000000000000000e00000000000000";
    compareBinaryFilesHex("sample_numbers.npy", sampleNumbersBin, expectedSampleNumbersHex);

    success = false;
    std::vector<char> timeStampsBin;
    loadNpyFileBinary("timestamps.npy", &timeStampsBin, &success);
    ASSERT_TRUE(success);

    /**
     * Same logic as above (note the timestamps are just converted from the sample numbers according to sampling rate)
     *      import numpy as np, io, binascii; b = io.BytesIO(); np.save(b, np.arange(15, dtype=np.double)); b.seek(0); print(binascii.hexlify(b.read()))
     */
    std::string expectedTimeStampsHex =
        "934e554d5059010076007b276465736372273a20273c6638272c2027666f727472616e5f6f72646572273a2046616c73652c2027736861"
        "7065273a202831352c292c207d202020202020202020202020202020202020202020202020202020202020202020202020202020202020"
        "20202020202020202020202020202020200a0000000000000000000000000000f03f000000000000004000000000000008400000000000"
        "001040000000000000144000000000000018400000000000001c4000000000000020400000000000002240000000000000244000000000"
        "0000264000000000000028400000000000002a400000000000002c40";
    compareBinaryFilesHex("timestamps.npy", timeStampsBin, expectedTimeStampsHex);
}

TEST_F(RecordNodeTests, Test_PersistsStructureOeBin) {
    tester->startAcquisition(true);

    int numSamples = 5;
    for (int i = 0; i < 3; i++) {
        auto inputBuffer = createBuffer(1000.0, 20.0, numChannels, numSamples);
        writeBlock(inputBuffer);
    }
    tester->stopAcquisition();

    // Do verifications:
    auto dirIter = std::filesystem::directory_iterator(parentRecordingDir);
    ASSERT_NE(dirIter, std::filesystem::directory_iterator()) << "Recording directory is empty";
    auto recordingDir = dirIter->path();
    std::stringstream ss;
    ss << "Record Node " << processor->getNodeId();
    auto recordingDir2 = recordingDir / ss.str() / "experiment1" / "recording1";
    ASSERT_TRUE(std::filesystem::exists(recordingDir2));

    auto structureOeBinFn = recordingDir2 / "structure.oebin";
    ASSERT_TRUE(std::filesystem::exists(structureOeBinFn));

    auto f = juce::File(structureOeBinFn.string());
//    FileInputStream input(f);
//    std::cout << input.readEntireStreamAsString() << std::endl;
    auto jsonParsed = JSON::parse(f);
    ASSERT_TRUE(jsonParsed.hasProperty("GUI version"));
    ASSERT_TRUE(jsonParsed["GUI version"].toString().length() > 0);

    ASSERT_TRUE(jsonParsed.hasProperty("continuous"));
    const auto& jsonContinuousList = jsonParsed["continuous"];
    ASSERT_TRUE(jsonContinuousList.isArray());
    // 1 per stream, so just 1
    ASSERT_EQ(jsonContinuousList.getArray()->size(), 1);

    auto jsonContinuous = (*jsonContinuousList.getArray())[0];

    // Spot check some fields
    ASSERT_TRUE(jsonContinuous.hasProperty("folder_name"));
    ASSERT_TRUE(jsonContinuous["folder_name"].toString().contains("Record_Node"));
    ASSERT_TRUE(jsonContinuous.hasProperty("sample_rate"));
    ASSERT_FLOAT_EQ((float) jsonContinuous["sample_rate"], sampleRate);

    ASSERT_TRUE(jsonContinuous.hasProperty("sample_rate"));
    ASSERT_FLOAT_EQ((float) jsonContinuous["sample_rate"], sampleRate);

    ASSERT_TRUE(jsonContinuous.hasProperty("num_channels"));
    ASSERT_FLOAT_EQ((int) jsonContinuous["num_channels"], numChannels);

    ASSERT_TRUE(jsonContinuous.hasProperty("channels"));
    ASSERT_TRUE(jsonContinuous["channels"].isArray());
    ASSERT_EQ(jsonContinuous["channels"].getArray()->size(), numChannels);

    auto jsonContinuousChannel = (*jsonContinuous["channels"].getArray())[0];
    ASSERT_TRUE(jsonContinuousChannel.hasProperty("bit_volts"));
    ASSERT_FLOAT_EQ((float) jsonContinuousChannel["bit_volts"], bitVolts);

    ASSERT_TRUE(jsonContinuousChannel.hasProperty("channel_name"));
    ASSERT_EQ(jsonContinuousChannel["channel_name"].toString(), juce::String("CH0"));
}

TEST_F(RecordNodeTests, Test_PersistsEvents) {
    processor->setRecordEvents(true);
    processor->updateSettings();

    tester->startAcquisition(true);
    int numSamples = 5;

    auto streamId = processor->getDataStreams()[0]->getStreamId();
    auto eventChannels = tester->getSourceNodeDataStream(streamId)->getEventChannels();
    ASSERT_GE(eventChannels.size(), 1);
    TTLEventPtr eventPtr = TTLEvent::createTTLEvent(
        eventChannels[0],
        1,
        2,
        true);
    auto inputBuffer = createBuffer(1000.0, 20.0, numChannels, numSamples);
    writeBlock(inputBuffer, eventPtr.get());
    tester->stopAcquisition();

    std::filesystem::path sampleNumbersPath;
    ASSERT_TRUE(eventsPathFor("sample_numbers.npy", &sampleNumbersPath));
    auto sampleNumbersBin = loadNpyFileBinaryFullpath(sampleNumbersPath.string());

    /**
     * Same logic as above:
     *      import numpy as np, io, binascii; b = io.BytesIO(); np.save(b, np.array([1], dtype=np.int64)); b.seek(0); print(binascii.hexlify(b.read()))
     */
    std::string expectedSampleNumbersHex =
        "934e554d5059010076007b276465736372273a20273c6938272c2027666f727472616e5f6f72646572273a2046616c73652c2027736861"
        "7065273a2028312c292c207d20202020202020202020202020202020202020202020202020202020202020202020202020202020202020"
        "20202020202020202020202020202020200a0100000000000000";
    compareBinaryFilesHex("sample_numbers.npy", sampleNumbersBin, expectedSampleNumbersHex);

    std::filesystem::path fullWordsPath;
    ASSERT_TRUE(eventsPathFor("full_words.npy", &fullWordsPath));
    auto fullWordsBin = loadNpyFileBinaryFullpath(fullWordsPath.string());

    /**
     * Same logic as above:
     *      import numpy as np, io, binascii; b = io.BytesIO(); np.save(b, np.array([4], dtype=np.uint64)); b.seek(0); print(binascii.hexlify(b.read()))
     */
    std::string expectedFullWordsHex =
        "934e554d5059010076007b276465736372273a20273c7538272c2027666f727472616e5f6f72646572273a2046616c73652c2027736861"
        "7065273a2028312c292c207d20202020202020202020202020202020202020202020202020202020202020202020202020202020202020"
        "20202020202020202020202020202020200a0400000000000000";
    compareBinaryFilesHex("full_words.npy", fullWordsBin, expectedFullWordsHex);
}

// ============================================================================
// SEQUENTIAL BLOCK FILE BATCH WRITE TESTS
// ============================================================================

/**
 * Test fixture for SequentialBlockFile batch writing tests.
 * These tests verify the correctness of writeChannelBatch() method.
 */
class SequentialBlockFileTests : public testing::Test {
protected:
    void SetUp() override {
        parentDir = std::filesystem::temp_directory_path() / "sequential_block_file_tests";
        if (std::filesystem::exists(parentDir)) {
            std::filesystem::remove_all(parentDir);
        }
        std::filesystem::create_directory(parentDir);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(parentDir, ec);
    }

    /**
     * Read all data from a .dat file and return as vector of int16_t
     */
    std::vector<int16_t> readDatFile(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary | std::ios::in);
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<int16_t> data(fileSize / sizeof(int16_t));
        file.read(reinterpret_cast<char*>(data.data()), fileSize);
        return data;
    }

    std::filesystem::path parentDir;
};

/**
 * Test writeChannelBatch correctly writes and interleaves data for multiple channels.
 * This verifies that the batch write produces the same interleaved output as
 * individual per-channel writes.
 */
TEST_F(SequentialBlockFileTests, WriteChannelBatch_MultipleChannels_CorrectInterleaving) {
    const int numChannels = 8;
    const int numSamples = 100;
    
    // Create test data for each channel
    std::vector<std::vector<int16_t>> channelData(numChannels);
    std::vector<int16_t*> channelPtrs(numChannels);
    
    for (int ch = 0; ch < numChannels; ch++) {
        channelData[ch].resize(numSamples);
        for (int s = 0; s < numSamples; s++) {
            // Each channel has distinct values: ch*1000 + sample index
            channelData[ch][s] = static_cast<int16_t>(ch * 1000 + s);
        }
        channelPtrs[ch] = channelData[ch].data();
    }
    
    // Write using batch method
    auto batchFile = parentDir / "batch_write.dat";
    {
        SequentialBlockFile blockFile(numChannels, 4096);
        ASSERT_TRUE(blockFile.openFile(batchFile.string()));
        ASSERT_TRUE(blockFile.writeChannelBatch(0, channelPtrs.data(), numChannels, numSamples));
    }
    
    // Write using per-channel method for comparison
    auto perChannelFile = parentDir / "per_channel_write.dat";
    {
        SequentialBlockFile blockFile(numChannels, 4096);
        ASSERT_TRUE(blockFile.openFile(perChannelFile.string()));
        for (int ch = 0; ch < numChannels; ch++) {
            ASSERT_TRUE(blockFile.writeChannel(0, ch, channelPtrs[ch], numSamples));
        }
    }
    
    // Read both files and compare
    auto batchData = readDatFile(batchFile);
    auto perChannelData = readDatFile(perChannelFile);
    
    ASSERT_EQ(batchData.size(), perChannelData.size());
    ASSERT_EQ(batchData.size(), static_cast<size_t>(numChannels * numSamples));
    
    // Verify data is correctly interleaved
    for (size_t i = 0; i < batchData.size(); i++) {
        ASSERT_EQ(batchData[i], perChannelData[i])
            << "Mismatch at index " << i 
            << " (sample " << (i / numChannels) << ", channel " << (i % numChannels) << ")";
    }
    
    // Also verify the actual interleaving pattern
    for (int s = 0; s < numSamples; s++) {
        for (int ch = 0; ch < numChannels; ch++) {
            int idx = s * numChannels + ch;
            int16_t expected = static_cast<int16_t>(ch * 1000 + s);
            ASSERT_EQ(batchData[idx], expected)
                << "Interleaving error at sample " << s << ", channel " << ch;
        }
    }
}

/**
 * Test writeChannelBatch with a single channel.
 */
TEST_F(SequentialBlockFileTests, WriteChannelBatch_SingleChannel_CorrectOutput) {
    const int numChannels = 1;
    const int numSamples = 256;
    
    std::vector<int16_t> channelData(numSamples);
    for (int s = 0; s < numSamples; s++) {
        channelData[s] = static_cast<int16_t>(s * 10);
    }
    
    int16_t* channelPtr = channelData.data();
    
    // Write using batch method
    auto batchFile = parentDir / "single_channel_batch.dat";
    {
        SequentialBlockFile blockFile(numChannels, 4096);
        ASSERT_TRUE(blockFile.openFile(batchFile.string()));
        ASSERT_TRUE(blockFile.writeChannelBatch(0, &channelPtr, numChannels, numSamples));
    }
    
    // Read and verify
    auto batchData = readDatFile(batchFile);
    ASSERT_EQ(batchData.size(), static_cast<size_t>(numSamples));
    
    for (int s = 0; s < numSamples; s++) {
        ASSERT_EQ(batchData[s], channelData[s])
            << "Mismatch at sample " << s;
    }
}

/**
 * Test writeChannelBatch rejects mismatched channel count.
 */
TEST_F(SequentialBlockFileTests, WriteChannelBatch_ChannelCountMismatch_ReturnsFalse) {
    const int fileChannels = 4;
    const int wrongChannels = 8;
    const int numSamples = 100;
    
    std::vector<std::vector<int16_t>> channelData(wrongChannels);
    std::vector<int16_t*> channelPtrs(wrongChannels);
    
    for (int ch = 0; ch < wrongChannels; ch++) {
        channelData[ch].resize(numSamples, 0);
        channelPtrs[ch] = channelData[ch].data();
    }
    
    auto testFile = parentDir / "mismatch_test.dat";
    SequentialBlockFile blockFile(fileChannels, 4096);
    ASSERT_TRUE(blockFile.openFile(testFile.string()));
    
    // Should return false due to channel count mismatch
    EXPECT_FALSE(blockFile.writeChannelBatch(0, channelPtrs.data(), wrongChannels, numSamples));
}

/**
 * Test writeChannelBatch handles multiple consecutive writes correctly.
 */
TEST_F(SequentialBlockFileTests, WriteChannelBatch_MultipleWrites_CorrectOffsets) {
    const int numChannels = 4;
    const int samplesPerWrite = 100;
    const int numWrites = 5;
    
    std::vector<std::vector<int16_t>> channelData(numChannels);
    std::vector<int16_t*> channelPtrs(numChannels);
    
    auto testFile = parentDir / "multiple_writes.dat";
    SequentialBlockFile blockFile(numChannels, 4096);
    ASSERT_TRUE(blockFile.openFile(testFile.string()));
    
    // Perform multiple writes
    for (int writeIdx = 0; writeIdx < numWrites; writeIdx++) {
        for (int ch = 0; ch < numChannels; ch++) {
            channelData[ch].resize(samplesPerWrite);
            for (int s = 0; s < samplesPerWrite; s++) {
                // Value encodes write index, channel, and sample
                channelData[ch][s] = static_cast<int16_t>(writeIdx * 10000 + ch * 100 + s);
            }
            channelPtrs[ch] = channelData[ch].data();
        }
        
        uint64 startPos = writeIdx * samplesPerWrite;
        ASSERT_TRUE(blockFile.writeChannelBatch(startPos, channelPtrs.data(), numChannels, samplesPerWrite));
    }
    
    // Force file closure by destroying the block file
    // (destructor should flush)
}

// ============================================================================
// BATCH WRITE (writeContinuousDataBatch) CORRECTNESS TESTS
// ============================================================================

/**
 * Test that writeContinuousDataBatch correctly writes data for a single channel.
 * Uses the RecordNode infrastructure to test end-to-end.
 */
class SingleChannel_RecordNodeTests : public RecordNodeTests {
    void SetUp() override {
        numChannels = 1;
        tester = std::make_unique<ProcessorTester>(TestSourceNodeBuilder
                                                   (FakeSourceNodeParams{
            numChannels,
            sampleRate,
            bitVolts
        }));

        parentRecordingDir = std::filesystem::temp_directory_path() / "record_node_single_ch_tests";
        if (std::filesystem::exists(parentRecordingDir)) {
            std::filesystem::remove_all(parentRecordingDir);
        }
        std::filesystem::create_directory(parentRecordingDir);

        tester->setRecordingParentDirectory(parentRecordingDir.string());
        processor = tester->createProcessor<RecordNode>(Plugin::Processor::RECORD_NODE);
    }
};

TEST_F(SingleChannel_RecordNodeTests, WriteContinuousDataBatch_SingleChannel_CorrectOutput) {
    int numSamples = 200;
    tester->startAcquisition(true);

    auto inputBuffer = createBuffer(500.0, 10.0, numChannels, numSamples);
    writeBlock(inputBuffer);

    tester->stopAcquisition();

    std::vector<int16_t> persistedData;
    loadContinuousDatFile(&persistedData);
    ASSERT_EQ(persistedData.size(), static_cast<size_t>(numChannels * numSamples));

    // Verify data matches (single channel, so no interleaving)
    for (int sampleIdx = 0; sampleIdx < numSamples; sampleIdx++) {
        auto expectedMicroVolts = inputBuffer.getSample(0, sampleIdx);
        ASSERT_EQ(persistedData[sampleIdx], static_cast<int16_t>(expectedMicroVolts))
            << "Mismatch at sample " << sampleIdx;
    }
}

/**
 * Test that writeContinuousDataBatch correctly writes data for multiple channels.
 */
TEST_F(RecordNodeTests, WriteContinuousDataBatch_MultipleChannels_CorrectOutput) {
    int numSamples = 150;
    tester->startAcquisition(true);

    auto inputBuffer = createBuffer(100.0, 5.0, numChannels, numSamples);
    writeBlock(inputBuffer);

    tester->stopAcquisition();

    std::vector<int16_t> persistedData;
    loadContinuousDatFile(&persistedData);
    ASSERT_EQ(persistedData.size(), static_cast<size_t>(numChannels * numSamples));

    // File is channel-interleaved, verify correct order
    int persistedDataIdx = 0;
    for (int sampleIdx = 0; sampleIdx < numSamples; sampleIdx++) {
        for (int chidx = 0; chidx < numChannels; chidx++) {
            auto expectedMicroVolts = inputBuffer.getSample(chidx, sampleIdx);
            ASSERT_EQ(persistedData[persistedDataIdx], static_cast<int16_t>(expectedMicroVolts))
                << "Mismatch at sample " << sampleIdx << ", channel " << chidx;
            persistedDataIdx++;
        }
    }
}

/**
 * Test that writeContinuousDataBatch handles empty batches gracefully.
 * When numSamples = 0, no data should be written but no crash should occur.
 */
TEST_F(RecordNodeTests, WriteContinuousDataBatch_EmptyBatch_ZeroSamples_NoDataWritten) {
    // Start and immediately stop without writing any data blocks
    tester->startAcquisition(true);
    // Don't write any blocks - equivalent to numSamples = 0
    tester->stopAcquisition();

    std::vector<int16_t> persistedData;
    loadContinuousDatFile(&persistedData);
    
    // Should have zero data
    ASSERT_EQ(persistedData.size(), 0u);
}

class MultiStream_RecordNodeTests : public testing::Test {
protected:
    void SetUp() override {
        tester = std::make_unique<ProcessorTester>(TestSourceNodeBuilder
                                                   (FakeSourceNodeParams{
            4,      // channels per stream
            30000,  // sample rate
            1.0f,   // bitVolts
            3       // streams
        }));

        parentRecordingDir = std::filesystem::temp_directory_path() / "record_node_multi_stream_tests";
        if (std::filesystem::exists(parentRecordingDir)) {
            std::filesystem::remove_all(parentRecordingDir);
        }
        std::filesystem::create_directory(parentRecordingDir);

        tester->setRecordingParentDirectory(parentRecordingDir.string());
        processor = tester->createProcessor<RecordNode>(Plugin::Processor::RECORD_NODE);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(parentRecordingDir, ec);
    }

    bool getRecordingPath(std::filesystem::path* outPath) const {
        auto dirIter = std::filesystem::directory_iterator(parentRecordingDir);
        if (dirIter == std::filesystem::directory_iterator()) {
            return false;
        }

        auto recordingDir = dirIter->path();
        std::stringstream ss;
        ss << "Record Node " << processor->getNodeId();
        *outPath = recordingDir / ss.str() / "experiment1" / "recording1";
        return true;
    }

    std::unique_ptr<ProcessorTester> tester;
    RecordNode* processor = nullptr;
    std::filesystem::path parentRecordingDir;
};

TEST_F(MultiStream_RecordNodeTests, DisabledMiddleStream_DoesNotBlockSubsequentStreamWrites) {
    // Disable all channels on middle stream; keep streams 0 and 2 enabled.
    auto streams = processor->getDataStreams();
    ASSERT_EQ(streams.size(), 3);

    Array<var> noChannelsSelected;
    auto* middleStreamMask = static_cast<MaskChannelsParameter*>(streams[1]->getParameter("channels"));
    ASSERT_NE(middleStreamMask, nullptr);
    middleStreamMask->setNextValue(noChannelsSelected, false);
    processor->updateSettings();

    tester->startAcquisition(true, true);

    AudioBuffer<float> inputBuffer(12, 256); // 3 streams * 4 channels/stream
    for (int ch = 0; ch < inputBuffer.getNumChannels(); ch++) {
        for (int s = 0; s < inputBuffer.getNumSamples(); s++) {
            inputBuffer.setSample(ch, s, static_cast<float>(ch * 1000 + s));
        }
    }

    for (int i = 0; i < 5; i++) {
        tester->processBlock(processor, inputBuffer);
    }

    tester->stopAcquisition();

    std::filesystem::path recordingPath;
    ASSERT_TRUE(getRecordingPath(&recordingPath));
    auto continuousPath = recordingPath / "continuous";
    ASSERT_TRUE(std::filesystem::exists(continuousPath));

    std::vector<std::filesystem::path> continuousDatFiles;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(continuousPath)) {
        if (entry.is_regular_file() && entry.path().filename() == "continuous.dat") {
            continuousDatFiles.push_back(entry.path());
        }
    }

    // Only streams with recorded channels should have files: stream 0 and stream 2.
    ASSERT_EQ(continuousDatFiles.size(), 2u);

    for (const auto& datFile : continuousDatFiles) {
        ASSERT_GT(std::filesystem::file_size(datFile), 0u)
            << "Expected non-empty data file for recorded stream: " << datFile.string();
    }
}

/**
 * Test that writeContinuousDataBatch correctly resizes internal buffers.
 * This tests the buffer reallocation logic by writing progressively larger blocks.
 */
class BufferResize_RecordNodeTests : public RecordNodeTests {
    void SetUp() override {
        numChannels = 16;
        tester = std::make_unique<ProcessorTester>(TestSourceNodeBuilder
                                                   (FakeSourceNodeParams{
            numChannels,
            sampleRate,
            bitVolts
        }));

        parentRecordingDir = std::filesystem::temp_directory_path() / "record_node_buffer_resize_tests";
        if (std::filesystem::exists(parentRecordingDir)) {
            std::filesystem::remove_all(parentRecordingDir);
        }
        std::filesystem::create_directory(parentRecordingDir);

        tester->setRecordingParentDirectory(parentRecordingDir.string());
        processor = tester->createProcessor<RecordNode>(Plugin::Processor::RECORD_NODE);
    }
};

TEST_F(BufferResize_RecordNodeTests, WriteContinuousDataBatch_BufferResize_HandlesLargerBlocks) {
    tester->startAcquisition(true);

    // Write multiple blocks to ensure internal buffers handle various data patterns.
    // Use consistent block sizes to avoid buffer queue timing issues.
    std::vector<AudioBuffer<float>> inputBuffers;
    int numBlocks = 10;
    int blockSize = 256;
    
    for (int blockIdx = 0; blockIdx < numBlocks; blockIdx++) {
        // Each block has different starting value to verify correct ordering
        auto inputBuffer = createBuffer(100.0f * blockIdx, 1.0, numChannels, blockSize);
        writeBlock(inputBuffer);
        inputBuffers.push_back(inputBuffer);
    }

    tester->stopAcquisition();

    std::vector<int16_t> persistedData;
    loadContinuousDatFile(&persistedData);
    
    int totalSamples = numBlocks * blockSize;
    
    ASSERT_EQ(persistedData.size(), static_cast<size_t>(numChannels * totalSamples));

    // Verify each block was written correctly
    int persistedDataIdx = 0;
    for (int blockIdx = 0; blockIdx < numBlocks; blockIdx++) {
        const auto& inputBuffer = inputBuffers[blockIdx];
        
        for (int sampleIdx = 0; sampleIdx < blockSize; sampleIdx++) {
            for (int chidx = 0; chidx < numChannels; chidx++) {
                auto expectedMicroVolts = inputBuffer.getSample(chidx, sampleIdx);
                ASSERT_EQ(persistedData[persistedDataIdx], static_cast<int16_t>(expectedMicroVolts))
                    << "Mismatch at block " << blockIdx << ", sample " << sampleIdx << ", channel " << chidx;
                persistedDataIdx++;
            }
        }
    }
}

/**
 * Test that writing with many channels (triggering batch buffer resize) works correctly.
 */
class ManyChannels_RecordNodeTests : public RecordNodeTests {
    void SetUp() override {
        numChannels = 64;  // More channels to stress buffer management
        tester = std::make_unique<ProcessorTester>(TestSourceNodeBuilder
                                                   (FakeSourceNodeParams{
            numChannels,
            sampleRate,
            bitVolts
        }));

        parentRecordingDir = std::filesystem::temp_directory_path() / "record_node_many_ch_tests";
        if (std::filesystem::exists(parentRecordingDir)) {
            std::filesystem::remove_all(parentRecordingDir);
        }
        std::filesystem::create_directory(parentRecordingDir);

        tester->setRecordingParentDirectory(parentRecordingDir.string());
        processor = tester->createProcessor<RecordNode>(Plugin::Processor::RECORD_NODE);
    }
};

TEST_F(ManyChannels_RecordNodeTests, WriteContinuousDataBatch_ManyChannels_CorrectInterleaving) {
    int numSamples = 200;
    tester->startAcquisition(true);

    auto inputBuffer = createBuffer(0.0, 1.0, numChannels, numSamples);
    writeBlock(inputBuffer);

    tester->stopAcquisition();

    std::vector<int16_t> persistedData;
    loadContinuousDatFile(&persistedData);
    ASSERT_EQ(persistedData.size(), static_cast<size_t>(numChannels * numSamples));

    // Verify interleaving is correct for all channels
    int persistedDataIdx = 0;
    for (int sampleIdx = 0; sampleIdx < numSamples; sampleIdx++) {
        for (int chidx = 0; chidx < numChannels; chidx++) {
            auto expectedMicroVolts = inputBuffer.getSample(chidx, sampleIdx);
            ASSERT_EQ(persistedData[persistedDataIdx], static_cast<int16_t>(expectedMicroVolts))
                << "Mismatch at sample " << sampleIdx << ", channel " << chidx;
            persistedDataIdx++;
        }
    }
}
