/*
  ==============================================================================

    EventNode.h
    Created: 13 Jun 2011 10:42:26am
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __EVENTNODE_H_9B67A789__
#define __EVENTNODE_H_9B67A789__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "GenericProcessor.h"
//#include "Editors/FilterEditor.h"

class FilterViewport;

class EventNode : public GenericProcessor

{
public:
	
	EventNode();
	~EventNode();
	
	void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);
	void setParameter (int parameterIndex, float newValue);

//	bool enable();
	//bool disable();

	// AudioProcessorEditor* createEditor();
	
private:

	int accumulator;
	bool isSource;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventNode);

};






#endif  // __EVENTNODE_H_9B67A789__
