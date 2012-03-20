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

#ifndef __SPIKEDETECTOR_H_3F920F95__
#define __SPIKEDETECTOR_H_3F920F95__

#include "../../JuceLibraryCode/JuceHeader.h"

#include "GenericProcessor.h"
#include "Editors/SpikeDetectorEditor.h"

/**

  == UNDER CONSTRUCTION ==

  Detects spikes in a continuous signal and outputs events containing the spike data.

  @see GenericProcessor, SpikeDetectorEditor

*/

class SpikeDetectorEditor;

class SpikeDetector : public GenericProcessor

{
public:
	
	SpikeDetector();
	~SpikeDetector();
	
	void process(AudioSampleBuffer &buffer, MidiBuffer &events, int& nSamples);
	void setParameter (int parameterIndex, float newValue);

	void updateSettings();

	bool enable();
	bool disable();

	AudioProcessorEditor* createEditor();

	AudioSampleBuffer overflowBuffer;
	AudioSampleBuffer& dataBuffer;

	bool addElectrode(int nChans);
	bool removeElectrode(int index);
	bool setChannel(int electrodeIndex, int channelNum);
	bool setName(int index, String newName);

	int getNumChannels(int index);
	int getChannel(int index, int chan);

	StringArray electrodeTypes;

	StringArray getElectrodeNames();

private:

	float getDefaultThreshold();

	int overflowBufferSize;

	int sampleIndex;
	//int lastBufferIndex;

	Array<int> electrodeCounter;

	float getNextSample(int& chan);
	float getCurrentSample(int& chan);
	bool samplesAvailable(int& nSamples);

	bool useOverflowBuffer;

	struct Electrode {

		String name;

		int numChannels;
		int prePeakSamples, postPeakSamples;
		int lastBufferIndex;

		int* channels;
		double* thresholds;
		bool* isActive;

	};

	Array<Electrode*> electrodes;

	void createSpikeEvent(int& peakIndex,
						  int& electrodeNumber,
						  int& currentChannel,
						  MidiBuffer& eventBuffer);

	void resetElectrode(Electrode*);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDetector);

};



#endif  // __SPIKEDETECTOR_H_3F920F95__
