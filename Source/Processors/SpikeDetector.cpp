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
    : GenericProcessor("Spike Detector"), 
	  sampleRate (40000.0), threshold(5000.0), prePeakMs(0.2), postPeakMs(0.6),
	  accumulator(0)
	
{

	spikeBuffer = new MidiBuffer();

}

SpikeDetector::~SpikeDetector()
{
	deleteAndZero(spikeBuffer);
}


AudioProcessorEditor* SpikeDetector::createEditor()
{

	editor = new SpikeDetectorEditor(this);
	
	//std::cout << "Creating editor." << std::endl;

    //setEditor(editor);

	return editor;
}

void SpikeDetector::setParameter (int parameterIndex, float newValue)
{
	//std::cout << "Message received." << std::endl;
	if (parameterIndex == 0) {
		for (int n = 0; n < getNumOutputs(); n++)
        {
            thresh.set(n,newValue);
        }
	}

}


bool SpikeDetector::enable()
{
	//std::cout << "SpikeDetector node preparing." << std::endl;
	prePeakSamples = int((prePeakMs / 1000.0f) / (1/getSampleRate()));
	postPeakSamples = int((postPeakMs / 1000.0f) / (1/getSampleRate()));

    thresh.ensureStorageAllocated(getNumOutputs());
    channels.ensureStorageAllocated(getNumOutputs());
    nChans.ensureStorageAllocated(getNumOutputs());
    isActive.ensureStorageAllocated(getNumOutputs());
    lastSpike.ensureStorageAllocated(getNumOutputs());

    for (int n = 0; n < getNumOutputs(); n++)
    {
        isActive.set(n,false);
        lastSpike.set(n,-40);
    }

    // check configuration
    for (int ds = 0; ds < getConfiguration()->numDataSources(); ds++)
    {
        for (int tt = 0; tt < getConfiguration()->getSource(ds)->numTetrodes(); tt++)
        {

            Trode* t = getConfiguration()->getSource(ds)->getTetrode(tt);

            for (int ch = 0; ch < t->numChannels(); ch++)
            {
                thresh.set(t->getChannel(ch),t->getThreshold(ch));
                channels.set(t->getChannel(ch),t->getRawDataPointer());
                nChans.set(t->getChannel(ch),t->numChannels());
                isActive.set(t->getChannel(ch),t->getState(ch));
            }
        }
    }

    return true;

}

bool SpikeDetector::disable() 
{	
    thresh.clear();
    channels.clear();
    nChans.clear();
    isActive.clear();
    lastSpike.clear();

    return true;
}

void SpikeDetector::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &midiMessages,
                            int& nSamples)
{

	int maxSamples = nSamples;//getNumSamples(midiMessages);
	int spikeSize = 2 + prePeakSamples*2 + postPeakSamples*2; 

    spikeBuffer->clear();

    for (int sample = prePeakSamples + 1; sample < maxSamples - postPeakSamples - 1; sample++)
    {
        for (int chan = 0; chan < getNumOutputs(); chan++) 
        {
            if (isActive[chan] && lastSpike[chan]+spikeSize < sample) // channel is active
            {
                if (*buffer.getSampleData(chan,sample) > thresh[chan])
                {
                    // if thresh cross is detected on one channel
                    // save a waveform on all of them
                   // std::cout << "Thresh cross on channel " << chan << ": " << thresh[chan] << std::endl;
                   // std::cout << "  Capturing spikes on " << nChans[chan] << " channels." << std::endl;

                    for (int wire = 0; wire < nChans[chan]; wire++)
                    {
                        int* firstChan = channels[chan];
                        int channelNum = *(firstChan+wire);

                        //std::cout << "     Found spike on channel " << channelNum << std::endl;

                        uint8 data[spikeSize];
                        data[0] = channelNum >> 8; // channel most-significant byte
                        data[1] = channelNum & 0xFF; // channel least-significant byte

                        // not currently looking for peak, just centering on thresh cross
                        uint8* dataptr = data+2;

                        for (int s = -prePeakSamples; s < postPeakSamples; s++) {
                
                            uint16 sampleValue = uint16(*buffer.getSampleData(channelNum, sample+s) + 32768);

                            *dataptr++ = uint8(sampleValue >> 8);
                            *dataptr++ = uint8(sampleValue & 255);

                        }

                        // broadcast spike
                        spikeBuffer->addEvent(data, // spike data
                              sizeof(data), // total bytes
                              sample);           // sample index
                        
                        // keep track of last spike
                        lastSpike.set(channelNum, sample-prePeakSamples);

                    }
                }                
            }
        }
    }

    // reset lastSpike at the end of each buffer
    for (int n = 0; n < getNumOutputs(); n++)
    {
        lastSpike.set(n,-40);
    }


}
