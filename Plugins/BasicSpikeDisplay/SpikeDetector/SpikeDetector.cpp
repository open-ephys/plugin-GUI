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

SpikeDetectorSettings::SpikeDetectorSettings() :
    nextAvailableChannel(0),
    singleElectrodeCount(0),
    stereotrodeCount(0),
    tetrodeCount(0)
{
    
}

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


StdDevThresholder::StdDevThresholder(int numChannels) : Thresholder()
{
    for (int i = 0; i < numChannels; i++)
    {
        stdLevels.set(i, 4.0f);
        thresholds.set(i, -50.0f);
        sampleBuffer.add(new Array<float>());
        bufferIndex.add(-1);
    }
}

void StdDevThresholder::setThreshold(int channel, float threshold)
{
    if (channel >= 0 && channel < stdLevels.size())
        stdLevels.set(channel, threshold);
}

float StdDevThresholder::getThreshold(int channel)
{
    if (channel >= 0 && channel < stdLevels.size())
        return stdLevels[channel];

    return 0.0f;
}

bool StdDevThresholder::checkSample(int channel, float sample)
{

    index += 1;
    index %= skipSamples;

    if (index == 0)
    {
        // update buffer
        int nextIndex = (bufferIndex[channel] + 1) % bufferSize;

        //std::cout << "Setting sample to " << sample << std::endl;
        //Array<float>* buffer = sampleBuffer[channel];
        
        sampleBuffer[channel]->set(nextIndex, sample);

        //std::cout << sampleBuffer[channel][nextIndex] << std::endl;

        bufferIndex.set(channel, nextIndex);

        // compute threshold
        if (nextIndex == bufferSize - 1)
            computeStd(channel);
    }

    if (sample < thresholds[channel])
        return true;

    return false;
}

void StdDevThresholder::computeStd(int channel)
{
    float mean = 0;

    for (int i = 0; i < bufferSize; i++)
        mean += sampleBuffer[channel]->getUnchecked(i);

    mean /= bufferSize;

    std::cout << "Mean: " << mean << std::endl;

    float std = 0;

    for (int i = 0; i < bufferSize; i++)
        std += pow(sampleBuffer[channel]->getUnchecked(i) - mean, 2);

    std = pow(std / bufferSize, 0.5);

    std::cout << "Std: " << std << std::endl;
  
    float threshold =  - std * stdLevels[channel];

    std::cout << "Threshold ch " << channel << " : " << threshold << std::endl;

    thresholds.set(channel, threshold);
}
    

SpikeDetector::SpikeDetector()
    : GenericProcessor ("Spike Detector")
{
    
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
        std::cout << "Value changed." << std::endl;
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
    settings.update(getDataStreams());
    
	if (getNumInputs() > 0)
	{
		overflowBuffer.setSize(getNumInputs(), OVERFLOW_BUFFER_SAMPLES);
		overflowBuffer.clear();
	}


}


SpikeChannel* SpikeDetector::addSpikeChannel (SpikeChannel::Type type, 
                                     uint16 currentStream,
                                     String name) //,
                                     //Array<int> localChannels,
                                    // int waveform_type,
                                    // float threshold)
{
    
    Array<var> selectedChannels;
    Array<int> localChannels;
    //bool useDefaultChannels = localChannels.size() > 0 ? false : true;

    std::cout << "SpikeDetector adding spike channel." << std::endl;
    
    std::cout << "Local channels: ";
    
    for (int i = 0; i < SpikeChannel::getNumChannels(type); i++)
    {
        std::cout << settings[currentStream]->nextAvailableChannel << " ";

        if (true)
        {
            localChannels.add(settings[currentStream]->nextAvailableChannel++);
        }
        else {
            settings[currentStream]->nextAvailableChannel = localChannels[i] + 1;
        }

        selectedChannels.add(localChannels[i]);
    }
    
    std::cout << std::endl;
    
    if (name.equalsIgnoreCase(""))
    {
        std::cout << "Auto-generating name" << std::endl;

        name = SpikeChannel::getDefaultChannelPrefix(type);

        switch (type)
        {
        case SpikeChannel::SINGLE:
            name += String(++settings[currentStream]->singleElectrodeCount);
            break;
        case SpikeChannel::STEREOTRODE:
            name += String(++settings[currentStream]->stereotrodeCount);
            break;
        case SpikeChannel::TETRODE:
            name += String(++settings[currentStream]->tetrodeCount);
            break;
        }
    }
    else {
        std::cout << "Using given name: " << name << std::endl;
    }
    
    SpikeChannel::Settings spikeChannelSettings
    {
        type,

        name,

        SpikeChannel::getDefaultChannelPrefix(type)
            + "from Spike Detector "
            + String(getNodeId()),

        SpikeChannel::getIdentifierFromType(type),

        localChannels

    };
    
    LOGA("Added spike channel.");
    
    spikeChannels.add(new SpikeChannel(spikeChannelSettings));
    
    SpikeChannel* spikeChannel = spikeChannels.getLast();
    
    spikeChannel->addProcessor(processorInfo.get());

    if (currentStream > 0)
        spikeChannel->setDataStream(getDataStream(currentStream), false);
    
    spikeChannel->thresholder =
        std::make_unique<StdDevThresholder>(
            SpikeChannel::getNumChannels(type));
    
    spikeChannel->addParameter(new StringParameter(this,
                            Parameter::SPIKE_CHANNEL_SCOPE,
                            "name",
                            "The name of a spike channel",
                            name));
    
    spikeChannel->addParameter(new CategoricalParameter(this,
                            Parameter::SPIKE_CHANNEL_SCOPE,
                            "waveform_type",
                            "The type of waveform packaged in each spike object",
                            {"FULL","PEAK"},
                            0));
    
    spikeChannel->addParameter(new FloatParameter(this,
        Parameter::SPIKE_CHANNEL_SCOPE,
        "threshold",
        "The threshold used for spike detection",
        4.0f, 1.0f, 10.0f, 0.01f));
                      //-50.0f,
                      //-500.0f,
                      //-20.0f,
                      //1.0f));
    
    spikeChannel->addParameter(new SelectedChannelsParameter(this,
                     Parameter::SPIKE_CHANNEL_SCOPE,
                     "local_channels",
                     "The local channel indices (within a Data Stream) used for spike detection",
                     selectedChannels,
                     spikeChannel->getNumChannels()));

    return spikeChannel;

}


