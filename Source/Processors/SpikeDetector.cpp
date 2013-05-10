/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "Channel.h"

SpikeDetector::SpikeDetector()
    : GenericProcessor("Spike Detector"),
      overflowBuffer(2,100), dataBuffer(overflowBuffer),
      overflowBufferSize(100), currentElectrode(-1)
{
    //// the standard form:
    electrodeTypes.add("single electrode");
    electrodeTypes.add("stereotrode");
    electrodeTypes.add("tetrode");

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

    for (int i = 0; i < electrodeTypes.size()+1; i++)
    {
        electrodeCounter.add(0);
    }

    spikeBuffer = new uint8_t[MAX_SPIKE_BUFFER_LEN]; // MAX_SPIKE_BUFFER_LEN defined in SpikeObject.h

}

SpikeDetector::~SpikeDetector()
{

}


AudioProcessorEditor* SpikeDetector::createEditor()
{
    editor = new SpikeDetectorEditor(this, true);
    return editor;
}

void SpikeDetector::updateSettings()
{

    if (getNumInputs() > 0)
        overflowBuffer.setSize(getNumInputs(), overflowBufferSize);

    for (int i = 0; i < electrodes.size(); i++)
    {

        Channel* ch = new Channel(this, i);
        ch->isEventChannel = true;
        ch->eventType = electrodes[i]->numChannels;
        ch->name = electrodes[i]->name;

        eventChannels.add(ch);
    }

}

bool SpikeDetector::addElectrode(int nChans)
{
    int firstChan;

    if (electrodes.size() == 0)
    {
        firstChan = 0;
    }
    else
    {
        Electrode* e = electrodes.getLast();
        firstChan = *(e->channels+(e->numChannels-1))+1;
    }

    if (firstChan + nChans > getNumInputs())
    {
        return false;
    }

    int currentVal = electrodeCounter[nChans];
    electrodeCounter.set(nChans,++currentVal);

    String electrodeName;

    // hard-coded for tetrode configuration
    if (nChans < 3)
        electrodeName = electrodeTypes[nChans-1];
    else
        electrodeName = electrodeTypes[nChans-2];

    String newName = electrodeName.substring(0,1);
    newName = newName.toUpperCase();
    electrodeName = electrodeName.substring(1,electrodeName.length());
    newName += electrodeName;
    newName += " ";
    newName += electrodeCounter[nChans];

    Electrode* newElectrode = new Electrode();

    newElectrode->name = newName;
    newElectrode->numChannels = nChans;
    newElectrode->prePeakSamples = 8;
    newElectrode->postPeakSamples = 32;
    newElectrode->thresholds = new double[nChans];
    newElectrode->isActive = new bool[nChans];
    newElectrode->channels = new int[nChans];

    for (int i = 0; i < nChans; i++)
    {
        *(newElectrode->channels+i) = firstChan+i;
        *(newElectrode->thresholds+i) = getDefaultThreshold();
        *(newElectrode->isActive+i) = true;
    }

    resetElectrode(newElectrode);

    electrodes.add(newElectrode);

    return true;

}

float SpikeDetector::getDefaultThreshold()
{
    return 200.0f;
}

StringArray SpikeDetector::getElectrodeNames()
{
    StringArray names;

    for (int i = 0; i < electrodes.size(); i++)
    {
        names.add(electrodes[i]->name);
    }

    return names;
}

void SpikeDetector::resetElectrode(Electrode* e)
{
    e->lastBufferIndex = 0;
}

bool SpikeDetector::removeElectrode(int index)
{

    // std::cout << "Spike detector removing electrode" << std::endl;

    if (index > electrodes.size() || index < 0)
        return false;

    electrodes.remove(index);
    return true;
}

void SpikeDetector::setElectrodeName(int index, String newName)
{
    electrodes[index-1]->name = newName;
}

void SpikeDetector::setChannel(int electrodeIndex, int channelNum, int newChannel)
{
    *(electrodes[electrodeIndex]->channels+channelNum) = newChannel;
}

int SpikeDetector::getNumChannels(int index)
{
    return electrodes[index]->numChannels;
}

int SpikeDetector::getChannel(int index, int i)
{
    return *(electrodes[index]->channels+i);
}

void SpikeDetector::setChannelThreshold(int electrodeNum, int channelNum, float thresh)
{
    currentElectrode = electrodeNum;
    currentChannelIndex = channelNum;
    setParameter(99, thresh);
}

