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

#include <stdio.h>

#include "gtest/gtest.h"

#include "../ChannelMap.h"
#include <ModelApplication.h>
#include <ModelProcessors.h>
#include <ProcessorHeaders.h>
#include <TestFixtures.h>
#include <filesystem>

class ChannelMapTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        numChannels = 8;
        tester = std::make_unique<ProcessorTester> (TestSourceNodeBuilder (FakeSourceNodeParams {
            numChannels,
            sampleRate,
            1.0,
        }));
        processor = tester->createProcessor<ChannelMap> (Plugin::Processor::FILTER);
        ASSERT_EQ (processor->getNumDataStreams(), 1);
        streamId = processor->getDataStreams()[0]->getStreamId();

        prbFilePath = std::filesystem::temp_directory_path() / "prb_file.json";
    }

    void TearDown() override
    {
        if (std::filesystem::exists (prbFilePath))
        {
            std::filesystem::remove (prbFilePath);
        }
    }

    ChannelMap* processor;
    int numChannels;
    uint16 streamId;
    std::unique_ptr<ProcessorTester> tester;
    float sampleRate = 30000.0;
    std::filesystem::path prbFilePath;
};

TEST_F (ChannelMapTests, TestRemapsChannels)
{
    // Make it backwards:
    juce::Array<int> newChanOrder;
    std::unordered_map<int, int> oldToNewChanMapping;
    for (int i = 0; i < numChannels; i++)
    {
        newChanOrder.add (numChannels - 1 - i);
        oldToNewChanMapping[i] = numChannels - 1 - i;
    }

    processor->setChannelOrder (streamId, newChanOrder);
    processor->updateSettings();

    auto originalDataStream = tester->getSourceNodeDataStream (streamId);
    auto mappedDataStream = tester->getProcessorDataStream (processor->getNodeId(), streamId);

    ASSERT_EQ (mappedDataStream->getContinuousChannels().size(),
               originalDataStream->getContinuousChannels().size());
    for (std::pair<int, int> oldToNew : oldToNewChanMapping)
    {
        ASSERT_EQ (
            mappedDataStream->getContinuousChannels()[oldToNew.second]->getGlobalIndex(),
            originalDataStream->getContinuousChannels()[oldToNew.first]->getGlobalIndex());
    }
}

TEST_F (ChannelMapTests, TestRespectsEnabledChannels)
{
    int expectedMappedNumChans = 4;
    // Make it backwards:
    juce::Array<int> newChanOrder;
    std::unordered_map<int, int> oldToNewChanMapping;
    for (int i = 0; i < numChannels; i++)
    {
        if (i < expectedMappedNumChans)
        {
            // Enable channels explicitly
            processor->setChannelEnabled (streamId, i, 1);
            oldToNewChanMapping[i] = i;
        }
        else
        {
            // Disable some channels
            processor->setChannelEnabled (streamId, i, 0);
        }

        // Always map all channels, or else it'll be ignored
        newChanOrder.add (i);
    }

    processor->setChannelOrder (streamId, newChanOrder);
    processor->updateSettings();

    auto originalDataStream = tester->getSourceNodeDataStream (streamId);
    auto mappedDataStream = tester->getProcessorDataStream (processor->getNodeId(), streamId);

    ASSERT_EQ (originalDataStream->getContinuousChannels().size(), numChannels);
    ASSERT_EQ (mappedDataStream->getContinuousChannels().size(), expectedMappedNumChans);
    for (std::pair<int, int> oldToNew : oldToNewChanMapping)
    {
        ASSERT_EQ (
            mappedDataStream->getContinuousChannels()[oldToNew.second]->getGlobalIndex(),
            originalDataStream->getContinuousChannels()[oldToNew.first]->getGlobalIndex());
    }
}

TEST_F (ChannelMapTests, ConfigFileTest)
{
    std::vector<std::string> prbFileContentsList = {
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

    std::stringstream prbFileSs;
    for (const auto& line : prbFileContentsList)
    {
        prbFileSs << line << std::endl;
    }

    std::string prbFileContents = prbFileSs.str();
    auto f = juce::File (prbFilePath.string());
    {
        // Write and close it via braces
        juce::FileOutputStream outStream (f);
        outStream.writeString (prbFileContents);
    }

    processor->loadStreamSettings (streamId, f);
    processor->updateSettings();

    auto originalDataStream = tester->getSourceNodeDataStream (streamId);
    auto mappedDataStream = tester->getProcessorDataStream (processor->getNodeId(), streamId);

    ASSERT_EQ (originalDataStream->getContinuousChannels().size(), numChannels);
    ASSERT_EQ (
        mappedDataStream->getContinuousChannels().size(),
        originalDataStream->getContinuousChannels().size());
    for (std::pair<int, int> oldToNew :
         std::unordered_map<int, int> ({
             { 0, 7 },
             { 1, 6 },
             { 2, 5 },
             { 3, 4 },
             { 4, 3 },
             { 5, 2 },
             { 6, 1 },
             { 7, 0 },
         }))
    {
        ASSERT_EQ (
            mappedDataStream->getContinuousChannels()[oldToNew.second]->getGlobalIndex(),
            originalDataStream->getContinuousChannels()[oldToNew.first]->getGlobalIndex());
    }
}
