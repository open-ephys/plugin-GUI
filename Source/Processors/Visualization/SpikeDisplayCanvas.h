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

#ifndef SPIKEDISPLAYCANVAS_H_
#define SPIKEDISPLAYCANVAS_H_

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../SpikeDisplayNode.h"
#include "SpikePlotting/ElectrodePlot.h"
#include "SpikePlotting/StereotrodePlot.h"
#include "SpikePlotting/TetrodePlot.h"
#include "SpikeObject.h"

#include "Visualizer.h"
#include <vector>

 /**
  
  Displays spike waveforms and projections.

  @see SpikeDisplayNode, SpikeDisplayEditor, Visualizer

*/

#define MAX_NUMBER_OF_SPIKE_SOURCES = 128;

class SpikeDisplayNode;

class SpikeDisplayCanvas : public Visualizer

{
public: 
	SpikeDisplayCanvas(SpikeDisplayNode* n);
	~SpikeDisplayCanvas();
	void newOpenGLContextCreated();
	void renderOpenGL();

	void processSpikeEvents();

	void beginAnimation();
	void endAnimation();

	void refreshState();

	void update();

	void setParameter(int, float);
	void setParameter(int, int, int, float);

	void panPlot(int, int, bool);
	void zoomPlot(int, int, bool);

private:


	MidiBuffer* spikeBuffer;

	int xBuffer, yBuffer;

	bool plotsInitialized;

	bool newSpike;
	SpikeObject spike;
	SpikeDisplayNode* processor;

	Array<BaseUIElement*> plots;

	// std::vector<StereotrodePlot> STplots;
	// std::vector<TetrodePlot> TTplots;
	// std::vector<ElectrodePlot> SEplots;

	Array<int> numChannelsPerPlot;

	int totalScrollPix;
	// AudioSampleBuffer* displayBuffer;
	// ScopedPointer<AudioSampleBuffer> screenBuffer;
	// MidiBuffer* eventBuffer;

	// void setViewport(int chan);
	// void drawBorder(bool isSelected);
	// void drawChannelInfo(int chan, bool isSelected);
	// void drawWaveform(int chan, bool isSelected);

	void drawPlotTitle(int chan);
	//void drawTicks();

	// bool checkBounds(int chan);

	// void updateScreenBuffer();
	// int screenBufferIndex;
	// int displayBufferIndex;
	// int displayBufferSize;

	int totalHeight;
	// int selectedChan;

	int getTotalHeight();

	int nPlots;
	int nCols;
	int nChannels[MAX_NUMBER_OF_SPIKE_CHANNELS];

	void initializeSpikePlots();
	void repositionSpikePlots();

	void disablePointSmoothing();
	void canvasWasResized();
	void mouseDownInCanvas(const MouseEvent& e);
	//void mouseDragInCanvas(const MouseEvent& e);
	//void mouseMoveInCanvas(const MouseEvent& e);
	void mouseUpInCanvas(const MouseEvent& e);
	void mouseWheelMoveInCanvas(const MouseEvent&, float, float);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDisplayCanvas);
	
};



#endif  // SPIKEDISPLAYCANVAS_H_
