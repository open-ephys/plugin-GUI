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


SpikeDetector::SpikeDetector()
    : GenericProcessor      ("Spike Detector")
    , overflowBuffer        (2, 100)
    , dataBuffer            (nullptr),
      overflowBufferSize    (100)
{
    setProcessorType (PROCESSOR_TYPE_FILTER);

}


SpikeDetector::~SpikeDetector()
{
}


AudioProcessorEditor* SpikeDetector::createEditor()
{
    editor = std::make_unique<SpikeDetectorEditor> (this, true);
    return editor.get();
}


void SpikeDetector::updateSettings()
{
	if (getNumInputs() > 0)
	{
		overflowBuffer.setSize(getNumInputs(), overflowBufferSize);
		overflowBuffer.clear();
	}

    for (int i = 0; i < internalSpikeChannels.size(); ++i)
    {
        spikeChannels.add(new SpikeChannel(*internalSpikeChannels[i]));
    }

}


bool SpikeDetector::addSpikeChannel (SpikeChannel::Type type)
{


    return true;
}


float SpikeDetector::getDefaultThreshold() const
{
    return -50.0f;
}


bool SpikeDetector::removeSpikeChannel (int index)
{
    // std::cout << "Spike detector removing electrode" << std::endl;

    //if (index > electrodes.size() || index < 0)
    //    return false;

    //electrodes.remove (index);
    return true;
}


Array<SpikeChannel*> SpikeDetector::getSpikeChannelsForStream(uint16 streamId)
{
    Array<SpikeChannel*> channels;

    for (int i = 0; i < internalSpikeChannels.size(); i++)
    {
        if (internalSpikeChannels[i]->getStreamId() == streamId)
            channels.add(internalSpikeChannels[i]);
    }

    return channels;
}




void SpikeDetector::setParameter (int parameterIndex, float newValue)
{
    //editor->updateParameterButtons(parameterIndex);

    if (parameterIndex == 99 && currentElectrode > -1)
    {
        //*(electrodes[currentElectrode]->thresholds + currentChannelIndex) = newValue;
    }
    else if (parameterIndex == 98 && currentElectrode > -1)
    {
       // if (newValue == 0.0f)
       //     *(electrodes[currentElectrode]->isActive + currentChannelIndex) = false;
       // else
       //     *(electrodes[currentElectrode]->isActive + currentChannelIndex) = true;
    }
}


bool SpikeDetector::startAcquisition()
{
    //sampleRateForElectrode = (uint16_t) getSampleRate();

    useOverflowBuffer.clear();

    for (int i = 0; i < internalSpikeChannels.size(); ++i)
        useOverflowBuffer.add (false);

    return true;
}


bool SpikeDetector::stopAcquisition()
{
    //for (int n = 0; n < electrodes.size(); ++n)
    //{
    //    resetElectrode (electrodes[n]);
    //}

    return true;
}


void SpikeDetector::addWaveformToSpikeObject (Spike::Buffer& s,
                                              int& peakIndex,
                                              int& electrodeNumber,
                                              int& currentChannel)
{
    /*int spikeLength = electrodes[electrodeNumber]->prePeakSamples
                      + electrodes[electrodeNumber]->postPeakSamples;


    const int chan = *(electrodes[electrodeNumber]->channels + currentChannel);

    if (isChannelActive (electrodeNumber, currentChannel))
    {
		
        for (int sample = 0; sample < spikeLength; ++sample)
        {
            s.set(currentChannel,sample, getNextSample (*(electrodes[electrodeNumber]->channels+currentChannel)));
            ++sampleIndex;

            //std::cout << currentIndex << std::endl;
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

    // cycle through electrodes
    SpikeChannel* electrode;
    dataBuffer = &buffer;

    //std::cout << dataBuffer.getMagnitude(0,nSamples) << std::endl;

    /*for (int i = 0; i < internalSpikeChannels.size(); ++i)
    {
        //  std::cout << "ELECTRODE " << i << std::endl;

        electrode = internalSpikeChannels[i];

        // refresh buffer index for this electrode
        sampleIndex = electrode->lastBufferIndex - 1; // subtract 1 to account for
        // increment at start of getNextSample()

        const int nSamples = getNumSamples (*electrode->channels);

        // cycle through samples
        while (samplesAvailable (nSamples))
        {
            ++sampleIndex;

            // cycle through channels
            for (int chan = 0; chan < electrode->numChannels; ++chan)
            {
                // std::cout << "  channel " << chan << std::endl;
                if (*(electrode->isActive + chan))
                {
                    int currentChannel = *(electrode->channels + chan);

                    if (-getNextSample (currentChannel) > *(electrode->thresholds + chan)) // trigger spike
                    {

                        // find the peak
                        int peakIndex = sampleIndex;

                        while (-getCurrentSample(currentChannel) < -getNextSample(currentChannel)
                               && sampleIndex < peakIndex + electrode->postPeakSamples)
                        {
                            ++sampleIndex;
                        }

                        peakIndex = sampleIndex;

                        sampleIndex -= (electrode->prePeakSamples + 1);

						const SpikeChannel* spikeChan = getSpikeChannel(i);
						Spike::Buffer spikeData(spikeChan);
						Array<float> thresholds;
						for (int channel = 0; channel < electrode->numChannels; ++channel)
						{
							addWaveformToSpikeObject(spikeData,
								peakIndex,
								i,
								channel);
							thresholds.add((int)*(electrode->thresholds + channel));
						}
						int64 timestamp = getTimestamp(electrode->channels[0]) + peakIndex;
						SpikePtr newSpike = Spike::createSpike(spikeChan, timestamp, thresholds, spikeData, 0);

                        // package spikes;
                        
						addSpike(spikeChan, newSpike, peakIndex);


                        // advance the sample index
                        sampleIndex = peakIndex + electrode->postPeakSamples;

                        // quit spike "for" loop
                        break;

                    // end spike trigger
                    }

                // end if channel is active
                }

            // end cycle through channels on electrode
            }

        // end cycle through samples
        }

        electrode->lastBufferIndex = sampleIndex - nSamples; // should be negative

        if (nSamples > overflowBufferSize)
        {
            for (int j = 0; j < electrode->numChannels; ++j)
            {
                overflowBuffer.copyFrom (*electrode->channels+j,
                                         0,
                                         buffer,
                                         *electrode->channels + j,
                                         nSamples-overflowBufferSize,
                                         overflowBufferSize);
            }

            useOverflowBuffer.set (i, true);
        }
        else
        {
            useOverflowBuffer.set (i, false);
        }

    // end cycle through electrodes
    }*/
}


