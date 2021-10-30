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

SpikeChannelSettings::SpikeChannelSettings(const SpikeChannel::Type type_) :
    type(type_), expectedChannelCount(SpikeChannel::getNumChannels(type)),
    currentSampleIndex(0),
    useOverflowBuffer(false)
{

    sendFullWaveform = true;
    prePeakSamples = 8;
    postPeakSamples = 32;
    thresholdType = SpikeChannelSettings::ThresholdType::FIXED;
    
    for (int i = 0; i < expectedChannelCount; i++)
    {
        thresholds.add(-50);
    }

    maxLocalChannel = 16;

}

void SpikeChannelSettings::setChannelIndexes(
    Array<int> localChannelIndexes_,
    Array<int> globalChannelIndexes_,
    int maxLocalChannel)
{

    jassert(localChannelIndexes_.size() == expectedChannelCount);

    jassert(globalChannelIndexes_.size() == expectedChannelCount);

    globalChannelIndexes = globalChannelIndexes_;
    localChannelIndexes = localChannelIndexes_;

    detectSpikesOnChannel.clear();

    for (int i = 0; i < expectedChannelCount; i++)
    {
         detectSpikesOnChannel.add(localChannelIndexes[i] < maxLocalChannel);
    }
}

void SpikeChannelSettings::toXml(XmlElement* xml)
{
    xml->setAttribute("type", type);

    xml->setAttribute("name", name);
    xml->setAttribute("description", description);

    xml->setAttribute("prePeakSamples", (int) prePeakSamples);
    xml->setAttribute("postPeakSamples", (int) postPeakSamples);

    xml->setAttribute("sendFullWaveform", sendFullWaveform);

    xml->setAttribute("thresholdType", thresholdType);

    for (int j = 0; j < localChannelIndexes.size(); ++j)
    {

        XmlElement* channelNode = xml->createNewChildElement("SUBCHANNEL");

        channelNode->setAttribute("localIndex", localChannelIndexes[j]);
        channelNode->setAttribute("globalIndex", globalChannelIndexes[j]);
        channelNode->setAttribute("detectSpikesOnChannel", detectSpikesOnChannel[j]);
        channelNode->setAttribute("threshold", thresholds[j]);

    }
}

void SpikeChannelSettings::fromXml(XmlElement* xml)
{
    name = xml->getStringAttribute("name");
    description = xml->getStringAttribute("description");

    prePeakSamples = xml->getIntAttribute("prePeakSamples", 8);
    postPeakSamples = xml->getIntAttribute("postPeakSamples", 32);

    sendFullWaveform = xml->getBoolAttribute("sendFullWaveform", true);

    thresholdType = (ThresholdType) xml->getIntAttribute("thresholdType", ThresholdType::FIXED);

    localChannelIndexes.clear();
    globalChannelIndexes.clear();
    thresholds.clear();
    detectSpikesOnChannel.clear();

    forEachXmlChildElement(*xml, subNode)
    {
        if (subNode->hasTagName("SUBCHANNEL"))
        {

            localChannelIndexes.add(subNode->getIntAttribute("localChannelIndex"));
            globalChannelIndexes.add(subNode->getIntAttribute("globalChannelIndex"));
            thresholds.add(subNode->getIntAttribute("threshold"));
            detectSpikesOnChannel.add(subNode->getBoolAttribute("detectSpikesOnChannel"));
        }
    }
}

SpikeDetectorSettings::SpikeDetectorSettings()
{
    nextAvailableChannel = 0;
    nextElectrodeIndex = 1;
}

SpikeDetectorSettings::~SpikeDetectorSettings()
{

}

void SpikeDetectorSettings::toXml(XmlElement* xml)
{
    for (auto channel : spikeChannels)
    {
        XmlElement* node = xml->createNewChildElement("SPIKESOURCE");

        channel->toXml(node);

    }
}

