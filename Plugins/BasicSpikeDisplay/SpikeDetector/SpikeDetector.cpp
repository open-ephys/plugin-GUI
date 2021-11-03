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
    : GenericProcessor ("Spike Detector")
{
    

}


AudioProcessorEditor* SpikeDetector::createEditor()
{
    editor = std::make_unique<SpikeDetectorEditor> (this);
    return editor.get();
}

void SpikeDetector::parameterValueChanged(Parameter* p)
{
    if (p->getName().equalsIgnoreCase("name"))
        CoreServices::updateSignalChain(getEditor());
    
    else if (p->getName().equalsIgnoreCase("channels"))
    {
        
        
        SelectedChannelsParameter* param = (SelectedChannelsParameter*) p;
        SpikeChannel* spikeChannel = p->getSpikeChannel();
        uint16 streamId = spikeChannel->getStreamId();
        
        std::cout << "Channels changed for spike channel " << spikeChannel->getName() << " on stream " << streamId << std::endl;
        
        Array<const ContinuousChannel*> arr;
        
        for (auto i : param->getArrayValue())
        {
            const ContinuousChannel* chan = getDataStream(streamId)->getContinuousChannels()[int(i)];
            std::cout << " --> Adding channel " << i << " : " << chan->getName() << std::endl;
            arr.add(chan);
        }
            
        
        p->getSpikeChannel()->setSourceChannels(arr);
        CoreServices::updateSignalChain(getEditor());
    } else if (p->getName().equalsIgnoreCase("waveform"))
    {
        
        
        SelectedChannelsParameter* param = (SelectedChannelsParameter*) p;
        SpikeChannel* spikeChannel = p->getSpikeChannel();
        uint16 streamId = spikeChannel->getStreamId();
        
        std::cout << "Waveform type changed for spike channel " << spikeChannel->getName() << " on stream " << streamId << std::endl;
        
        // switch number of channels!!!
        //p->getSpikeChannel()->setPrePostSamples(arr);
        CoreServices::updateSignalChain(getEditor());
    }
        
}

void SpikeDetector::updateSettings()
{
	if (getNumInputs() > 0)
	{
		overflowBuffer.setSize(getNumInputs(), OVERFLOW_BUFFER_SAMPLES);
		overflowBuffer.clear();
	}
    
    spikeChannels.clear();
    
    {
        for (int ch = 0; ch < 4; ch++)
        {
            std::cout << "Adding tetrode " << ch << std::endl;
            
            Array<const ContinuousChannel*> arr;
            Array<var> channels;
            int startChan = ch* 4;
            for (int i= startChan; i < startChan+4; i++)
            {
                arr.add(continuousChannels[i]);
                channels.add(i);
            }
                
            
            addSpikeChannel("Tetrode " + String(ch + 4), SpikeChannel::TETRODE, arr);
            
            spikeChannels.getLast()->addParameter(new StringParameter(this,
                                                                      Parameter::SPIKE_CHANNEL_SCOPE,
                                                                      "name",
                                                                      "the name of this spike channel",
                                                                      "Tetrode " + String(ch + 4),
                                                                      true));
            
            spikeChannels.getLast()->getParameter("name")->setSpikeChannel(spikeChannels.getLast());
            
            
            
            spikeChannels.getLast()->addParameter(new SelectedChannelsParameter(this,
                                                                      Parameter::SPIKE_CHANNEL_SCOPE,
                                                                      "channels",
                                                                      "the channels for this spike channel",
                                                                      channels,
                                                                      4,
                                                                      true));
            
            spikeChannels.getLast()->getParameter("channels")->setSpikeChannel(spikeChannels.getLast());
            SelectedChannelsParameter* p = (SelectedChannelsParameter*) spikeChannels.getLast()->getParameter("channels");
            p->setChannelCount(continuousChannels.size());
            
            spikeChannels.getLast()->addParameter(new CategoricalParameter(this,
                                                                      Parameter::SPIKE_CHANNEL_SCOPE,
                                                                      "waveform",
                                                                      "the type of waveform to send",
                                                                      {"FULL", "PEAK"},
                                                                      0,
                                                                      true));
            
            spikeChannels.getLast()->getParameter("waveform")->setSpikeChannel(spikeChannels.getLast());
            
            spikeChannels.getLast()->addParameter(new FloatParameter(this,
                                                                      Parameter::SPIKE_CHANNEL_SCOPE,
                                                                      "threshold",
                                                                      "the threshold for this channel",
                                                                      -50.0f,
                                                                      -250.0f,
                                                                      0.0f,
                                                                      5.0f,
                                                                      false));
            
            spikeChannels.getLast()->getParameter("threshold")->setSpikeChannel(spikeChannels.getLast());
            
            spikeChannels.getLast()->thresholder = new AbsValueThresholder(4);
        }
        
    }
}


void SpikeDetector::addSpikeChannel (const String& name, SpikeChannel::Type type, Array<const ContinuousChannel*> sourceChannels)
{
    
    SpikeChannel::Settings settings
    {
        type,

        name,
        "a nice little channel",
        SpikeChannel::getIdentifierFromType(type),

        getDataStream(sourceChannels[0]->getStreamId()),

        sourceChannels

    };
    
    LOGA("Added spike channel.");
    
    spikeChannels.add(new SpikeChannel(settings));
    spikeChannels.getLast()->addProcessor(processorInfo.get());

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

