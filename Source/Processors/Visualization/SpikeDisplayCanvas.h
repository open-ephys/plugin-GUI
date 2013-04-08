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
#include "SpikeObject.h"

#include "Visualizer.h"
#include <vector>

#define WAVE1 0
#define WAVE2 1
#define WAVE3 2
#define WAVE4 3
#define PROJ1x2 4
#define PROJ1x3 5
#define PROJ1x4 6
#define PROJ2x3 7
#define PROJ2x4 8
#define PROJ3x4 9

#define TETRODE_PLOT 1004
#define STEREO_PLOT  1002
#define SINGLE_PLOT  1001

#define MAX_NUMBER_OF_SPIKE_SOURCES 128
#define MAX_N_CHAN 4

class SpikeDisplayNode;

class SpikeDisplay;
class GenericAxes;
class ProjectionAxes;
class WaveAxes;
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

	void clear();
	void addSpikePlot(int numChannels, int electrodeNum);

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

	// float tetrodePlotMinWidth, stereotrodePlotMinWidth, singleElectrodePlotMinWidth;
	// float tetrodePlotRatio, stereotrodePlotRatio, singleElectrodePlotRatio;

};

/**

  Class for drawing the waveforms and projections of incoming spikes.

*/

class SpikePlot : public Component
{
public:
	SpikePlot(SpikeDisplayCanvas*, int elecNum, int plotType);
	virtual ~SpikePlot();

	void paint(Graphics& g);
	void resized();

	void select();
	void deselect();

	void processSpikeObject(SpikeObject s);

	SpikeDisplayCanvas* canvas;

	bool isSelected;

	int electrodeNumber;

	int nChannels;

	void initAxes();
	void getBestDimensions(int*, int*);

	void clear();
	void zoom(int, bool);
	void pan(int, bool);

	float minWidth;
	float aspectRatio;

private:

	
	int plotType;
	int nWaveAx;
	int nProjAx;
	
	bool limitsChanged;

	double limits[MAX_N_CHAN][2];

	OwnedArray<ProjectionAxes> pAxes;
	OwnedArray<WaveAxes> wAxes;

	void initLimits();
	void setLimitsOnAxes();
	void updateAxesPositions();

	void n2ProjIdx(int i, int* p1, int* p2);

	Font font;

};


// class TetrodePlot : public SpikePlot
// {
// public:
// 	TetrodePlot(SpikeDisplayCanvas*, int elecNum);
// 	~TetrodePlot() {}

// 	void resized();

// private:

// };

// class StereotrodePlot : public SpikePlot
// {
// public:
// 	StereotrodePlot(SpikeDisplayCanvas*, int elecNum);
// 	~StereotrodePlot() {}

// 	void resized();

// private:
	
// };

// class SingleElectrodePlot : public SpikePlot
// {
// public:
// 	SingleElectrodePlot(SpikeDisplayCanvas*, int elecNum);
// 	~SingleElectrodePlot() {}

// 	void resized();

// private:
	
// };


/**

  Base class for drawing axes for spike visualization.

  @see SpikeDisplayCanvas

*/

class GenericAxes : public Component
{
public:

    GenericAxes(int t);

    virtual ~GenericAxes();

    void updateSpikeData(SpikeObject s);

    void setXLims(double xmin, double xmax);
    void getXLims(double* xmin, double* xmax);
    void setYLims(double ymin, double ymax);
    void getYLims(double* ymin, double* ymax);

    void setType(int type);
    int getType();

    virtual void paint(Graphics& g) = 0;

protected:
 	double xlims[2];
    double ylims[2];

    SpikeObject s;

    bool gotFirstSpike;

    int type;

    Font font;

};



/**

  Class for drawing spike waveforms.

*/

class WaveAxes : public GenericAxes
{
public:
	WaveAxes(int channel);
	~WaveAxes() {}

	void paint(Graphics& g);

	void clear();

private:

	Colour waveColour;
	Colour thresholdColour;
	Colour gridColour;



};



/**

  Class for drawing the peak projections of spike waveforms.

*/

class ProjectionAxes : public GenericAxes
{
public:
	ProjectionAxes(int projectionNum);
	~ProjectionAxes() {}

	void paint(Graphics& g);

	void clear();

private:

	Colour pointColour;
	Colour gridColour;

};



#endif  // SPIKEDISPLAYCANVAS_H_
