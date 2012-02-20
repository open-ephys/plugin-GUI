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
#include "../UI/Configuration.h"

/**

  --UNDER CONSTRUCTION--

  Detects spikes in a continuous signal and outputs events containing the spike data.

  @see GenericProcessor, SpikeDetectorEditor

*/

class SpikeDetectorEditor;
class FilterViewport;

class SpikeDetector : public GenericProcessor

{
public:
	
	SpikeDetector();
	~SpikeDetector();
	
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);
	void setParameter (int parameterIndex, float newValue);

	bool enable();
	bool disable();

	MidiBuffer* getEventBuffer() {return spikeBuffer;}

	AudioProcessorEditor* createEditor();

	bool hasEditor() const {return true;}
	
private:
	double sampleRate, threshold;
	double prePeakMs, postPeakMs;
	int prePeakSamples, postPeakSamples;
	int accumulator;

	Array<float> thresh;
	Array<int*> channels;
	Array<int> nChans;
	Array<bool> isActive;
	Array<int> lastSpike;

	MidiBuffer* spikeBuffer;

	//AudioData::ConverterInstance<AudioData::Float32, AudioData::Int16> converter;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDetector);

};



#endif  // __SPIKEDETECTOR_H_3F920F95__
