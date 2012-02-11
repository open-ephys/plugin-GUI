/*
  ==============================================================================

    SpikeDetector.cpp
    Created: 14 Aug 2011 3:36:00pm
    Author:  jsiegle

  ==============================================================================
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

	SpikeDetectorEditor* editor = new SpikeDetectorEditor(this, viewport);
	
	std::cout << "Creating editor." << std::endl;

    setEditor(editor);

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


void SpikeDetector::prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock)
{
	//std::cout << "SpikeDetector node preparing." << std::endl;
	prePeakSamples = int((prePeakMs / 1000.0f) / (1/sampleRate));
	postPeakSamples = int((postPeakMs / 1000.0f) / (1/sampleRate));

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
    for (int ds = 0; ds < config->numDataSources(); ds++)
    {
        for (int tt = 0; tt < config->getSource(ds)->numTetrodes(); tt++)
        {

            Trode* t = config->getSource(ds)->getTetrode(tt);

            for (int ch = 0; ch < t->numChannels(); ch++)
            {
                thresh.set(t->getChannel(ch),t->getThreshold(ch));
                channels.set(t->getChannel(ch),t->getRawDataPointer());
                nChans.set(t->getChannel(ch),t->numChannels());
                isActive.set(t->getChannel(ch),t->getState(ch));
            }
        }
    }

}

void SpikeDetector::releaseResources() 
{	
    thresh.clear();
    channels.clear();
    nChans.clear();
    isActive.clear();
    lastSpike.clear();
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
