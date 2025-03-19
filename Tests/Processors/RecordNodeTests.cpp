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
        auto recordingDir = std::filesystem::directory_iterator(parentRecordingDir)->path();
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
        // The min value is actually -32767 in the math in RecordNode, not -32768 like the "true" min for int16_t
        return (std::numeric_limits<int16_t>::min)() + 1;
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
    auto recordingDir = std::filesystem::directory_iterator(parentRecordingDir)->path();
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
