/*
  ==============================================================================

    LfpDisplayNode.h
    Created: 8 Feb 2012 12:31:32pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __LFPDISPLAYNODE_H_D969A379__
#define __LFPDISPLAYNODE_H_D969A379__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "Editors/LfpDisplayEditor.h"
#include "GenericProcessor.h"
//#include "Visualization/OpenGLCanvas.h"

class DataViewport;

class LfpDisplayNode :  public GenericProcessor
						///public OpenGLCanvas,
					  

{
public:

	LfpDisplayNode();
	~LfpDisplayNode();

	// void newOpenGLContextCreated();
	// void renderOpenGL();
	// int getTotalHeight();

	AudioProcessorEditor* createEditor();

	bool isSink() {return true;}

	void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples);

	void setParameter(int, float);

	void setNumInputs(int inputs);
	void setSampleRate(float r);

	bool enable();
	bool disable();

	void prepareToPlay(double, int);

	AudioSampleBuffer* getDisplayBufferAddress() {return displayBuffer;}
	int getDisplayBufferIndex() {return displayBufferIndex;}
	ReadWriteLock* getLock() {return lock;}

	bool isVisible;

	ReadWriteLock* lock;

private:

	DataViewport* dataViewport;

	AudioSampleBuffer* displayBuffer;
	//AudioSampleBuffer* screenBuffer;
	MidiBuffer* eventBuffer;

	int displayBufferIndex;//, screenBufferIndex;

	int repaintInterval, repaintCounter;

	// void setViewport(int chan);
	// void drawBorder(bool isSelected);
	// void drawChannelInfo(int chan, bool isSelected);
	// void drawWaveform(int chan, bool isSelected);

	// void drawTicks();

	// bool checkBounds(int chan);

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