void SpikeDetectorSettings::fromXml(XmlElement* xml)
{

    spikeChannels.clear();

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("SPIKESOURCE"))
        {
            SpikeChannel::Type type = (SpikeChannel::Type) xmlNode->getIntAttribute("type", SpikeChannel::SINGLE);

            SpikeChannelSettings* settings = new SpikeChannelSettings(type);

            settings->fromXml(xmlNode);

            spikeChannels.add(settings);
        }
    }
}

SpikeDetector::SpikeDetector()
    : GenericProcessor      ("Spike Detector")
{
    setProcessorType (PROCESSOR_TYPE_FILTER);
}


SpikeDetector::~SpikeDetector()
{
}


AudioProcessorEditor* SpikeDetector::createEditor()
{
    editor = std::make_unique<SpikeDetectorEditor> (this);
    return editor.get();
}


void SpikeDetector::updateSettings()
{
	if (getNumInputs() > 0)
	{
		overflowBuffer.setSize(getNumInputs(), OVERFLOW_BUFFER_SAMPLES);
		overflowBuffer.clear();
	}

    /*settings.update(getDataStreams());

    for (auto stream : getDataStreams())
    {
        for (auto channel : settings[stream->getStreamId()]->spikeChannels)
        {

            Array<const ContinuousChannel*> sourceChannels;

            for (int i = 0; i < channel->expectedChannelCount; i++)
            {
                sourceChannels.add(getContinuousChannel(channel->globalChannelIndexes[i]));
            }

            unsigned int numPrePeakSamples; 
            unsigned int numPostPeakSamples;

            if (channel->sendFullWaveform)
            {
                numPrePeakSamples = channel->prePeakSamples;
                numPostPeakSamples = channel->postPeakSamples;
            }
            else {
                numPrePeakSamples = 0;
                numPostPeakSamples = 1;
            }
                
            SpikeChannel::Settings settings
            {
                channel->type,

                channel->name,
                channel->description,
                SpikeChannel::getIdentifierFromType(channel->type),

                getDataStream(stream->getStreamId()),

                sourceChannels,
                
                numPrePeakSamples,
                numPostPeakSamples

            };

            spikeChannels.add(new SpikeChannel(settings));

            channel->spikeChannel = spikeChannels.getLast();
        }
    }*/

}


void SpikeDetector::addSpikeChannel (SpikeChannel::Type type, Array<const ContinuousChannel*> sourceChannels)
{
    
    SpikeChannel::Settings settings
    {
        type,

        "BIGBOY",
        "a nice little channel",
        SpikeChannel::getIdentifierFromType(type),

        sourceChannels[0]->getStreamId(),

        sourceChannels

    };
    
    spikeChannels.add(new SpikeChannel(settings));

}


float SpikeDetector::getDefaultThreshold() const
{
    return -50.0f;
}


void SpikeDetector::removeSpikeChannel (SpikeChannel* spikeChannel)
{
 
    spikeChannels.removeObject(spikeChannel);
    
    return true;
}