void SpikeDetector::removeSpikeChannel (SpikeChannel* spikeChannel)
{
 
    spikeChannels.removeObject(spikeChannel);
    
    
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

bool SpikeDetector::startAcquisition()
{
    totalCallbacks = 0;
    spikeCount = 0;

    return true;
}

bool SpikeDetector::stopAcquisition()
{
    // cycle through channels
    for (auto spikeChannel : spikeChannels)
    {
        spikeChannel->reset();
    }

    //std::cout << "Detected " << spikeCount << " spikes in " << totalCallbacks << " callbacks." << std::endl;

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
    totalCallbacks++;

    // cycle through streams
    for (auto spikeChannel : spikeChannels)
    {

        if (spikeChannel->isLocal())
        {

            const uint16 streamId = spikeChannel->getStreamId();

            const int nSamples = getNumSourceSamples(streamId);

            int sampleIndex = spikeChannel->currentSampleIndex - 1;

            // cycle through samples
            while (sampleIndex < nSamples - OVERFLOW_BUFFER_SAMPLES / 2)
            {
                ++sampleIndex;

                // cycle through channels
                for (int ch = 0; ch < spikeChannel->getNumChannels(); ch++)
                {
                    // check whether spike detection is active
                    if (spikeChannel->detectSpikesOnChannel(ch))
                    {

                        int currentChannel = spikeChannel->globalChannelIndexes[ch];

                        float currentSample = getSample(currentChannel, sampleIndex, buffer);

                        if (spikeChannel->thresholder->checkSample(ch, currentSample))
                        {

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

                            spikeCount++;

                            // add spike to the outgoing EventBuffer
                            addSpike(newSpike, peakIndex);

                            // advance the sample index
                            sampleIndex = peakIndex + spikeChannel->getPostPeakSamples();

                            break; // quit channels "for" loop
                        }

                    } // if detectSpikesOnChannel

                } // cycle through channels

            } // while (sampleIndex < nSamples - OVERFLOW_BUFFER_SAMPLES)

            spikeChannel->currentSampleIndex = sampleIndex - nSamples; // should be negative

            //std::cout << spikeChannel->currentSampleIndex << std::endl;

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
                //spikeChannel->currentSampleIndex = -OVERFLOW_BUFFER_SAMPLES / 2;
            }
            else
            {
                spikeChannel->useOverflowBuffer = false;
                //spikeChannel->currentSampleIndex = 0;
            }

        } // local channels
    
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

    for (auto spikeChannel : spikeChannels)
    {

        if (spikeChannel->isLocal())
        {
            const uint16 streamId = spikeChannel->getStreamId();

            XmlElement* spikeParamsXml = xml->createNewChildElement("SPIKE_CHANNEL");

            // general settings
            spikeParamsXml->setAttribute("name", spikeChannel->getName());
            spikeParamsXml->setAttribute("description", spikeChannel->getDescription());
            spikeParamsXml->setAttribute("num_channels", (int)spikeChannel->getNumChannels());

            // stream info
            spikeParamsXml->setAttribute("sample_rate", spikeChannel->getSampleRate());
            spikeParamsXml->setAttribute("stream_name", getDataStream(streamId)->getName());
            spikeParamsXml->setAttribute("stream_source", getDataStream(streamId)->getSourceNodeId());

            // parameters
            spikeChannel->getParameter("local_channels")->toXml(spikeParamsXml);
            spikeChannel->getParameter("threshold")->toXml(spikeParamsXml);
            spikeChannel->getParameter("waveform_type")->toXml(spikeParamsXml);
        }
    }

}


void SpikeDetector::loadCustomParametersFromXml(XmlElement* xml)
{

    Array<const DataStream*> availableStreams = getDataStreams();

    for (auto* spikeParamsXml : xml->getChildIterator())
    {
        if (spikeParamsXml->hasTagName("SPIKE_CHANNEL"))
        {
            String name = spikeParamsXml->getStringAttribute("name", "");

            std::cout << "SPIKE CHANNEL NAME: " << name << std::endl;

            double sample_rate = spikeParamsXml->getDoubleAttribute("sample_rate", 0.0f);
            String stream_name = spikeParamsXml->getStringAttribute("stream_name", "");
            int stream_source = spikeParamsXml->getIntAttribute("stream_source", 0);

            SpikeChannel::Type type = SpikeChannel::typeFromNumChannels(spikeParamsXml->getIntAttribute("num_channels", 1));

            uint16 streamId = findSimilarStream(stream_source, stream_name, sample_rate);

            SpikeChannel* spikeChannel = addSpikeChannel(type, streamId, name);

            spikeChannel->getParameter("local_channels")->fromXml(spikeParamsXml);
            
            SelectedChannelsParameter* param = (SelectedChannelsParameter*)spikeChannel->getParameter("local_channels");
            param->getSpikeChannel()->localChannelIndexes = param->getArrayValue();

            spikeChannel->getParameter("threshold")->fromXml(spikeParamsXml);
            spikeChannel->getParameter("waveform_type")->fromXml(spikeParamsXml);

        }
    }
}

