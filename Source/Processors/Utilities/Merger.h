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

#ifndef __MERGER_H_ED548E77__
#define __MERGER_H_ED548E77__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor.h"

#include <stdio.h>


/**

  --UNDER CONSTRUCTION--
  
  Allows the user to merge two signal chains.

  @see GenericProcessor, ProcessorGraph

*/

class Merger : public GenericProcessor
{
public:

	Merger();
	~Merger();

	AudioProcessorEditor* createEditor();

	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples) {}
//	void setParameter (int parameterIndex, float newValue) {}

	bool isMerger() {return true;}

	//void tabNumber(int);

	void switchSource(int);
	void switchSource();
	void setMergerSourceNode(GenericProcessor* sn);

	//void setNumOutputs(int);
	void setNumInputs(int);

	bool stillHasSource();

	//int tabA, tabB;

private:

	GenericProcessor* sourceNodeA;
	GenericProcessor* sourceNodeB;



	int activePath;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Merger);
	
};




#endif  // __MERGER_H_ED548E77__
