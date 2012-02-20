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

#ifndef __LFPDISPLAYNODE_H_D969A379__
#define __LFPDISPLAYNODE_H_D969A379__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "Editors/LfpDisplayEditor.h"
#include "GenericProcessor.h"

/**
  
  Holds data in a displayBuffer to be used by the LfpDisplayCanvas
  for rendering continuous data streams.

  @see GenericProcessor, LfpDisplayEditor, LfpDisplayCanvas

*/


class DataViewport;

class LfpDisplayNode :  public GenericProcessor
	  

{
public:

	LfpDisplayNode();
	~LfpDisplayNode();

	AudioProcessorEditor* createEditor();

	bool isSink() {return true;}

	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);

	void setParameter(int, float);

	void setNumInputs(int inputs);
	void setSampleRate(float r);

	bool enable();
	bool disable();

	AudioSampleBuffer* getDisplayBufferAddress() {return displayBuffer;}
	int getDisplayBufferIndex() {return displayBufferIndex;}
	ReadWriteLock* getLock() {return lock;}

	bool isVisible;

	ReadWriteLock* lock;

private:

	DataViewport* dataViewport;

	AudioSampleBuffer* displayBuffer;

	MidiBuffer* eventBuffer;

	int displayBufferIndex;

	int repaintInterval, repaintCounter;

	int selectedChan;

	float timebase; // ms
	float displayGain; // 

	int xBuffer, yBuffer, plotHeight, totalHeight;

	bool parameterChanged;

	void resizeBuffer();

	void resized();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfpDisplayNode);

};




#endif  // __LFPDISPLAYNODE_H_D969A379__
