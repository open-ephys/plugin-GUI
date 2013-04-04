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

#ifndef SPIKEDISPLAYCANVAS_H_
#define SPIKEDISPLAYCANVAS_H_

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../SpikeDisplayNode.h"
//#include "SpikePlotting/SpikePlot.h"
#include "SpikeObject.h"

#include "Visualizer.h"
#include <vector>

#define MAX_NUMBER_OF_SPIKE_SOURCES = 128;

class SpikeDisplayNode;

class SpikeDisplay;
class SpikePlot;

/**
  
  Displays spike waveforms and projections.

  @see SpikeDisplayNode, SpikeDisplayEditor, Visualizer

*/

class SpikeDisplayCanvas : public Visualizer

{
public: 
	SpikeDisplayCanvas(SpikeDisplayNode* n);
	~SpikeDisplayCanvas();

	void paint(Graphics& g);

	void refresh();

	void processSpikeEvents();

	void beginAnimation();
	void endAnimation();

	void refreshState();

	void setParameter(int, float) {}
	void setParameter(int, int, int, float){}

	void update();

	void resized();

private:

	SpikeDisplayNode* processor;
	MidiBuffer* spikeBuffer;

	ScopedPointer<SpikeDisplay> spikeDisplay;
	ScopedPointer<Viewport> viewport;

	bool newSpike;
	SpikeObject spike;

	int scrollBarThickness;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDisplayCanvas);
	
};

class SpikeDisplay : public Component
{
public:
	SpikeDisplay(SpikeDisplayCanvas*, Viewport*);
	~SpikeDisplay();

	void addSpikePlot(int numChannels);

	void paint(Graphics& g);

	void resized();

	void mouseDown(const MouseEvent& event);

	void plotSpike(const SpikeObject& spike);


	int getTotalHeight() {return totalHeight;}

private:

	//void computeColumnLayout();
	//void initializeSpikePlots();
	//void repositionSpikePlots();

	int numColumns;

	int totalHeight;

	SpikeDisplayCanvas* canvas;
	Viewport* viewport;

	OwnedArray<SpikePlot> spikePlots;

	int maxWidth, maxHeight, minWidth, minHeight;

};

class SpikePlot : public Component
{
public:
	SpikePlot(SpikeDisplayCanvas*, int elecNum, int numChans);
	~SpikePlot();

	void paint(Graphics& g);

	void select();
	void deselect();

	void resized();

private:

	SpikeDisplayCanvas* canvas;

	bool isSelected;

	int electrodeNumber;

	int numChannels;

};


#endif  // SPIKEDISPLAYCANVAS_H_
