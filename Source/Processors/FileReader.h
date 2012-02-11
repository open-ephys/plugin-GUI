/*
  ==============================================================================

    FileReader.h
    Created: 13 Aug 2011 7:18:22pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __FILEREADER_H_605BF4A__
#define __FILEREADER_H_605BF4A__


#include "../../JuceLibraryCode/JuceHeader.h"
#include "GenericProcessor.h"

//class FileReaderEditor;

class FileReader : public GenericProcessor

{
public:
	
	// real member functions:
	FileReader();
	~FileReader();
	
	void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);

	void setParameter (int parameterIndex, float newValue);

	//AudioProcessorEditor* createEditor();
	bool hasEditor() const {return true;}

	bool enable();
	bool disable();

	bool isSource() {return true;}

private:

	float sampleRate;
	int numChannels;
	int samplesPerBlock;

	FileInputStream* input;

	//SourceNodeEditor* sourceEditor;
	

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileReader);

};




#endif  // __FILEREADER_H_605BF4A__
