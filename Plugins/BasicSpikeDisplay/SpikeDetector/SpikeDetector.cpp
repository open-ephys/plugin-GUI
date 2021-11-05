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
#include "SpikeDetector.h"

#include "SpikeDetectorEditor.h"

#define OVERFLOW_BUFFER_SAMPLES 200


AbsValueThresholder::AbsValueThresholder(int numChannels) : Thresholder()
{
    for (int i = 0; i < numChannels; i++)
    {
        thresholds.set(i, -50.0f);
    }
}

void AbsValueThresholder::setThreshold(int channel, float threshold)
{
    if (channel >= 0 && channel < thresholds.size())
        thresholds.set(channel, threshold);
}
    
float AbsValueThresholder::getThreshold(int channel)
{
    if (channel >= 0 && channel < thresholds.size())
        return thresholds[channel];

    return 0.0f;
}

bool AbsValueThresholder::checkSample(int channel, float sample)
{
    if (sample < thresholds[channel])
        return true;
    
    return false;
}
    

SpikeDetector::SpikeDetector()
    : GenericProcessor ("Spike Detector"),
      nextAvailableChannel(0)
{
    
    addStringParameter(Parameter::SPIKE_CHANNEL_SCOPE,
                            "name",
                            "The name of a spike channel",
                            "Default Name");
    
    addCategoricalParameter(Parameter::SPIKE_CHANNEL_SCOPE,
                            "waveform_type",
                            "The type of waveform packaged in each spike object",
                            {"FULL","PEAK"},
                            0);
    
    addFloatParameter(Parameter::SPIKE_CHANNEL_SCOPE,
                      "threshold",
                      "The threshold used for spike detection",
                      -50.0f,
                      -500.0f,
                      0.0f,
                      1.0f);
    
    addSelectedChannelsParameter(Parameter::SPIKE_CHANNEL_SCOPE,
                     "local_channels",
                     "The local channel indices (within a Data Stream) used for spike detection",
                     4);
    
    
}

SpikeDetector::~SpikeDetector()
{
    //mostRecentParameters.clear();
}


AudioProcessorEditor* SpikeDetector::createEditor()
{
    editor = std::make_unique<SpikeDetectorEditor> (this);
    return editor.get();
}

void SpikeDetector::parameterValueChanged(Parameter* p)
{
    if (p->getName().equalsIgnoreCase("name"))
    {
        p->getSpikeChannel()->setName(p->getValueAsString());
        CoreServices::updateSignalChain(getEditor());
    }
    
    else if (p->getName().equalsIgnoreCase("local_channels"))
    {
        
        SelectedChannelsParameter* param = (SelectedChannelsParameter*) p;

        p->getSpikeChannel()->localChannelIndexes = param->getArrayValue();
        
        CoreServices::updateSignalChain(getEditor());
        
        
    } else if (p->getName().equalsIgnoreCase("waveform_type"))
    {
        
        CategoricalParameter* param = (CategoricalParameter*) p;
        SpikeChannel* spikeChannel = p->getSpikeChannel();

        // switch number of channels!!!
        if (param->getSelectedIndex() == 0)
            spikeChannel->sendFullWaveform = true;
        else
            spikeChannel->sendFullWaveform = false;

        CoreServices::updateSignalChain(getEditor());
    }
    else if (p->getName().equalsIgnoreCase("threshold"))
    {
        
        FloatParameter* param = (FloatParameter*) p;
        SpikeChannel* spikeChannel = p->getSpikeChannel();

        for (int i = 0; i < spikeChannel->getNumChannels(); i++)
            spikeChannel->thresholder->setThreshold(i, param->getFloatValue());
    }
        
}

void SpikeDetector::updateSettings()
{
	if (getNumInputs() > 0)
	{
		overflowBuffer.setSize(getNumInputs(), OVERFLOW_BUFFER_SAMPLES);
		overflowBuffer.clear();
	}

}


