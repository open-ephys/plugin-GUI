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
#ifndef __LFPDISPLAYCANVAS_H_B711873A__
#define __LFPDISPLAYCANVAS_H_B711873A__

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../LfpDisplayNode.h"
#include "Visualizer.h"

class LfpDisplayNode;

/**

  Displays multiple channels of continuous data.

  @see LfpDisplayNode, LfpDisplayEditor

*/

class LfpDisplayCanvas : public Visualizer

{
public: 
	LfpDisplayCanvas(LfpDisplayNode* n);
	~LfpDisplayCanvas();
	void newOpenGLContextCreated();
	void renderOpenGL();

	void beginAnimation();
	void endAnimation();

	void refreshState();

	void update();

	void setParameter(int, float);
	void setParameter(int, int, int, float){}

	int getHeaderHeight() {return headerHeight;}
	int getFooterHeight() {return footerHeight;}

	const MouseCursor getMouseCursor();


private:

	int xBuffer, yBuffer;

	float sampleRate;
	float timebase;
	float displayGain;
	float timeOffset;

	static const int MAX_N_CHAN = 128;
	static const int MAX_N_SAMP = 3000;
	GLfloat waves[MAX_N_SAMP][MAX_N_SAMP*2]; // we need an x and y point for each sample

	LfpDisplayNode* processor;
	AudioSampleBuffer* displayBuffer;
	MidiBuffer* eventBuffer;

	void setViewport(int chan);
	void setInfoViewport(int chan);
	void drawBorder(bool isSelected);
	void drawChannelInfo(int chan, bool isSelected);
	void drawWaveform(int chan, bool isSelected);
	void drawEvents();
	void drawProgressBar();
	void drawTimeline();

	bool checkBounds(int chan);

	void refreshScreenBuffer();
	void updateScreenBuffer();
	int screenBufferIndex;
	int displayBufferIndex;
	int displayBufferSize;

	int nChans, plotHeight, totalHeight;
	int headerHeight, footerHeight;
	int interplotDistance;
	int plotOverlap;
	int selectedChan;

	MouseCursor::StandardCursorType cursorType;

	int getTotalHeight();

	 void canvasWasResized();
	 void mouseDownInCanvas(const MouseEvent& e);
	 void mouseDragInCanvas(const MouseEvent& e);
	 void mouseMoveInCanvas(const MouseEvent& e);
	// void mouseUp(const MouseEvent& e);
	// void mouseWheelMove(const MouseEvent&, float, float);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfpDisplayCanvas);
	
};



#endif  // __LFPDISPLAYCANVAS_H_B711873A__
 