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
        stds.set(i, 50.0/4.0f);
        thresholds.set(i, -50.0f);
        sampleBuffer.add(new Array<float>());
        bufferIndex.add(-1);
    }
}

void StdDevThresholder::setThreshold(int channel, float threshold)
{
    if (channel >= 0 && channel < stdLevels.size())
    {
        //std::cout << "Setting threshold for ch " << channel << " to " << threshold << std::endl;
        stdLevels.set(channel, threshold);
        thresholds.set(channel, - stds[channel] * stdLevels[channel]);
        //std::cout << "Actual threshold: " << thresholds[channel] << std::endl;
    }
        
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

        sampleBuffer[channel]->set(nextIndex, sample);
        
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

    float std = 0;

    for (int i = 0; i < bufferSize; i++)
        std += pow(sampleBuffer[channel]->getUnchecked(i) - mean, 2);

    std = pow(std / bufferSize, 0.5);
    
    stds.set(channel, std);

    float threshold =  - std * stdLevels[channel];

    thresholds.set(channel, threshold);
}


DynamicThresholder::DynamicThresholder(int numChannels) : Thresholder()
{
    for (int i = 0; i < numChannels; i++)
    {
        sigmaLevels.set(i, 4.0f);
        medians.set(i, 50.0 / 4.0f);
        thresholds.set(i, -50.0f);
        sampleBuffer.add(new std::vector<float>(bufferSize));
        bufferIndex.add(-1);
    }
}

void DynamicThresholder::setThreshold(int channel, float threshold)
{
    if (channel >= 0 && channel < sigmaLevels.size())
    {
        sigmaLevels.set(channel, threshold);
        thresholds.set(channel, -medians[channel] * sigmaLevels[channel]);
    }
        
}

float DynamicThresholder::getThreshold(int channel)
{
    if (channel >= 0 && channel < sigmaLevels.size())
        return sigmaLevels[channel];

    return 0.0f;
}

bool DynamicThresholder::checkSample(int channel, float sample)
{

    index += 1;
    index %= skipSamples;

    if (index == 0)
    {
        // update buffer
        int nextIndex = (bufferIndex[channel] + 1) % bufferSize;

        sampleBuffer.getUnchecked(channel)->at(nextIndex) = abs(sample) / scalar;

        bufferIndex.set(channel, nextIndex);

        // compute threshold
        if (nextIndex == bufferSize - 1)
            computeSigma(channel);
    }

    if (sample < thresholds[channel])
        return true;

    return false;
}

void DynamicThresholder::computeSigma(int channel)
{
   
    std::sort(sampleBuffer.getUnchecked(channel)->begin(),
        sampleBuffer.getUnchecked(channel)->end());
    
    float median = sampleBuffer.getUnchecked(channel)->at(bufferSize / 2);

    medians.set(channel, median);
    
    float threshold = - ( median * sigmaLevels[channel]);

    thresholds.set(channel, threshold);
}

    