void SpikeDetector::addSpikeChannel (SpikeChannel::Type type, uint16 currentStream)
{
    
    Array<int> localChannels;
    
    std::cout << "SpikeDetector adding spike channel." << std::endl;
    
    std::cout << "Local channels: ";
    
    for (int i = 0; i < SpikeChannel::getNumChannels(type); i++)
    {
        std::cout << nextAvailableChannel << " ";
        localChannels.add(nextAvailableChannel++);
    }
    
    std::cout << std::endl;
    
    SpikeChannel::Settings settings
    {
        type,

        "name",
        "Spike detector spike channel",
        SpikeChannel::getIdentifierFromType(type),

        localChannels

    };
    
    LOGA("Added spike channel.");
    
    spikeChannels.add(new SpikeChannel(settings));
    spikeChannels.getLast()->addProcessor(processorInfo.get());
    spikeChannels.getLast()->setDataStream(getDataStream(currentStream), false);
    
    spikeChannels.getLast()->thresholder =
        std::make_unique<AbsValueThresholder>(
            SpikeChannel::getNumChannels(type));

}


void SpikeDetector::removeSpikeChannel (SpikeChannel* spikeChannel)
{
 
    spikeChannels.removeObject(spikeChannel);
    
    return true;
}


Array<SpikeChannel*> SpikeDetector::getSpikeChannelsForStream(uint16 streamId)
{
    Array<SpikeChannel*> channels;

    for (auto spikeChannel : spikeChannels)
    {
        if (spikeChannel->getStreamId() == streamId && spikeChannel->isLocal())
            channels.add(spikeChannel);
    }

    return channels;
}



bool SpikeDetector::stopAcquisition()
{
    // cycle through channels
    for (auto spikeChannel : spikeChannels)
    {
        spikeChannel->reset();
    }

    return true;
}


void SpikeDetector::addWaveformToSpikeBuffer (Spike::Buffer& s,
                                              int sampleIndex,
                                              AudioBuffer<float>& buffer)
{
    
    int spikeLength = s.spikeChannel->getTotalSamples();
    
    if (spikeLength == 1)
    {
        sampleIndex += s.spikeChannel->getPrePeakSamples();
    }
    
    for (int sample = 0; sample < spikeLength; ++sample)
    {
        for (int ch = 0; ch < s.spikeChannel->getNumChannels(); ch++)
        {
            if (s.spikeChannel->detectSpikesOnChannel(ch))
            {
                s.set(ch, sample, getSample(s.spikeChannel->globalChannelIndexes[ch],
                                            sampleIndex,
                                            buffer));
            } else {
                s.set(ch, sample, 0);
            }
        }
        ++sampleIndex;
    }
}


