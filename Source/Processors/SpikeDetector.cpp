/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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
    : GenericProcessor("Spike Detector"), overflowBufferSize(100),
      overflowBuffer(2,100), dataBuffer(overflowBuffer)
	
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

}

SpikeDetector::~SpikeDetector()
{

}


AudioProcessorEditor* SpikeDetector::createEditor()
{
	editor = new SpikeDetectorEditor(this);
	return editor;
}

void SpikeDetector::updateSettings()
{

    overflowBuffer.setSize(getNumInputs(),overflowBufferSize);

    for (int i = 0; i < electrodes.size(); i++)
    {
       settings.eventChannelIds.add(i);
       settings.eventChannelNames.add(electrodes[i]->name);
       settings.eventChannelTypes.add(electrodes[i]->numChannels);
    }

}

bool SpikeDetector::addElectrode(int nChans)
{
    int firstChan;

    int currentVal = electrodeCounter[nChans];
    electrodeCounter.set(nChans,++currentVal);

    String electrodeName = electrodeTypes[nChans-1];
    String newName = electrodeName.substring(0,1);
    newName = newName.toUpperCase();
    electrodeName = electrodeName.substring(1,electrodeName.length());
    newName += electrodeName;
    newName += " ";
    newName += electrodeCounter[nChans];

    if (electrodes.size() == 0)
    {
        firstChan = 0;
    } else {
        Electrode* e = electrodes.getLast();
        firstChan = *(e->channels+(e->numChannels-1))+1;
    }

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

}

float SpikeDetector::getDefaultThreshold()
{
    return 0.08;
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

bool SpikeDetector::setName(int index, String newName)
{
    electrodes[index-1]->name = newName;
}

bool SpikeDetector::setChannel(int electrodeIndex, int channelNum, int newChannel) 
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

void SpikeDetector::setParameter (int parameterIndex, float newValue)
{
	//std::cout << "Message received." << std::endl;
	// if (parameterIndex == 0) 
 //    {
 //        thresh.set(currentChannel,newValue);
	// }

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

void SpikeDetector::createSpikeEvent(int& peakIndex,
                                     int& electrodeNumber, int& currentChannel,
                                     MidiBuffer& eventBuffer)
{

    int spikeLength = electrodes[electrodeNumber]->prePeakSamples +
                        + electrodes[electrodeNumber]->postPeakSamples;

    uint8 dataSize = spikeLength*2;

    uint8 data[dataSize];
    uint8* dataptr = data;

    // cycle through buffer
    for (int sample = 0; sample < spikeLength; sample++)
    {
         uint16 sampleValue = uint16(getNextSample(currentChannel) + 32768);

         *dataptr++ = uint8(sampleValue >> 8);
         *dataptr++ = uint8(sampleValue & 255);

         sampleIndex++;

    }

    addEvent(eventBuffer,
             SPIKE,
             peakIndex,
             uint8(electrodeNumber),
             uint8(currentChannel),
             dataSize,
             data);

    sampleIndex -= spikeLength; // reset sample index

}

void SpikeDetector::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &events,
                            int& nSamples)
{

    // cycle through electrodes
    Electrode* electrode;
    dataBuffer = buffer;

    //std::cout << dataBuffer.getMagnitude(0,nSamples) << std::endl;

    for (int i = 0; i < electrodes.size(); i++)
    {

        //std::cout << "ELECTRODE " << i << std::endl;

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

             //   std::cout << "  channel " << chan << std::endl;

                if (*(electrode->isActive+chan))
                {

                    int currentChannel = *(electrode->channels+chan);

                    if (getNextSample(currentChannel) > *(electrode->thresholds+chan)) // trigger spike
                    {

                        //std::cout << "Spike detected on electrode " << i << std::endl;
                        // find the peak
                        int peakIndex = sampleIndex;

                        while (getCurrentSample(currentChannel) <
                               getNextSample(currentChannel) &&
                               sampleIndex < peakIndex + electrode->postPeakSamples) 
                        {
                            sampleIndex++;
                        }

                        peakIndex = sampleIndex;
                        sampleIndex -= (electrode->prePeakSamples+1);

                        // package spikes;
                        for (int currentChannel = 0; currentChannel < electrode->numChannels; currentChannel++)
                        {

                            if (*(electrode->isActive+currentChannel))
                            {

                                createSpikeEvent(peakIndex,       // peak index
                                                 i,               // electrodeNumber
                                                 currentChannel,  // channel number
                                                 events);         // event buffer


                            } // end if channel is active
                        }

                        // advance the sample index
                        sampleIndex = peakIndex + electrode->postPeakSamples;

                        break; // quit "for" loop
                   } // end spike trigger

               } // end if chanel is active
            } // end cycle through channels
           
        } // end cycle through samples

        electrode->lastBufferIndex = sampleIndex - nSamples; // should be negative

        jassert(electrode->lastBufferIndex < 0);

    } // end cycle through electrodes

    // copy end of this buffer into the overflow buffer

    //std::cout << "Copying buffer" << std::endl;
   // std::cout << "nSamples: " << nSamples;
    //std::cout << "overflowBufferSize:" << overflowBufferSize;

    //if (nSamples > overflowBufferSize) {

    for (int i = 0; i < overflowBuffer.getNumChannels(); i++)
    {
        overflowBuffer.copyFrom(i, 0, 
                                buffer, i, 
                                nSamples-overflowBufferSize, 
                                overflowBufferSize);
    }
        useOverflowBuffer = true;
    //} else {
   //     useOverflowBuffer = false;
   // }
    
}

float SpikeDetector::getNextSample(int& chan)
{

    

    //if (useOverflowBuffer)
    //{
        if (sampleIndex < 0)
        {
          // std::cout << "  sample index " << sampleIndex << "from overflowBuffer" << std::endl;
            return *overflowBuffer.getSampleData(chan, overflowBufferSize + sampleIndex);
        } else {
          //  useOverflowBuffer = false;
          // std::cout << "  sample index " << sampleIndex << "from regular buffer" << std::endl;
            return *dataBuffer.getSampleData(chan, sampleIndex);
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
        } else {
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
    } else {
        return true;
    }

}