double SpikeDetector::getChannelThreshold(int electrodeNum, int channelNum)
{
    return *(electrodes[electrodeNum]->thresholds+channelNum);
}

void SpikeDetector::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
    if (parameterIndex == 99 && currentElectrode > -1)
    {
        *(electrodes[currentElectrode]->thresholds+currentChannelIndex) = newValue;
    }
}


bool SpikeDetector::enable()
{

    useOverflowBuffer = false;
    return true;
}

bool SpikeDetector::disable()
{

    for (int n = 0; n < electrodes.size(); n++)
    {
        resetElectrode(electrodes[n]);
    }

    return true;
}

// void SpikeDetector::createSpikeEvent(int& peakIndex,
//                                      int& electrodeNumber, int& currentChannel,
//                                      MidiBuffer& eventBuffer)
// {

//     int spikeLength = electrodes[electrodeNumber]->prePeakSamples +
//                         + electrodes[electrodeNumber]->postPeakSamples;

//     uint8 dataSize = spikeLength*2;

//     uint8 data[dataSize];
//     uint8* dataptr = data;

//     // cycle through buffer
//     for (int sample = 0; sample < spikeLength; sample++)
//     {
//          uint16 sampleValue = uint16(getNextSample(currentChannel) / settings.bitVolts[0]);

//          *dataptr++ = uint8(sampleValue >> 8);
//          *dataptr++ = uint8(sampleValue & 255);

//          sampleIndex++;

//     }

//     addEvent(eventBuffer,
//              SPIKE,
//              peakIndex,
//              uint8(electrodeNumber),
//              uint8(currentChannel),
//              dataSize,
//              data);

//     sampleIndex -= spikeLength; // reset sample index

// }

void SpikeDetector::addSpikeEvent(SpikeObject* s, MidiBuffer& eventBuffer, int peakIndex)
{

    // std::cout << "Adding spike event for index " << peakIndex << std::endl;

    int numBytes = packSpike(s, spikeBuffer, 256);

    eventBuffer.addEvent(spikeBuffer, numBytes, peakIndex);

}

void SpikeDetector::addWaveformToSpikeObject(SpikeObject* s,
                                             int& peakIndex,
                                             int& electrodeNumber,
                                             int& currentChannel)
{
    int spikeLength = electrodes[electrodeNumber]->prePeakSamples +
                      + electrodes[electrodeNumber]->postPeakSamples;

    //uint8 dataSize = spikeLength*2;

    // uint8 data[dataSize];
    // uint8* dataptr = data;

    s->nSamples = spikeLength;

    int chan = *(electrodes[electrodeNumber]->channels+currentChannel);

    s->gain[currentChannel] = (int)(1.0f / channels[chan]->bitVolts)*1000;
    s->threshold[currentChannel] = (int) *(electrodes[electrodeNumber]->thresholds+currentChannel) / channels[chan]->bitVolts * 1000;

    // cycle through buffer
    for (int sample = 0; sample < spikeLength; sample++)
    {

        // warning -- be careful of bitvolts conversion
        s->data[currentIndex] = uint16(getNextSample(*(electrodes[electrodeNumber]->channels+currentChannel)) / channels[chan]->bitVolts + 32768);

        currentIndex++;
        sampleIndex++;

        //std::cout << currentIndex << std::endl;

    }


    sampleIndex -= spikeLength; // reset sample index


}

