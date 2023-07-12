/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#include "CommonAverageRef.h"
#include "CommonAverageRefEditor.h"


CARSettings::CARSettings()
{
    m_avgBuffer = AudioBuffer<float>(1, 10000); // 1-dimensional buffer to hold the average
}

CommonAverageRef::CommonAverageRef()
    : GenericProcessor ("Common Avg Ref") 
{
    addSelectedChannelsParameter(Parameter::STREAM_SCOPE,
                                 "affected",
                                 "Affected",
                                 "Channels from which the average is subtracted");
    
    addSelectedChannelsParameter(Parameter::STREAM_SCOPE,
                                 "reference",
                                 "Reference",
                                 "Channels to use as the reference");
    
    addFloatParameter(Parameter::STREAM_SCOPE,
                      "gain",
                      "Gain",
                      "Multiplier for reference value",
                      "", //Unitless
                      100.0f,
                      0.0f,
                      100.0f,
                      1.0f);
}


CommonAverageRef::~CommonAverageRef()
{
}


AudioProcessorEditor* CommonAverageRef::createEditor()
{
    editor = std::make_unique<CommonAverageRefEditor> (this);
    return editor.get();
}


void CommonAverageRef::updateSettings()
{
    settings.update(getDataStreams());
    
}

void CommonAverageRef::process (AudioBuffer<float>& buffer)
{

    for (auto stream : getDataStreams())
    {

        if ((*stream)["enable_stream"])
        {
            CARSettings* settings_ = settings[stream->getStreamId()];

            const int numSamples = getNumSamplesInBlock(stream->getStreamId());
            const int numReferenceChannels = (*stream)["reference"].getArray()->size();
            const int numAffectedChannels = (*stream)["affected"].getArray()->size();

            // There is no need to do any processing if either number of reference or affected channels is zero.
            if (!numReferenceChannels
                || !numAffectedChannels)
            {
                return;
            }

            settings_->m_avgBuffer.clear();

            for (int i = 0; i < numReferenceChannels; ++i)
            {
                int localIndex = (*stream)["reference"][i];
                int globalIndex = stream->getContinuousChannels()[localIndex]->getGlobalIndex();

                settings_->m_avgBuffer.addFrom(0,       // destChannel
                    0,                                  // destStartSample
                    buffer,                             // source
                    globalIndex,                        // sourceChannel
                    0,                                  // sourceStartSample
                    numSamples,                         // numSamples
                    1.0f);                              // gain to apply
            }

            settings_->m_avgBuffer.applyGain(1.0f / float(numReferenceChannels));

            const float gain = -1.0f * float((*stream)["Gain"]) / 100.f;

            for (int i = 0; i < numAffectedChannels; ++i)
            {
                int localIndex = (*stream)["affected"][i];
                int globalIndex = stream->getContinuousChannels()[localIndex]->getGlobalIndex();

                buffer.addFrom(globalIndex,                // destChannel
                    0,                                     // destStartSample
                    settings_->m_avgBuffer,                // source
                    0,                                     // sourceChannel
                    0,                                     // sourceStartSample
                    numSamples,                            // numSamples
                    gain);                                 // gain to apply
            }
        }

        
    }

   
}