float SpikeDetector::getNextSample (int& chan)
{
    if (sampleIndex < 0)
    {
        const int ind = overflowBufferSize + sampleIndex;

        if (ind < overflowBuffer.getNumSamples())
            return *overflowBuffer.getWritePointer (chan, ind);
        else
            return 0;

    }
    else
    {
        if (sampleIndex < getNumSamples(chan))
            return *dataBuffer->getWritePointer (chan, sampleIndex);
        else
            return 0;
    }
}


float SpikeDetector::getCurrentSample (int& chan)
{
    if (sampleIndex < 1)
    {
        return *overflowBuffer.getWritePointer (chan, overflowBufferSize + sampleIndex - 1);
    }
    else
    {
        return *dataBuffer->getWritePointer (chan, sampleIndex - 1);
    }
}


bool SpikeDetector::samplesAvailable (int nSamples)
{
    if (sampleIndex > nSamples - overflowBufferSize/2)
    {
        return false;
    }
    else
    {
        return true;
    }
}


void SpikeDetector::saveCustomParametersToXml (XmlElement* parentElement)
{
    for (int i = 0; i < internalSpikeChannels.size(); ++i)
    {
        XmlElement* electrodeNode = parentElement->createNewChildElement ("SPIKECHANNEL");
        electrodeNode->setAttribute ("name",             internalSpikeChannels[i]->getName());
        electrodeNode->setAttribute ("numChannels",      (int) internalSpikeChannels[i]->getNumChannels());
        electrodeNode->setAttribute ("prePeakSamples",   (int) internalSpikeChannels[i]->getPrePeakSamples());
        electrodeNode->setAttribute ("postPeakSamples",  (int) internalSpikeChannels[i]->getPrePeakSamples());

        //for (int j = 0; j < internalSpikeChannels[i]->getNumChannels(); ++j)
        //{
        //    XmlElement* channelNode = electrodeNode->createNewChildElement ("SUBCHANNEL");
        ///    channelNode->setAttribute ("ch",        *(internalSpikeChannels[i]->channels + j));
        //    channelNode->setAttribute ("thresh",    *(internalSpikeChannels[i]->thresholds + j));
        //    channelNode->setAttribute ("isActive",  *(internalSpikeChannels[i]->isActive + j));
       // }
    }
}


void SpikeDetector::loadCustomParametersFromXml()
{
    if (parametersAsXml != nullptr) // prevent double-loading
    {
        // use parametersAsXml to restore state

        /*(SpikeDetectorEditor* sde = (SpikeDetectorEditor*) getEditor();

        int electrodeIndex = -1;

        forEachXmlChildElement (*parametersAsXml, xmlNode)
        {
            if (xmlNode->hasTagName ("ELECTRODE"))
            {
                ++electrodeIndex;

                std::cout << "ELECTRODE>>>" << std::endl;

                const int channelsPerElectrode = xmlNode->getIntAttribute ("numChannels");
                const int electrodeID          = xmlNode->getIntAttribute ("electrodeID");

                sde->addElectrode (channelsPerElectrode, electrodeID);

                setElectrodeName (electrodeIndex + 1, xmlNode->getStringAttribute ("name"));
                sde->refreshElectrodeList();

                int channelIndex = -1;

                forEachXmlChildElement (*xmlNode, channelNode)
                {
                    if (channelNode->hasTagName ("SUBCHANNEL"))
                    {
                        ++channelIndex;

                        std::cout << "Subchannel " << channelIndex << std::endl;

                        setChannel          (electrodeIndex, channelIndex, channelNode->getIntAttribute ("ch"));
                        setChannelThreshold (electrodeIndex, channelIndex, channelNode->getDoubleAttribute ("thresh"));
                        setChannelActive    (electrodeIndex, channelIndex, channelNode->getBoolAttribute ("isActive"));
                    }
                }
            }
        }

        sde->checkSettings();*/
    }
}