void SpikeDetector::process(AudioSampleBuffer& buffer,
                            MidiBuffer& events,
                            int& nSamples)
{

    // cycle through electrodes
    Electrode* electrode;
    dataBuffer = buffer;

    //std::cout << dataBuffer.getMagnitude(0,nSamples) << std::endl;

    for (int i = 0; i < electrodes.size(); i++)
    {

        //  std::cout << "ELECTRODE " << i << std::endl;

        electrode = electrodes[i];

        // refresh buffer index for this electrode
        sampleIndex = electrode->lastBufferIndex - 1; // subtract 1 to account for
        // increment at start of getNextSample()

        // cycle through samples
        while (samplesAvailable(nSamples))
        {

            sampleIndex++;


            // cycle through channels
            for (int chan = 0; chan < electrode->numChannels; chan++)
            {

                // std::cout << "  channel " << chan << std::endl;

                if (*(electrode->isActive+chan))
                {

                    int currentChannel = *(electrode->channels+chan);

                    if (-getNextSample(currentChannel) > *(electrode->thresholds+chan)) // trigger spike
                    {

                        //std::cout << "Spike detected on electrode " << i << std::endl;
                        // find the peak
                        int peakIndex = sampleIndex;

                        while (-getCurrentSample(currentChannel) <
                               -getNextSample(currentChannel) &&
                               sampleIndex < peakIndex + electrode->postPeakSamples)
                        {
                            sampleIndex++;
                        }

                        peakIndex = sampleIndex;
                        sampleIndex -= (electrode->prePeakSamples+1);

                        SpikeObject newSpike;
                        newSpike.timestamp = peakIndex;
                        newSpike.source = i;
                        newSpike.nChannels = electrode->numChannels;

                        currentIndex = 0;

                        // package spikes;
                        for (int channel = 0; channel < electrode->numChannels; channel++)
                        {

                            addWaveformToSpikeObject(&newSpike,
                                                     peakIndex,
                                                     i,
                                                     channel);

                            // if (*(electrode->isActive+currentChannel))
                            // {

                            //     createSpikeEvent(peakIndex,       // peak index
                            //                      i,               // electrodeNumber
                            //                      currentChannel,  // channel number
                            //                      events);         // event buffer


                            // } // end if channel is active

                        }

                        addSpikeEvent(&newSpike, events, peakIndex);

                        // advance the sample index
                        sampleIndex = peakIndex + electrode->postPeakSamples;

                        break; // quit spike "for" loop
                    } // end spike trigger

                } // end if channel is active
            } // end cycle through channels on electrode

        } // end cycle through samples

        electrode->lastBufferIndex = sampleIndex - nSamples; // should be negative

        //jassert(electrode->lastBufferIndex < 0);

    } // end cycle through electrodes

    // copy end of this buffer into the overflow buffer

    //std::cout << "Copying buffer" << std::endl;
    // std::cout << "nSamples: " << nSamples;
    //std::cout << "overflowBufferSize:" << overflowBufferSize;

    //std::cout << "sourceStartSample = " << nSamples-overflowBufferSize << std::endl;
    // std::cout << "numSamples = " << overflowBufferSize << std::endl;
    // std::cout << "buffer size = " << buffer.getNumSamples() << std::endl;

    if (nSamples > overflowBufferSize)
    {

        for (int i = 0; i < overflowBuffer.getNumChannels(); i++)
        {

            overflowBuffer.copyFrom(i, 0,
                                    buffer, i,
                                    nSamples-overflowBufferSize,
                                    overflowBufferSize);

            useOverflowBuffer = true;
        }

    }
    else
    {

        useOverflowBuffer = false;
    }



}

float SpikeDetector::getNextSample(int& chan)
{



    //if (useOverflowBuffer)
    //{
    if (sampleIndex < 0)
    {
        // std::cout << "  sample index " << sampleIndex << "from overflowBuffer" << std::endl;
        int ind = overflowBufferSize + sampleIndex;

        if (ind < overflowBuffer.getNumSamples())
            return *overflowBuffer.getSampleData(chan, ind);
        else
            return 0;

    }
    else
    {
        //  useOverflowBuffer = false;
        // std::cout << "  sample index " << sampleIndex << "from regular buffer" << std::endl;

        if (sampleIndex < dataBuffer.getNumSamples())
            return *dataBuffer.getSampleData(chan, sampleIndex);
        else
            return 0;
    }
    //} else {
    //    std::cout << "  sample index " << sampleIndex << "from regular buffer" << std::endl;
    //     return *dataBuffer.getSampleData(chan, sampleIndex);
    //}

}

float SpikeDetector::getCurrentSample(int& chan)
{

    // if (useOverflowBuffer)
    // {
    //     return *overflowBuffer.getSampleData(chan, overflowBufferSize + sampleIndex - 1);
    // } else {
    //     return *dataBuffer.getSampleData(chan, sampleIndex - 1);
    // }

    if (sampleIndex < 1)
    {
        //std::cout << "  sample index " << sampleIndex << "from overflowBuffer" << std::endl;
        return *overflowBuffer.getSampleData(chan, overflowBufferSize + sampleIndex - 1);
    }
    else
    {
        //  useOverflowBuffer = false;
        // std::cout << "  sample index " << sampleIndex << "from regular buffer" << std::endl;
        return *dataBuffer.getSampleData(chan, sampleIndex - 1);
    }
    //} else {

}


bool SpikeDetector::samplesAvailable(int& nSamples)
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