Array<SpikeChannelSettings*> SpikeDetector::getSpikeChannelsForStream(uint16 streamId)
{
    Array<SpikeChannelSettings*> channels;

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


void SpikeDetector::addWaveformToSpikeObject (Spike::Buffer& s,
                                              int& peakIndex,
                                              uint16& streamId,
                                              int& spikeChannelIndex,
                                              int& continuousChannelIndex)
{
    int spikeLength = settings[streamId]->prePeakSamples
                      + settings[electrodeNumber]->postPeakSamples;


    const int chan = *(electrodes[electrodeNumber]->channels + currentChannel);

    if (isChannelActive (electrodeNumber, currentChannel))
    {
		
        for (int sample = 0; sample < spikeLength; ++sample)
        {
            s.set(currentChannel,sample, getNextSample (*(electrodes[electrodeNumber]->channels+currentChannel)));
            ++sampleIndex;

        }
    }
    else
    {
        for (int sample = 0; sample < spikeLength; ++sample)
        {
            // insert a blank spike if the
			s.set(currentChannel, sample, 0);
            ++sampleIndex;
            //std::cout << currentIndex << std::endl;
        }
    }

    sampleIndex -= spikeLength; // reset sample index*/
}


void SpikeDetector::process (AudioSampleBuffer& buffer)
{

    // cycle through streams
    for (auto stream : getDataStreams())
    {
        const uint16 streamId = stream->getStreamId();
        
        const int nSamples = getNumSourceSamples(streamId);

        // cycle through SpikeChannels
        for (auto spikeChannel : getSpikeChannelsForStream(streamId))
        {

            int sampleIndex = spikeChannel->currentSampleIndex - 1;
            
            // cycle through samples
            while (sampleIndex < nSamples - OVERFLOW_BUFFER_SAMPLES / 2)
            {
                ++sampleIndex;

                // cycle through channels
                for (int ch = 0; ch < spikeChannel->getNumChannels(); ch++)
                {

                    // check whether spike detection is active
                    if (spikeChannel->detectSpikesOnChannel[ch])
                    {
                        int currentChannel = spikeChannel->globalChannelIndexes[ch];

                        float currentSample = getSample(currentChannel, sampleIndex, buffer);

                        if (spikeChannel->thresholder->checkThreshold(ch, currentSample))
                        {

                            // find the peak
                            int peakIndex = sampleIndex;

                            while (getSample(currentChannel, sampleIndex, buffer) > 
                                   getSample(currentChannel, sampleIndex + 1, buffer)
                                && sampleIndex < peakIndex + spikeChannel->postPeakSamples)
                            {
                                ++sampleIndex;
                            }

                            peakIndex = sampleIndex;

                            sampleIndex -= (spikeChannel->prePeakSamples + 1);

                            Spike::Buffer spikeData(spikeChannel);

                            Array<float> thresholds;
                            
                            for (int channel = 0; channel < spikeChannel->getNumChannels(); ++channel)
                            {
                                addWaveformToSpikeObject(spikeData,
                                    peakIndex,
                                    channel,
                                    channel);

                                thresholds.add((int)*(spikeChannel->threshold->getThreshold(channel));
                            }

                            int64 timestamp = getSourceTimestamp(streamId) + peakIndex;

                            SpikePtr newSpike = Spike::createSpike(spikeChannel, timestamp, thresholds, spikeData, 0);

                            addSpike(spikeChannel, newSpike, peakIndex);

                            // advance the sample index
                            sampleIndex = peakIndex + spikeChannel->postPeakSamples;
                            
                            break; // quit channels "for" loop
                        }
                    }
                } // cycle through channels

            } // while (sampleIndex < nSamples - OVERFLOW_BUFFER_SAMPLES)
        
            spikeChannel->lastBufferIndex = sampleIndex - nSamples; // should be negative

            if (nSamples > OVERFLOW_BUFFER_SAMPLES)
            {
                for (int j = 0; j < spikeChannel->getNumChannels(); ++j)
                {
                    overflowBuffer.copyFrom(spikeChannel->globalChannelIndex[j],
                        0,
                        buffer,
                        spikeChannel->globalChannelIndex[j],
                        nSamples - OVERFLOW_BUFFER_SAMPLES,
                        OVERFLOW_BUFFER_SAMPLES);
                }

                spikeChannel->useOverflowBuffer = true;
            }
            else
            {
                spikeChannel->useOverflowBuffer = false;
            }
        
        } // electrode
    
    } // streams

}

float SpikeDetector::getSample (int& globalChannelIndex, int& sampleIndex, AudioBuffer<float>& buffer)
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

    for (auto stream : getDataStreams())
    {
        XmlElement* streamParams = xml->createNewChildElement("STREAM");

        settings[stream->getStreamId()]->toXml(streamParams);
    }

}


void SpikeDetector::loadCustomParametersFromXml()
{
    int streamIndex = 0;

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
    }
}

