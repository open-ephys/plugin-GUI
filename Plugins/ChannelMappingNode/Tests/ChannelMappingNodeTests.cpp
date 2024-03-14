#include <stdio.h>

#include "gtest/gtest.h"

#include "../ChannelMappingNode.h"
#include <ProcessorHeaders.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>
#include <filesystem>

class ChannelMappingNodeTests : public ::testing::Test {
protected:
    void SetUp() override {
        num_channels = 8;
        tester = std::make_unique<ProcessorTester>(FakeSourceNodeParams{
            num_channels,
            sample_rate_,
            1.0,
        });
        processor = tester->Create<ChannelMappingNode>(Plugin::Processor::FILTER);
        ASSERT_EQ(processor->getNumDataStreams(), 1);
        stream_id = processor->getDataStreams()[0]->getStreamId();

        prb_file_path_ = std::filesystem::temp_directory_path() / "prb_file.json";
    }

    void TearDown() override {
        if (std::filesystem::exists(prb_file_path_)) {
            std::filesystem::remove(prb_file_path_);
        }
    }

    ChannelMappingNode *processor;
    int num_channels;
    uint16 stream_id;
    std::unique_ptr<ProcessorTester> tester;
    float sample_rate_ = 30000.0;
    std::filesystem::path prb_file_path_;
};

TEST_F(ChannelMappingNodeTests, TestRemapsChannels) {
    // Make it backwards:
    juce::Array<int> new_channel_order;
    std::unordered_map<int, int> old_to_new_channel_mapping;
    for (int i = 0; i < num_channels; i++) {
        new_channel_order.add(num_channels - 1 - i);
        old_to_new_channel_mapping[i] = num_channels - 1 - i;
    }

    processor->setChannelOrder(stream_id, new_channel_order);
    processor->updateSettings();

    auto original_data_stream = tester->GetSourceNodeDataStream(stream_id);
    auto mapped_data_stream = tester->GetProcessorDataStream(processor->getNodeId(), stream_id);

    ASSERT_EQ(mapped_data_stream->getContinuousChannels().size(),
              original_data_stream->getContinuousChannels().size());
    for (std::pair<int, int> old_to_new : old_to_new_channel_mapping) {
        ASSERT_EQ(
            mapped_data_stream->getContinuousChannels()[old_to_new.second]->getGlobalIndex(),
            original_data_stream->getContinuousChannels()[old_to_new.first]->getGlobalIndex());
    }
}

TEST_F(ChannelMappingNodeTests, TestRespectsEnabledChannels) {
    int expected_mapped_num_channels = 4;
    // Make it backwards:
    juce::Array<int> new_channel_order;
    std::unordered_map<int, int> old_to_new_channel_mapping;
    for (int i = 0; i < num_channels; i++) {
        if (i < expected_mapped_num_channels) {
            // Enable channels explicitly
            processor->setChannelEnabled(stream_id, i, 1);
            old_to_new_channel_mapping[i] = i;
        } else {
            // Disable some channels
            processor->setChannelEnabled(stream_id, i, 0);
        }

        // Always map all channels, or else it'll be ignored
        new_channel_order.add(i);
    }

    processor->setChannelOrder(stream_id, new_channel_order);
    processor->updateSettings();

    auto original_data_stream = tester->GetSourceNodeDataStream(stream_id);
    auto mapped_data_stream = tester->GetProcessorDataStream(processor->getNodeId(), stream_id);

    ASSERT_EQ(original_data_stream->getContinuousChannels().size(), num_channels);
    ASSERT_EQ(mapped_data_stream->getContinuousChannels().size(), expected_mapped_num_channels);
    for (std::pair<int, int> old_to_new : old_to_new_channel_mapping) {
        ASSERT_EQ(
            mapped_data_stream->getContinuousChannels()[old_to_new.second]->getGlobalIndex(),
            original_data_stream->getContinuousChannels()[old_to_new.first]->getGlobalIndex());
    }
}

TEST_F(ChannelMappingNodeTests, ConfigFileTest) {
    std::vector<std::string> prb_file_contents_list = {
        "{",
        "  \"0\": {",
        "    \"mapping\": [",
        "      7,",
        "      6,",
        "      5,",
        "      4,",
        "      3,",
        "      2,",
        "      1,",
        "      0",
        "    ],",
        "    \"enabled\": [",
        "      true,",
        "      true,",
        "      true,",
        "      true,",
        "      true,",
        "      true,",
        "      true,",
        "      true",
        "    ]",
        "  }",
        "}"
    };

    std::stringstream prb_file_ss;
    for (const auto &line : prb_file_contents_list) {
        prb_file_ss << line << std::endl;
    }

    std::string prb_file_contents = prb_file_ss.str();
    auto f = juce::File(prb_file_path_.string());
    {
        // Write and close it via braces
        juce::FileOutputStream output_stream(f);
        output_stream.writeString(prb_file_contents);
    }

    processor->loadStreamSettings(stream_id, f);
    processor->updateSettings();

    auto original_data_stream = tester->GetSourceNodeDataStream(stream_id);
    auto mapped_data_stream = tester->GetProcessorDataStream(processor->getNodeId(), stream_id);

    ASSERT_EQ(original_data_stream->getContinuousChannels().size(), num_channels);
    ASSERT_EQ(
        mapped_data_stream->getContinuousChannels().size(),
        original_data_stream->getContinuousChannels().size());
    for (std::pair<int, int> old_to_new :
        std::unordered_map<int, int>({
                                         {0, 7},
                                         {1, 6},
                                         {2, 5},
                                         {3, 4},
                                         {4, 3},
                                         {5, 2},
                                         {6, 1},
                                         {7, 0},
                                     })) {
        ASSERT_EQ(
            mapped_data_stream->getContinuousChannels()[old_to_new.second]->getGlobalIndex(),
            original_data_stream->getContinuousChannels()[old_to_new.first]->getGlobalIndex());
    }
}