SpikeDetector::SpikeDetector()
    : GenericProcessor ("Spike Detector"),
      nextAvailableChannel(0),
      singleElectrodeCount(0),
      stereotrodeCount(0),
      tetrodeCount(0)
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
    else if (p->getName().contains("threshold"))
    {
        
        FloatParameter* param = (FloatParameter*) p;
        SpikeChannel* spikeChannel = p->getSpikeChannel();
        
        int channelIndex = p->getName().getTrailingIntValue() - 1;

        spikeChannel->thresholder->setThreshold(channelIndex, param->getFloatValue());
    }
    else if (p->getName().equalsIgnoreCase("thrshlder_type"))
    {
        
        CategoricalParameter* param = (CategoricalParameter*) p;
        SpikeChannel* spikeChannel = p->getSpikeChannel();
        
        if (param->getSelectedString().equalsIgnoreCase("ABS"))
        {
            spikeChannel->thresholder.reset();
            spikeChannel->thresholder =
                std::make_unique<AbsValueThresholder>(
                spikeChannel->getNumChannels());
            
            for (int ch = 0; ch < spikeChannel->getNumChannels(); ch++)
            {
                spikeChannel->thresholder->setThreshold(
                    ch,
                    (float) spikeChannel->getParameter("abs_threshold" + String(ch+1))->getValue());
            }
        } else if (param->getSelectedString().equalsIgnoreCase("STD"))
        {
            spikeChannel->thresholder.reset();
            spikeChannel->thresholder =
                std::make_unique<StdDevThresholder>(
                spikeChannel->getNumChannels());
            
            for (int ch = 0; ch < spikeChannel->getNumChannels(); ch++)
            {
                spikeChannel->thresholder->setThreshold(
                    ch,
                    (float) spikeChannel->getParameter("std_threshold" + String(ch+1))->getValue());
            }
        } else if (param->getSelectedString().equalsIgnoreCase("STD"))
        {
            spikeChannel->thresholder.reset();
            spikeChannel->thresholder =
                std::make_unique<DynamicThresholder>(
                spikeChannel->getNumChannels());
            
            for (int ch = 0; ch < spikeChannel->getNumChannels(); ch++)
            {
                spikeChannel->thresholder->setThreshold(
                    ch,
                    (float) spikeChannel->getParameter("dyn_threshold" + String(ch+1))->getValue());
            }
        }
        
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

String SpikeDetector::ensureUniqueName(String name, uint16 currentStream)
{

   // std::cout << "Candidate name: " << name << std::endl;

    bool matchingName = true;

    int append = 0;

    String nameToCheck = name;

    while (matchingName)
    {
        if (append > 0)
            nameToCheck = name + " (" + String(append) + ")";

        matchingName = false;

        for (auto spikeChannel : getSpikeChannelsForStream(currentStream))
        {
            if (spikeChannel->getName().equalsIgnoreCase(nameToCheck))
            {
                matchingName = true;
                append += 1;
                break;
            }
        }
    }

   // std::cout << "New name: " << nameToCheck;

    nameToCheck.replaceCharacter('|','_');

    return nameToCheck;
}


SpikeChannel* SpikeDetector::addSpikeChannel (SpikeChannel::Type type, 
                                     uint16 currentStream,
                                     int startChannel,
                                     String name)
{
    
    Array<var> selectedChannels;
    Array<int> localChannels;

    if (startChannel > -1)
        settings[currentStream]->nextAvailableChannel = startChannel;

    if (currentStream > 0)
    {
        int numAvailableInputChannels = getDataStream(currentStream)->getChannelCount();
        if (settings[currentStream]->nextAvailableChannel >= numAvailableInputChannels - 1)
        {
            settings[currentStream]->nextAvailableChannel = numAvailableInputChannels - SpikeChannel::getNumChannels(type);
            nextAvailableChannel = settings[currentStream]->nextAvailableChannel;
        }
    }

    for (int i = 0; i < SpikeChannel::getNumChannels(type); i++)
    {

        if (currentStream > 0)
        {
            localChannels.add(settings[currentStream]->nextAvailableChannel++);
            nextAvailableChannel++;
        }
        else {
            localChannels.add(nextAvailableChannel++);
        }
        
        selectedChannels.add(localChannels[i]);
    }
    
    if (name.equalsIgnoreCase(""))
    {

        name = SpikeChannel::getDefaultChannelPrefix(type);

        switch (type)
        {
        case SpikeChannel::SINGLE:
            if (currentStream > 0)
            {
                name += String(++settings[currentStream]->singleElectrodeCount);
                singleElectrodeCount++;
            }
            else {
                name += String(++singleElectrodeCount);
            }
            
            break;
        case SpikeChannel::STEREOTRODE:
            if (currentStream > 0)
            {
                name += String(++settings[currentStream]->stereotrodeCount);
                stereotrodeCount++;
            }
            else {
                name += String(++stereotrodeCount);
            }
            break;
        case SpikeChannel::TETRODE:
            if (currentStream > 0)
            {
                name += String(++settings[currentStream]->tetrodeCount);
                tetrodeCount++;
            }
            else {
                name += String(++tetrodeCount);
            }
            break;
        case SpikeChannel::INVALID:
                break;
        }
    }

    name = ensureUniqueName(name, currentStream);

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
    
    spikeChannels.add(new SpikeChannel(spikeChannelSettings));
    
    SpikeChannel* spikeChannel = spikeChannels.getLast();
    
    spikeChannel->addProcessor(processorInfo.get());

    if (currentStream > 0)
        spikeChannel->setDataStream(getDataStream(currentStream), false);
    
    spikeChannel->thresholder =
        std::make_unique<AbsValueThresholder>(
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
    
    spikeChannel->addParameter(new CategoricalParameter(this,
                              Parameter::SPIKE_CHANNEL_SCOPE,
                              "thrshlder_type",
                              "The type of thresholder to use",
                              {"ABS", "STD", "DYN"},0));
    
    for (int ch = 0; ch < SpikeChannel::getNumChannels(type); ch++)
    {
        spikeChannel->addParameter(new FloatParameter(this,
            Parameter::SPIKE_CHANNEL_SCOPE,
            "abs_threshold" + String(ch+1),
            "Threshold for one channel when the absolute value thresholder is active",
            -50.0f, -500.0f, -20.0f, 1.0f));
        
        spikeChannel->addParameter(new FloatParameter(this,
           Parameter::SPIKE_CHANNEL_SCOPE,
           "std_threshold" + String(ch+1),
           "Threshold for one channel when the std thresholder is active",
           4.0f, 1.0f, 10.0f, 0.01f));
        
        spikeChannel->addParameter(new FloatParameter(this,
          Parameter::SPIKE_CHANNEL_SCOPE,
          "dyn_threshold" + String(ch+1),
          "Threshold for one channel when the dynamic thresholder is active",
          4.0f, 1.0f, 10.0f, 0.01f));
    }
    
    spikeChannel->addParameter(new SelectedChannelsParameter(this,
                     Parameter::SPIKE_CHANNEL_SCOPE,
                     "local_channels",
                     "The local channel indices (within a Data Stream) used for spike detection",
                     selectedChannels,
                     spikeChannel->getNumChannels()));

    //Whenever a new spike channel is created, we need to update the unique ID
    //TODO: This should be automatically done in the SpikeChannel constructor next time we change the API
    // <SOURCE_NODE_ID> | <STREAM_NAME> | <SPIKE_DETECTOR_NODE_ID> | <CHANNEL/ELECTRODE NAME>
    std::string stream_source = std::to_string(getDataStream(currentStream)->getSourceNodeId());
    std::string stream_name = getDataStream(currentStream)->getName().toStdString();
    std::string spike_source = std::to_string(spikeChannel->getSourceNodeId());
    std::string channel_name = spikeChannel->getName().toStdString();

    std::string cacheKey = stream_source + "|" + stream_name + "|"  + spike_source + "|" + channel_name;

    spikeChannel->setIdentifier(cacheKey);
    LOGD("Added SpikeChannel w/ identifier: ", cacheKey);

    return spikeChannel;

}


void SpikeDetector::removeSpikeChannel (SpikeChannel* spikeChannel)
{

    LOGD("Removing spike channel: ", spikeChannel->getName(), " from stream ", spikeChannel->getStreamId());
 
    spikeChannels.removeObject(spikeChannel);

    //Reset electrode and channel counters if no more spike channels after this delete
    if (!spikeChannels.size())
    {

        nextAvailableChannel = 0;
        singleElectrodeCount = 0;
        stereotrodeCount = 0;
        tetrodeCount = 0;

        for (auto& stream : getDataStreams())
        {
            settings[stream->getStreamId()]->singleElectrodeCount = 0;
            settings[stream->getStreamId()]->stereotrodeCount = 0;
            settings[stream->getStreamId()]->tetrodeCount = 0;

            settings[stream->getStreamId()]->nextAvailableChannel = 0;
        }

        //TODO: Can make this smarter by resetting by electrode type
    }
    
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

    //LOGC("SpikeDetector detected ", spikeCount, " spikes in ", totalCallbacks, " callbacks.");

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


void SpikeDetector::process (AudioBuffer<float>& buffer)
{
    totalCallbacks++;

    // cycle through streams
    for (auto spikeChannel : spikeChannels)
    {

        if (spikeChannel->isLocal() && spikeChannel->isValid())
        {

            const uint16 streamId = spikeChannel->getStreamId();

            const int nSamples = getNumSamplesInBlock(streamId);

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
                            int64 sampleNumber = getFirstSampleNumberForBlock(streamId) + peakIndex;

                            // create a spike object
                            SpikePtr newSpike = Spike::createSpike(spikeChannel,
                                                                   sampleNumber,
                                                                   spikeChannel->thresholder->getThresholds(),
                                                                   spikeBuffer);

                            spikeCount++;

                            // add spike to the outgoing EventBuffer
                            addSpike(newSpike);

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

            if (streamId > 0)
            {
                spikeParamsXml->setAttribute("sample_rate", spikeChannel->getSampleRate());
                spikeParamsXml->setAttribute("stream_name", getDataStream(streamId)->getName());
                spikeParamsXml->setAttribute("stream_source", getDataStream(streamId)->getSourceNodeId());
            }
            else {
                spikeParamsXml->setAttribute("sample_rate", 0);
                spikeParamsXml->setAttribute("stream_name", "");
                spikeParamsXml->setAttribute("stream_source", 0);
            }

            // parameters
            spikeChannel->getParameter("local_channels")->toXml(spikeParamsXml);
            spikeChannel->getParameter("thrshlder_type")->toXml(spikeParamsXml);
            
            for (int ch = 0; ch < (int)spikeChannel->getNumChannels(); ch++)
            {
                spikeChannel->getParameter("abs_threshold" + String(ch+1))->toXml(spikeParamsXml);
                spikeChannel->getParameter("std_threshold" + String(ch+1))->toXml(spikeParamsXml);
                spikeChannel->getParameter("dyn_threshold" + String(ch+1))->toXml(spikeParamsXml);
            }
            
            spikeChannel->getParameter("waveform_type")->toXml(spikeParamsXml);
        }
    }

}


void SpikeDetector::loadCustomParametersFromXml(XmlElement* xml)
{

    //std::cout << "Spike detector loading params" << std::endl;

    Array<const DataStream*> availableStreams = getDataStreams();

    for (auto* spikeParamsXml : xml->getChildIterator())
    {
        //std::cout << spikeParamsXml->getTagName() << std::endl;

        if (spikeParamsXml->hasTagName("SPIKE_CHANNEL"))
        {
            String name = spikeParamsXml->getStringAttribute("name", "");

            //std::cout << "SPIKE CHANNEL NAME: " << name << std::endl;

            double sample_rate = spikeParamsXml->getDoubleAttribute("sample_rate", 0.0f);
            String stream_name = spikeParamsXml->getStringAttribute("stream_name", "");
            int stream_source = spikeParamsXml->getIntAttribute("stream_source", 0);

            SpikeChannel::Type type = SpikeChannel::typeFromNumChannels(spikeParamsXml->getIntAttribute("num_channels", 1));

            if (!alreadyLoaded(name, type, stream_source, stream_name))
            {
                uint16 streamId = findSimilarStream(stream_source, stream_name, sample_rate, true);

                if (streamId > 0)
                {
                    //std::cout << "STREAM ID: " << streamId << std::endl;

                    SpikeChannel* spikeChannel = addSpikeChannel(type, streamId, -1, name);

                    spikeChannel->getParameter("local_channels")->fromXml(spikeParamsXml);

                    SelectedChannelsParameter* param = (SelectedChannelsParameter*)spikeChannel->getParameter("local_channels");
                    param->getSpikeChannel()->localChannelIndexes = param->getArrayValue();
                    
                    spikeChannel->getParameter("thrshlder_type")->fromXml(spikeParamsXml);
                    
                    for (int ch = 0; ch < SpikeChannel::getNumChannels(type); ch++)
                    {
                        spikeChannel->getParameter("abs_threshold" + String(ch+1))->fromXml(spikeParamsXml);
                        spikeChannel->getParameter("std_threshold" + String(ch+1))->fromXml(spikeParamsXml);
                        spikeChannel->getParameter("dyn_threshold" + String(ch+1))->fromXml(spikeParamsXml);
                    }
                    
                    spikeChannel->getParameter("waveform_type")->fromXml(spikeParamsXml);
                }
            }
        }
    }
}

bool SpikeDetector::alreadyLoaded(String name, SpikeChannel::Type type, int stream_source, String stream_name)
{
    //std::cout << "Next channel: " << name << ", " << (int) type << ", " << stream_source << std::endl;

    for (auto ch : spikeChannels)
    {
        //std::cout << "Existing channel: " << ch->getName() << ", " << (int)ch->getChannelType() << ", " << getDataStream(ch->getStreamId())->getSourceNodeId() << std::endl;

        if (ch->isLocal())
        {
            
            //std::cout << "LOCAL" << std::endl;

            if (ch->getName() == name && ch->getChannelType() == type
                && getDataStream(ch->getStreamId())->getSourceNodeId() == stream_source
                && getDataStream(ch->getStreamId())->getName() == stream_name)
            {
                //std::cout << "found match." << std::endl;
                return true;
            }
                
        }
       else {
            //std::cout << "Not local" << std::endl;
        }
    }

    return false;
}

