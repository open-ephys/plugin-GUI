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


#include "SignalGenerator.h"
#include <stdio.h>
#include <math.h>

SignalGenerator::SignalGenerator()
	: GenericProcessor("Signal Generator"),

	  defaultFrequency(10.0),
	  defaultAmplitude (100.0f),
	  nOut(5)	
{


}


SignalGenerator::~SignalGenerator()
{

}


AudioProcessorEditor* SignalGenerator::createEditor( )
{
	editor = new SignalGeneratorEditor(this);
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

void SignalGenerator::setParameter (int parameterIndex, float newValue)
{
	//std::cout << "Message received." << std::endl;

	if (currentChannel > -1) {
		if (parameterIndex == 0) {
			amplitude.set(currentChannel,newValue*100.0f);
		} else if (parameterIndex == 1) {
			frequency.set(currentChannel,newValue);
			phasePerSample.set(currentChannel, double_Pi * 2.0 / (getSampleRate() / frequency[currentChannel]));
		} else if (parameterIndex == 2) {
			phase.set(currentChannel, newValue/360.0f * (double_Pi * 2.0));
		} else if (parameterIndex == 3) {
			waveformType.set(currentChannel, (int) newValue);
		}

		//updateWaveform(currentChannel);
	}

}


bool SignalGenerator::enable () {

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

bool SignalGenerator::disable() {
	
	std::cout << "Signal generator received disable signal." << std::endl;
	return true;
}


void SignalGenerator::process(AudioSampleBuffer &buffer,
                            MidiBuffer &midiMessages,
                            int& nSamps)
{

	nSamps = int((float) buffer.getNumSamples() * sampleRateRatio);

    for (int i = 0; i < nSamps; ++i)
    {
        for (int j = buffer.getNumChannels(); --j >= 0;) {

        	float sample;

        	switch (waveformType[j])
        	{
        		case SINE:
        			sample = amplitude[j] * (float) std::sin (currentPhase[j] + phase[j]);
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
					sample = amplitude[j] * (float(rand()) / float(RAND_MAX)-0.5f);
					break;
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
            *buffer.getSampleData (j, i) = sample;
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
float SignalGenerator::generateSpikeSample(double amp, double phase, double noise){
    
    const int N_SAMP = 80;
    
    double waveform[80] =
    {   1.0000, 1.0002, 1.0003, 1.0006, 1.0009, 1.0016, 1.0026, 1.0041, 1.0065, 1.0101, 1.0152, 1.0225, 1.0324, 1.0455, 1.0623, 1.0831, 1.1079,
        1.1363, 1.1675, 1.2001, 1.2324, 1.2623, 1.2876, 1.3061, 1.3161, 1.3163, 1.3062, 1.2863, 1.2575, 1.2216, 1.1808, 1.1375, 1.0941, 1.0527,
        1.0151, 0.9827, 0.9562, 0.9360, 0.9220, 0.9137, 0.9105, 0.9117, 0.9162, 0.9231, 0.9317, 0.9410, 0.9505, 0.9595, 0.9678, 0.9750, 0.9811,
        0.9861, 0.9900, 0.9931, 0.9953, 0.9969, 0.9980, 0.9987, 0.9992, 0.9995, 0.9997, 0.9999, 0.9999, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
        1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000};
    
    
    // We don't want to shift the waveform but scale it, and we don't want to scale
    // the baseline, just the peak of the waveform
    double scale[80] =
    {	1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0001, 1.0004, 1.0009, 1.0020, 1.0044, 1.0091, 1.0175, 1.0317, 1.0540, 1.0863, 1.1295, 1.1827, 1.2420, 1.3011, 1.3521, 1.3867, 1.3990, 1.3867, 1.3521, 1.3011, 1.2420, 1.1827, 1.1295, 1.0863, 1.0540, 1.0317, 1.0175, 1.0091, 1.0044, 1.0020, 1.0009, 1.0004, 1.0001, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000};
    
    
    
    int shift = -4500;//1000 + 32768;
    int sampIdx = 0;
    int gain = 3000;

    double r = ((rand() % 201) - 100) / 1000.0; // Generate random number between -.1 and .1
    noise = r  * noise / (double_Pi * 2); // Shrink the range of r based upon the value of noise
   
    sampIdx = (int) (phase / (2 * double_Pi) * (N_SAMP-1)); // bind between 0 and N_SAMP-1, too tired to figure out the proper math
    //sampIdx = sampIdx + 8;

    float sample = shift + gain * ( ( waveform[sampIdx] + noise ) * pow( scale[sampIdx], amp / 250.0 ) ) ;
    
    return sample;
}
