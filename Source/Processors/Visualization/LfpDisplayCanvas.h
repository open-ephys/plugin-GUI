/*
  ==============================================================================

    LfpDisplayCanvas.h
    Created: 8 Feb 2012 1:06:53pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __LFPDISPLAYCANVAS_H_B711873A__
#define __LFPDISPLAYCANVAS_H_B711873A__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "OpenGLCanvas.h"
#include "../../UI/Configuration.h"
#include "../LfpDisplayNode.h"

class LfpDisplayNode;

class LfpDisplayCanvas : public OpenGLCanvas

{
public: 
	LfpDisplayCanvas(LfpDisplayNode* n);
	~LfpDisplayCanvas();
	void newOpenGLContextCreated();
	void renderOpenGL();

	void beginAnimation();
	void endAnimation();

	void updateNumInputs(int);
	void updateSampleRate(float);

	void setParameter(int, float);

private:

	ReadWriteLock* lock;

	int xBuffer, yBuffer;

	float sampleRate;
	float timebase;
	float displayGain;
	//float ratio;

	LfpDisplayNode* processor;
	AudioSampleBuffer* displayBuffer;
	ScopedPointer<AudioSampleBuffer> screenBuffer;
	MidiBuffer* eventBuffer;

	void setViewport(int chan);
	void drawBorder(bool isSelected);
	void drawChannelInfo(int chan, bool isSelected);
	void drawWaveform(int chan, bool isSelected);

	void drawTicks();

	bool checkBounds(int chan);



	void updateScreenBuffer();
	int screenBufferIndex;
	int displayBufferIndex;
	int displayBufferSize;

	int nChans, plotHeight, totalHeight;
	int selectedChan;

	int getTotalHeight();

	// void resized();
	// void mouseDown(const MouseEvent& e);
	// void mouseDrag(const MouseEvent& e);
	// void mouseMove(const MouseEvent& e);
	// void mouseUp(const MouseEvent& e);
	// void mouseWheelMove(const MouseEvent&, float, float);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfpDisplayCanvas);
	
};



#endif  // __LFPDISPLAYCANVAS_H_B711873A__
