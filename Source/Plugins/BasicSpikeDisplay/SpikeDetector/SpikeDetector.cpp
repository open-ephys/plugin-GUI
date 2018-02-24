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


SpikeDetector::SpikeDetector()
    : GenericProcessor      ("Spike Detector")
    , overflowBuffer        (2, 100)
    , dataBuffer            (nullptr),
      overflowBufferSize    (100)
    , currentElectrode      (-1)
    , uniqueID              (0)
{
    setProcessorType (PROCESSOR_TYPE_FILTER);

    //// the standard form:
    electrodeTypes.add ("single electrode");
    electrodeTypes.add ("stereotrode");
    electrodeTypes.add ("tetrode");

    //// the technically correct form (Greek cardinal prefixes):
    // electrodeTypes.add("hentrode");
    // electrodeTypes.add("duotrode");
    // electrodeTypes.add("triode");
    // electrodeTypes.add("tetrode");
    // electrodeTypes.add("pentrode");
    // electrodeTypes.add("hextrode");
    // electrodeTypes.add("heptrode");
    // electrodeTypes.add("octrode");
    // electrodeTypes.add("enneatrode");
    // electrodeTypes.add("decatrode");
    // electrodeTypes.add("hendecatrode");
    // electrodeTypes.add("dodecatrode");
    // electrodeTypes.add("triskaidecatrode");
    // electrodeTypes.add("tetrakaidecatrode");
    // electrodeTypes.add("pentakaidecatrode");
    // electrodeTypes.add("hexadecatrode");
    // electrodeTypes.add("heptakaidecatrode");
    // electrodeTypes.add("octakaidecatrode");
    // electrodeTypes.add("enneakaidecatrode");
    // electrodeTypes.add("icosatrode");

    for (int i = 0; i < electrodeTypes.size() + 1; ++i)
    {
        electrodeCounter.add (0);
    }

}


SpikeDetector::~SpikeDetector()
{
}


AudioProcessorEditor* SpikeDetector::createEditor()
{
    editor = new SpikeDetectorEditor (this, true);
    return editor;
}

void SpikeDetector::createSpikeChannels()
{
	for (int i = 0; i < electrodes.size(); ++i)
	{
		SimpleElectrode* elec = electrodes[i];
		unsigned int nChans = elec->numChannels;
		Array<const DataChannel*> chans;
		for (int c = 0; c < nChans; c++)
		{
			const DataChannel* ch = getDataChannel(elec->channels[c]);
			if (!ch)
			{
				//not enough channels for the electrodes
				return;
			}
			chans.add(ch);
		}
		SpikeChannel* spk = new SpikeChannel(SpikeChannel::typeFromNumChannels(nChans), this, chans);
		spk->setNumSamples(elec->prePeakSamples, elec->postPeakSamples);
		spikeChannelArray.add(spk);
	}
}


void SpikeDetector::updateSettings()
{
	if (getNumInputs() > 0)
	{
		overflowBuffer.setSize(getNumInputs(), overflowBufferSize);
		overflowBuffer.clear();
	}

}


bool SpikeDetector::addElectrode (int nChans, int electrodeID)
{
    std::cout << "Adding electrode with " << nChans << " channels." << std::endl;

    int firstChan;

    if (electrodes.size() == 0)
    {
        firstChan = 0;
    }
    else
    {
        SimpleElectrode* e = electrodes.getLast();
        firstChan = *(e->channels + (e->numChannels - 1)) + 1;
    }

    if (firstChan + nChans > getNumInputs())
    {
        firstChan = 0; // make sure we don't overflow available channels
    }

    int currentVal = electrodeCounter[nChans];
    electrodeCounter.set (nChans, ++currentVal);

    String electrodeName;

    // hard-coded for tetrode configuration
    if (nChans < 3)
        electrodeName = electrodeTypes[nChans - 1];
    else
        electrodeName = electrodeTypes[nChans - 2];

    String newName = electrodeName.substring (0,1);
    newName = newName.toUpperCase();
    electrodeName = electrodeName.substring (1, electrodeName.length());
    newName += electrodeName;
    newName += " ";
    newName += electrodeCounter[nChans];

    SimpleElectrode* newElectrode = new SimpleElectrode;

    newElectrode->name = newName;
    newElectrode->numChannels = nChans;
    newElectrode->prePeakSamples = 8;
    newElectrode->postPeakSamples = 32;
    newElectrode->thresholds.malloc (nChans);
    newElectrode->isActive.malloc (nChans);
    newElectrode->channels.malloc (nChans);
    newElectrode->isMonitored = false;

    for (int i = 0; i < nChans; ++i)
    {
        *(newElectrode->channels + i) = firstChan+i;
        *(newElectrode->thresholds + i) = getDefaultThreshold();
        *(newElectrode->isActive + i) = true;
    }

    if (electrodeID > 0) 
    {
        newElectrode->electrodeID = electrodeID;
        uniqueID = std::max (uniqueID, electrodeID);
    }
    else
    {
        newElectrode->electrodeID = ++uniqueID;
    }

    resetElectrode (newElectrode);

    electrodes.add (newElectrode);

    currentElectrode = electrodes.size() - 1;

    return true;
}


float SpikeDetector::getDefaultThreshold() const
{
    return 50.0f;
}


StringArray SpikeDetector::getElectrodeNames() const
{
    StringArray names;

    for (int i = 0; i < electrodes.size(); ++i)
    {
        names.add (electrodes[i]->name);
    }

    return names;
}


void SpikeDetector::resetElectrode (SimpleElectrode* e)
{
    e->lastBufferIndex = 0;
}


bool SpikeDetector::removeElectrode (int index)
{
    // std::cout << "Spike detector removing electrode" << std::endl;

    if (index > electrodes.size() || index < 0)
        return false;

    electrodes.remove (index);
    return true;
}


