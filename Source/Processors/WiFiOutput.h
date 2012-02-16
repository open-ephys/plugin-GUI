/*
  ==============================================================================

    WiFiOutput.h
    Created: 15 Feb 2012 9:09:19pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __WIFIOUTPUT_H_94D625CE__
#define __WIFIOUTPUT_H_94D625CE__


#include "../../JuceLibraryCode/JuceHeader.h"
#include "GenericProcessor.h"
#include "Editors/WiFiOutputEditor.h"

#include "../Network/PracticalSocket.h"  // For UDPSocket and SocketException
#include <iostream>           			 // For cout and cerr
#include <cstdlib>            			 // For atoi()


class FilterViewport;

class WiFiOutputEditor;

class WiFiOutput : public GenericProcessor,
		           public Timer

{
public:
	
	WiFiOutput();
	~WiFiOutput();
	
	void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);
	void setParameter (int parameterIndex, float newValue);

	AudioProcessorEditor* createEditor();

	bool isSink() {return true;}
	
private:

	UDPSocket socket;

	void timerCallback();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WiFiOutput);

};





#endif  // __WIFIOUTPUT_H_94D625CE__
