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

#ifndef __FILTERNODE_H_CED428E__
#define __FILTERNODE_H_CED428E__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Dsp/Dsp.h"
#include "GenericProcessor.h"
#include "Editors/FilterEditor.h"

/**

  Filters data using a filter from the DSP library.

  The user can select the low- and high-frequency cutoffs.

  @see GenericProcessor, FilterEditor

*/


class FilterEditor; 
class FilterViewport;

class FilterNode : public GenericProcessor

{
public:
	
	FilterNode();
	~FilterNode();
	
	void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);
	void setParameter (int parameterIndex, float newValue);

	AudioProcessorEditor* createEditor();

	bool hasEditor() const {return true;}

	// void setSourceNode(GenericProcessor* sn);
	// void setDestNode(GenericProcessor* dn);
	
	void setNumInputs(int);
	void setSampleRate(float);
	
private:
	double lowCut, highCut;
	Dsp::Filter* filter;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterNode);

	void setFilterParameters();

};





#endif  // __FILTERNODE_H_CED428E__
