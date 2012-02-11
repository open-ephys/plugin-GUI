/*
  ==============================================================================

    Splitter.h
    Created: 17 Aug 2011 1:02:57am
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __SPLITTER_H_A75239F7__
#define __SPLITTER_H_A75239F7__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor.h"

#include <stdio.h>

class Splitter : public GenericProcessor
{
public:

	Splitter();
	~Splitter();

	AudioProcessorEditor* createEditor();

	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples) {}

	bool isSplitter() {return true;}

	void switchDest(int);
	void setDestNode(GenericProcessor* dn);

private:

	GenericProcessor* destNodeA;
	GenericProcessor* destNodeB;
	int activePath;
	
};


#endif  // __SPLITTER_H_A75239F7__
