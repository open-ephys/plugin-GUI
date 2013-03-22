/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

class LfpTimescale;
class LfpDisplay;
class LfpChannelDisplay;

/**

  Displays multiple channels of continuous data.

  @see LfpDisplayNode, LfpDisplayEditor

*/

class LfpDisplayCanvas : public Visualizer

{
public: 
	LfpDisplayCanvas(LfpDisplayNode* n);
	~LfpDisplayCanvas();

	//void paintCanvas(Graphics& g);

	void beginAnimation();
	void endAnimation();

	void refreshState();

	void update();

	void setParameter(int, float);
	void setParameter(int, int, int, float){}

	void paint(Graphics& g);

private:

	float sampleRate;
	float timebase;
	float displayGain;
	float timeOffset;

	static const int MAX_N_CHAN = 256;  // maximum number of channels
	static const int MAX_N_SAMP = 5000; // maximum display size in pixels
	float waves[MAX_N_SAMP][MAX_N_SAMP*2]; // we need an x and y point for each sample

	LfpDisplayNode* processor;
	AudioSampleBuffer* displayBuffer;
	MidiBuffer* eventBuffer;

	void refreshScreenBuffer();
	void updateScreenBuffer();
	int screenBufferIndex;
	int displayBufferIndex;
	int displayBufferSize;

	int nChans;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfpDisplayCanvas);
	
};

class LfpTimescale : public Component
{



};

class LfpDisplay : public Component
{



};

class LfpChannelDisplay : public Component
{


};


#endif  // __LFPDISPLAYCANVAS_H_B711873A__
 