void SpikeDetector::process (AudioSampleBuffer& buffer)
{

    // cycle through streams
    for (auto spikeChannel : spikeChannels)
    {
        const uint16 streamId = spikeChannel->getStreamId();
        
        const int nSamples = getNumSourceSamples(streamId);

        int sampleIndex = spikeChannel->currentSampleIndex - 1;
        
       //std::cout << "Checking " << spikeChannel->getName() << std::endl;
        
        // cycle through samples
        while (sampleIndex < nSamples - OVERFLOW_BUFFER_SAMPLES / 2)
        {
            ++sampleIndex;
            
            //std::cout << "Checking sample " << sampleIndex << std::endl;

            // cycle through channels
            for (int ch = 0; ch < spikeChannel->getNumChannels(); ch++)
            {
                
                //std::cout << "Checking channel " << ch << std::endl;
                
                // check whether spike detection is active
                if (spikeChannel->detectSpikesOnChannel(ch))
                {
                    
                    //std::cout << "Active " << ch << std::endl;
                    
                    int currentChannel = spikeChannel->globalChannelIndexes[ch];
                    
                    //std::cout << "Global channel " << currentChannel << std::endl;

                    float currentSample = getSample(currentChannel, sampleIndex, buffer);

                    if (spikeChannel->thresholder->checkSample(ch, currentSample))
                    {
                        
                        //std::cout << "Above thresh " << currentChannel << std::endl;

                        // find the peak
                        int peakIndex = sampleIndex;

                        while (getSample(currentChannel, sampleIndex, buffer) >
                               getSample(currentChannel, sampleIndex + 1, buffer)
                            && sampleIndex < peakIndex + spikeChannel->getPostPeakSamples())
                        {
                            ++sampleIndex;
                        }

                        peakIndex = sampleIndex;

                        sampleIndex -= (spikeChannel->getPrePeakSamples() + 1);
                        
                        // create a buffer to hold the spike data
                        Spike::Buffer spikeBuffer(spikeChannel);

                        // add the waveform
                        addWaveformToSpikeBuffer(spikeBuffer,
                                sampleIndex,
                                buffer);

                        // get the spike timestamp (aligned to the peak index)
                        int64 timestamp = getSourceTimestamp(streamId) + peakIndex;

                        // create a spike object
                        SpikePtr newSpike = Spike::createSpike(spikeChannel,
                                                               timestamp,
                                                               spikeChannel->thresholder->getThresholds(),
                                                               spikeBuffer);
                        
                        // add spike to the outgoing EventBuffer
                        addSpike(newSpike, peakIndex);
                        
                        //std::cout << "Added spike object" << std::endl;

                        // advance the sample index
                        sampleIndex = peakIndex + spikeChannel->getPostPeakSamples();
                        
                        break; // quit channels "for" loop
                    }
                    
                } // if detectSpikesOnChannel
                
            } // cycle through channels

        } // while (sampleIndex < nSamples - OVERFLOW_BUFFER_SAMPLES)
    
        spikeChannel->lastBufferIndex = sampleIndex - nSamples; // should be negative

        if (nSamples > OVERFLOW_BUFFER_SAMPLES)
        {
            for (int j = 0; j < spikeChannel->getNumChannels(); ++j)
            {
                overflowBuffer.copyFrom(spikeChannel->globalChannelIndexes[j],
                      0,
                      buffer,
                     spikeChannel->globalChannelIndexes[j],
                     nSamples - OVERFLOW_BUFFER_SAMPLES,
                     OVERFLOW_BUFFER_SAMPLES);
            }

            spikeChannel->useOverflowBuffer = true;
        }
        else
        {
            spikeChannel->useOverflowBuffer = false;
        }
    
    } // spikeChannel loop
    
}

float SpikeDetector::getSample (int globalChannelIndex, int sampleIndex, AudioBuffer<float>& buffer)
{
    if (sampleIndex < 0)
    {
        return *overflowBuffer.getReadPointer(
            globalChannelIndex,
            OVERFLOW_BUFFER_SAMPLES + sampleIndex);
    }
    else
    {
        return  *buffer.getReadPointer(
            globalChannelIndex,
            sampleIndex);
    }
}


void SpikeDetector::saveCustomParametersToXml (XmlElement* xml)
{

    /*for (auto stream : getDataStreams())
    {
        XmlElement* streamParams = xml->createNewChildElement("STREAM");

        settings[stream->getStreamId()]->toXml(streamParams);
    }*/

}


void SpikeDetector::loadCustomParametersFromXml(XmlElement* xml)
{
    /*int streamIndex = 0;

    Array<const DataStream*> availableStreams = getDataStreams();

    forEachXmlChildElement(*parametersAsXml, streamParams)
    {
        if (streamParams->hasTagName("STREAM"))
        {

            if (availableStreams.size() > streamIndex)
            {
                settings[availableStreams[streamIndex]->getStreamId()]->fromXml(streamParams);
            }
            else {
                LOGD("Did not find stream!");
            }

            streamIndex++;
        }
    }*/
}

