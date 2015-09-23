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


#include "SignalGenerator.h"
#include <stdio.h>
#include <math.h>
#include "../Visualization/SpikeObject.h"

#ifdef WIN32
#define copysign(x,y) _copysign(x,y)
#endif

SignalGenerator::SignalGenerator()
    : GenericProcessor("Signal Generator"),
      nOut(5), defaultFrequency(10.0), defaultAmplitude(0.5f),
      previousPhase(1000), spikeDelay(0)
{
    parameters.add(Parameter("Amplitude", 0.0005f, 500.0f, .5f, 0, true));
    parameters.add(Parameter("Frequency", 0.01, 10000.0, 10, 1, true));
    parameters.add(Parameter("Phase", -double_Pi, double_Pi, 0, 2, true));
    parameters.add(Parameter("Waveform Type", waveformParameter, 0, 3, true));
}


SignalGenerator::~SignalGenerator()
{

}

/*void SignalGenerator::initializeParameters(){
    parameters.add(Parameter("Amplitude", 0.0005f, 500.0f, .5f, 1, true));
    parameters.add(Parameter("Frequency", 0.01, 10000.0, 10, 2, true));
    parameters.add(Parameter("Phase", -double_Pi, double_Pi, 0, 3, true));
    parameters.add(Parameter("Waveform Type", waveformParameter, 0, 0, true));
}
*/

AudioProcessorEditor* SignalGenerator::createEditor()
{
    editor = new SignalGeneratorEditor(this, false);
    return editor;
}

void SignalGenerator::updateSettings()
{

    //std::cout << "Signal generator updating parameters" << std::endl;

    while (waveformType.size() < getNumOutputs())
    {
        waveformType.add(SINE);
        frequency.add(defaultFrequency);
        amplitude.add(defaultAmplitude);
        phase.add(0);
        phasePerSample.add(double_Pi * 2.0 / (getSampleRate() / frequency.getLast()));
        currentPhase.add(0);
    }

    sampleRateRatio = getSampleRate() / 44100.0;

    std::cout << "Sample rate ratio: " << sampleRateRatio << std::endl;

}

void SignalGenerator::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
    std::cout << "Message received." << std::endl;
    Parameter* parameterPointer=parameters.getRawDataPointer();
    parameterPointer=parameterPointer+parameterIndex;
    editor->updateParameterButtons(parameterIndex);

    if (currentChannel > -1)
    {
        if (parameterIndex == 0)
        {
            amplitude.set(currentChannel,newValue*100.0f);
            parameterPointer->setValue(newValue*100.0f, currentChannel);
        }
        else if (parameterIndex == 1)
        {
            frequency.set(currentChannel,newValue);
            phasePerSample.set(currentChannel, double_Pi * 2.0 / (getSampleRate() / frequency[currentChannel]));
            parameterPointer->setValue(newValue, currentChannel);
        }
        else if (parameterIndex == 2)
        {
            phase.set(currentChannel, newValue/360.0f * (double_Pi * 2.0));
            parameterPointer->setValue(newValue/360.0f * (double_Pi * 2.0), currentChannel);
        }
        else if (parameterIndex == 3)
        {
            waveformType.set(currentChannel, (int) newValue);
            parameterPointer->setValue(newValue, currentChannel);
        }
        //updateWaveform(currentChannel);
    }

}


bool SignalGenerator::enable()
{

    std::cout << "Signal generator received enable signal." << std::endl;

    // for (int n = 0; n < waveformType.size(); n++)
    // {
    // 	updateWaveform(n);

    // }

    return true;
}

// void SignalGenerator::updateWaveform(int n)
// {

// Array<float> cycleData;

// int cycleLength = int(getSampleRate() / frequency[n]);
// float phasePerSample = double_Pi * 2.0 / (getSampleRate() / frequency[n]);

// cycleData.ensureStorageAllocated(cycleLength);

// for (int i = 0; i < cycleLength; i++)
// {
// 	switch (waveformType[n])
// 	{
// 		case SINE:
// 			cycleData.add(amplitude[n] * std::sin(i*phasePerSample + phase[n]));
// 			break;
// 		case SQUARE:
// 			cycleData.add(amplitude[n] * copysign(1,std::sin(i*phasePerSample + phase[n])));
// 			break;
// 		case TRIANGLE:
// 			cycleData.add(0);
// 			break;
// 		case SAW:
// 			cycleData.add(0);
// 			break;
// 		case NOISE:
// 			cycleData.add(0);
// 			break;
// 		default:
// 			cycleData.set(i, 0);
// 	}

// }

// waveforms.set(n, cycleData);

// currentSample.set(n,0);

// }

bool SignalGenerator::disable()
{

    std::cout << "Signal generator received disable signal." << std::endl;
    return true;
}


