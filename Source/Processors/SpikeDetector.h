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

#include "Visualization/SpikeObject.h"

/**

  == UNDER CONSTRUCTION ==

  Detects spikes in a continuous signal and outputs events containing the spike data.

  @see GenericProcessor, SpikeDetectorEditor

*/

class SpikeDetectorEditor;

class SpikeDetector : public GenericProcessor

{
public:
	
	// CONSTRUCTOR AND DESTRUCTOR // 

	/** constructor */
	SpikeDetector();

	/** destructor */
	~SpikeDetector();

	
	// PROCESSOR METHODS // 

	/** Processes an incoming continuous buffer and places new
	    spikes into the event buffer. */
	void process(AudioSampleBuffer &buffer, MidiBuffer &events, int& nSamples);
	
	/** Used to alter parameters of data acquisition. */
	void setParameter (int parameterIndex, float newValue);

	/** Called whenever the signal chain is altered. */
	void updateSettings();

	/** Called prior to start of acquisition. */
	bool enable();

	/** Called after acquisition is finished. */
	bool disable();

	/** Creates the SpikeDetectorEditor. */
	AudioProcessorEditor* createEditor();


	// INTERNAL BUFFERS // 

	/** Extra samples are placed in this buffer to allow seamless
	    transitions between callbacks. */
	AudioSampleBuffer overflowBuffer;

	/** Reference to a continuous buffer (for internal use only). */
	AudioSampleBuffer& dataBuffer;


	// CREATE AND DELETE ELECTRODES // 

	/** Adds an electrode with n channels to be processed. */
	bool addElectrode(int nChans);

	/** Removes an electrode with a given index. */
	bool removeElectrode(int index);


	// EDIT AND QUERY ELECTRODE SETTINGS // 

	/** Returns the number of channels for a given electrode. */
	int getNumChannels(int index);

	/** Edits the mapping between input channels and electrode channels. */
	bool setChannel(int electrodeIndex, int channelNum, int newChannel);

	/** Returns the continuous channel that maps to a given 
		electrode channel. */
	int getChannel(int index, int chan);

	/** Sets the name of a given electrode. */
	bool setElectrodeName(int index, String newName);


	// RETURN STRING ARRAYS // 

	/** Returns a StringArray containing the names of all electrodes */
	StringArray getElectrodeNames();

	/** Returns a list of possible electrode types (e.g., stereotrode, tetrode). */
	StringArray electrodeTypes;

private:

	float getDefaultThreshold();

	int overflowBufferSize;

	int sampleIndex;

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