void SpikeDetector::setElectrodeName (int index, String newName)
{
    electrodes[index - 1]->name = newName;
}


void SpikeDetector::setChannel (int electrodeIndex, int channelNum, int newChannel)
{
    std::cout << "Setting electrode " << electrodeIndex << " channel " << channelNum
                << " to " << newChannel << std::endl;

    *(electrodes[electrodeIndex]->channels + channelNum) = newChannel;
}


int SpikeDetector::getNumChannels (int index) const
{
    if (index < electrodes.size())
        return electrodes[index]->numChannels;
    else
        return 0;
}


int SpikeDetector::getChannel (int index, int i) const
{
    return *(electrodes[index]->channels + i);
}


void SpikeDetector::getElectrodes (Array<SimpleElectrode*>& electrodeArray)
{
    electrodeArray.addArray (electrodes);
}


SimpleElectrode* SpikeDetector::setCurrentElectrodeIndex (int i)
{
    jassert (i >= 0 & i < electrodes.size());
    currentElectrode = i;

    return electrodes[i];
}


SimpleElectrode* SpikeDetector::getActiveElectrode() const
{
    if (electrodes.size() == 0)
        return nullptr;

    return electrodes[currentElectrode];
}


void SpikeDetector::setChannelActive (int electrodeIndex, int subChannel, bool active)
{
    currentElectrode = electrodeIndex;
    currentChannelIndex = subChannel;

    std::cout << "Setting channel active to " << active << std::endl;

    if (active)
        setParameter (98, 1);
    else
        setParameter (98, 0);
}


bool SpikeDetector::isChannelActive (int electrodeIndex, int i)
{
    return *(electrodes[electrodeIndex]->isActive + i);
}


void SpikeDetector::setChannelThreshold (int electrodeNum, int channelNum, float thresh)
{
    currentElectrode = electrodeNum;
    currentChannelIndex = channelNum;

    std::cout << "Setting electrode " << electrodeNum << " channel threshold " << channelNum << " to " << thresh << std::endl;

    setParameter (99, thresh);
}


double SpikeDetector::getChannelThreshold(int electrodeNum, int channelNum) const
{
    return *(electrodes[electrodeNum]->thresholds + channelNum);
}


void SpikeDetector::setParameter (int parameterIndex, float newValue)
{
    //editor->updateParameterButtons(parameterIndex);

    if (parameterIndex == 99 && currentElectrode > -1)
    {
        *(electrodes[currentElectrode]->thresholds + currentChannelIndex) = newValue;
    }
    else if (parameterIndex == 98 && currentElectrode > -1)
    {
        if (newValue == 0.0f)
            *(electrodes[currentElectrode]->isActive + currentChannelIndex) = false;
        else
            *(electrodes[currentElectrode]->isActive + currentChannelIndex) = true;
    }
}


bool SpikeDetector::enable()
{
    sampleRateForElectrode = (uint16_t) getSampleRate();

    useOverflowBuffer.clear();

    for (int i = 0; i < electrodes.size(); ++i)
        useOverflowBuffer.add (false);

    return true;
}


bool SpikeDetector::disable()
{
    for (int n = 0; n < electrodes.size(); ++n)
    {
        resetElectrode (electrodes[n]);
    }

    return true;
}


void SpikeDetector::addWaveformToSpikeObject (SpikeEvent::SpikeBuffer& s,
                                              int& peakIndex,
                                              int& electrodeNumber,
                                              int& currentChannel)
{
    int spikeLength = electrodes[electrodeNumber]->prePeakSamples
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

    sampleIndex -= spikeLength; // reset sample index
}


void SpikeDetector::process (AudioSampleBuffer& buffer)
{
    // cycle through electrodes
    SimpleElectrode* electrode;
    dataBuffer = &buffer;

    //std::cout << dataBuffer.getMagnitude(0,nSamples) << std::endl;

    for (int i = 0; i < electrodes.size(); ++i)
    {
        //  std::cout << "ELECTRODE " << i << std::endl;

        electrode = electrodes[i];

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
                        //std::cout << "Spike detected on electrode " << i << std::endl;
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
						SpikeEvent::SpikeBuffer spikeData(spikeChan);
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
						SpikeEventPtr newSpike = SpikeEvent::createSpikeEvent(spikeChan, timestamp, thresholds, spikeData, 0);

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
    }
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
    for (int i = 0; i < electrodes.size(); ++i)
    {
        XmlElement* electrodeNode = parentElement->createNewChildElement ("ELECTRODE");
        electrodeNode->setAttribute ("name",             electrodes[i]->name);
        electrodeNode->setAttribute ("numChannels",      electrodes[i]->numChannels);
        electrodeNode->setAttribute ("prePeakSamples",   electrodes[i]->prePeakSamples);
        electrodeNode->setAttribute ("postPeakSamples",  electrodes[i]->postPeakSamples);
        electrodeNode->setAttribute ("electrodeID",      electrodes[i]->electrodeID);

        for (int j = 0; j < electrodes[i]->numChannels; ++j)
        {
            XmlElement* channelNode = electrodeNode->createNewChildElement ("SUBCHANNEL");
            channelNode->setAttribute ("ch",        *(electrodes[i]->channels + j));
            channelNode->setAttribute ("thresh",    *(electrodes[i]->thresholds + j));
            channelNode->setAttribute ("isActive",  *(electrodes[i]->isActive + j));
        }
    }
}


void SpikeDetector::loadCustomParametersFromXml()
{
    if (parametersAsXml != nullptr) // prevent double-loading
    {
        // use parametersAsXml to restore state

        SpikeDetectorEditor* sde = (SpikeDetectorEditor*) getEditor();

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

        sde->checkSettings();
    }
}