void SignalGenerator::process(AudioSampleBuffer& buffer,
                              MidiBuffer& midiMessages)
{

    int nSamps = int((float) buffer.getNumSamples() * sampleRateRatio);

    for (int i = 0; i < nSamps; ++i)
    {
        for (int j = buffer.getNumChannels(); --j >= 0;)
        {

            float sample;

            switch (waveformType[j])
            {
                case SINE:
                    sample = amplitude[j] * (float) std::sin(currentPhase[j] + phase[j]);
                    break;
                case SQUARE:
                    sample = amplitude[j] * copysign(1,std::sin(currentPhase[j] + phase[j]));
                    break;
                case TRIANGLE:
                    sample = amplitude[j] * ((currentPhase[j] + phase[j]) / double_Pi - 1) *
                             copysign(2,std::sin(currentPhase[j] + phase[j]));
                    break;
                case SAW:
                    sample = amplitude[j] * ((currentPhase[j] + phase[j]) / double_Pi - 1);
                    break;
                case NOISE:
                    // sample = amplitude[j] * (float(rand()) / float(RAND_MAX)-0.5f);
                    // break;
                case SPIKE:
                    sample = generateSpikeSample(amplitude[j], currentPhase[j], phase[j]);
                    break;
                default:
                    sample = 0;
            }

            currentPhase.set(j,currentPhase[j] + phasePerSample[j]);

            if (currentPhase[j] > double_Pi*2)
                currentPhase.set(j,0);

            // dereference pointer to one of the buffer's samples
            *buffer.getWritePointer(j, i) = sample;
        }
    }


    // for (int chan = 0; chan < buffer.getNumChannels(); chan++)
    // {

    // 	int dataSize = waveforms[chan].size();
    // 	int destSample = -dataSize;
    // 	int lastSample = dataSize;

    // 	while (lastSample < nSamps)
    // 	{

    // 		destSample += dataSize;

    // 		//std::cout << lastSample << " " << destSample << " " << currentSample[chan] << " " << dataSize << std::endl;

    // 		// buffer.copyFrom(chan,
    // 		//  				destSample,
    // 		//  				waveforms[chan].getRawDataPointer() + currentSample[chan],
    // 		//  				dataSize - currentSample[chan]);

    // 		lastSample += dataSize;

    // 		currentSample.set(chan,0);

    // 		//std::cout << "DONE" << std::endl;
    // 	}

    // 	//std::cout << lastSample << " " << destSample << " " << currentSample[chan] << " " << dataSize << std::endl;

    // 	if (destSample < 0)
    // 		destSample = 0;

    // 	int samplesLeft = nSamps - destSample;

    // 	if (samplesLeft < dataSize - currentSample[chan])
    // 	{
    // 	 	// buffer.copyFrom(chan,
    // 	 	// 			destSample,
    // 	 	// 			waveforms[chan].getRawDataPointer() + currentSample[chan],
    // 	 	// 			samplesLeft);

    // 	 	currentSample.set(chan, currentSample[chan] + samplesLeft);

    // 	} else {

    // 		int samps = dataSize - currentSample[chan];

    // 		// buffer.copyFrom(chan,
    // 	 // 				destSample,
    // 	 // 				waveforms[chan].getRawDataPointer() + currentSample[chan],
    // 	 // 				samps);

    // 		destSample += samps;
    // 		samplesLeft -= samps;

    // 		// buffer.copyFrom(chan,
    // 	 // 				destSample,
    // 	 // 				waveforms[chan].getRawDataPointer(),
    // 	 // 				samplesLeft);

    // 		currentSample.set(chan, samplesLeft);
    // 	}

    // }

}
float SignalGenerator::generateSpikeSample(double amp, double phase, double noise)
{


    // if the current phase is less than the previous phase we've probably wrapped and its time to select a new spike
    // if we've delayed long enough then draw a new spike otherwise wait until spikeDelay==0
    if (phase < previousPhase)
    {
        spikeIdx = rand()%5;

        if (spikeDelay <= 0)
            spikeDelay = rand()%200 + 50;
        if (spikeDelay > 0)
            spikeDelay --;
    }


    previousPhase = phase;

    int shift = -9500;//1000 + 32768;
    int gain = 8000;

    double r = ((rand() % 201) - 100) / 1000.0; // Generate random number between -.1 and .1
    noise = r  * noise / (double_Pi * 2); // Shrink the range of r based upon the value of noise

    int sampIdx = (int)(phase / (2 * double_Pi) * (N_WAVEFORM_SAMPLES-1));  // bind between 0 and N_SAMP-1, too tired to figure out the proper math
    //sampIdx = sampIdx + 8;

    // Right now only sample from the 3rd waveform. I need to figure out a way to only sample from a single spike until the phase wraps
    float baseline = shift + gain *  SPIKE_WAVEFORMS[spikeIdx][1] ;

    float sample = shift + gain * (SPIKE_WAVEFORMS[spikeIdx][sampIdx] + noise);  // * pow( WAVEFORM_SCALE[sampIdx], amp / 200.0 ) ) ;
    float dV = sample  - baseline;
    dV = dV * (1 + amp / 250);
    sample = baseline + dV;

    if (spikeDelay==0)
        return sample;
    else
        return baseline;
}
