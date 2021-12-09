/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 /Users/franman/Documents/Programming/GitHub/plugin-GUI/Plugins/BasicSpikeDisplay/SpikeDisplayNode/SpikeDisplayCanvas.h

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SPIKEPLOTS_H_
#define SPIKEPLOTS_H_

#include <VisualizerWindowHeaders.h>


class SpikeDisplayCanvas;

class SpikeDisplay;
class GenericAxes;
class ProjectionAxes;
class WaveAxes;

/**

  Class for drawing the waveforms and projections of incoming spikes

*/

class SpikePlot : public Component, 
                  Button::Listener
{
public:
    
    /** Constructor*/
    SpikePlot(SpikeDisplayCanvas*, 
              int elecNum, 
              int plotType, 
              String name_);

    /** Destructor */
    virtual ~SpikePlot();

    /** Draws outline and electrode name*/
    void paint(Graphics& g);

    /** Sets bounds of sub-axes*/
    void resized();

    /** Handles an incoming spike*/
    void processSpikeObject(const Spike* s);

    /** Initializes the WaveAxes and ProjectionAxes*/
    void initAxes();

    /** Sets the aspect ratio for this plot*/
    void getBestDimensions(int*, int*);

    /** Clears the sub-axes*/
    void clear();

    /** Determines whether spike waveforms are inverted*/
    void invertSpikes(bool);

    /** Responds to monitor button and range buttons*/
    void buttonClicked(Button* button);

    /** Returns the threshold for a given channel*/
    float getDisplayThresholdForChannel(int);
    
    /** Sets the threshold for a given channel*/
    void setDisplayThresholdForChannel(int axisNum, float threshold);
    void setDetectorThresholdForChannel(int, float);

    float getRangeForChannel(int);
    void setRangeForChannel(int axisNum, float range);

    //For locking the tresholds
    void registerThresholdCoordinator(SpikeThresholdCoordinator* stc);
    void setAllThresholds(float displayThreshold, float range);

    SpikeDisplayCanvas* canvas;

    int electrodeNumber;

    int nChannels;

    float minWidth;

    float aspectRatio;

private:

    int plotType;
    int nWaveAx;
    int nProjAx;

    bool limitsChanged;

    double limits[4][2];

    OwnedArray<ProjectionAxes> projectionAxes;
    OwnedArray<WaveAxes> waveAxes;
    OwnedArray<UtilityButton> rangeButtons;

    std::unique_ptr<UtilityButton> monitorButton;

    Array<float> ranges;

    void initLimits();
    void setLimitsOnAxes();
    void updateAxesPositions();

    String name;
    Font font;

    WeakReference<SpikeThresholdCoordinator> thresholdCoordinator;

};

/**

  Base class for drawing axes for spike visualization.

  @see SpikeDisplayCanvas

*/

class GenericAxes : public Component
{
public:

    /** Constructor */
    GenericAxes(SpikePlotType type);

    /** Destructor */
    virtual ~GenericAxes();

    /** Called when a new spike is received*/
    virtual bool updateSpikeData(const Spike* s) = 0;

    /** Get/set X and Y limits*/
    void setXLims(double xmin, double xmax);
    void getXLims(double* xmin, double* xmax);
    void setYLims(double ymin, double ymax);
    void getYLims(double* ymin, double* ymax);

    /** Helper function for rounding integers*/
    int roundUp(int, int);

    /** Helper function for creating units labels*/
    void makeLabel(int val, int gain, bool convert, char* s);

protected:
    double xlims[2];
    double ylims[2];

    bool gotFirstSpike;

    const SpikePlotType type;

    Font font;

    double ad16ToUv(int x, int gain);

};


/**

  Class for drawing spike waveforms.

*/

class WaveAxes : public GenericAxes
{
public:

    /** Constructor*/
    WaveAxes(int channel);

    /** Destructor*/
    ~WaveAxes() {}

    /** Adds a new spike*/
    bool updateSpikeData(const Spike* s);

    /** Checks whether a spike is above threshold*/
    bool checkThreshold(const Spike* spike);

    /** Draws the component (calls plotSpike)*/
    void paint(Graphics& g);

    /** Draws a single spike */
    void plotSpike(const Spike* s, Graphics& g);

    /** Removes spikes that have been previously drawn*/
    void clear();

    void mouseMove(const MouseEvent& event);
    void mouseExit(const MouseEvent& event);
    void mouseDrag(const MouseEvent& event);

    void setRange(float);

    float getRange()
    {
        return range;
    }

    float getDisplayThreshold();
    void setDetectorThreshold(float);

    //For locking the thresholds
    void registerThresholdCoordinator(SpikeThresholdCoordinator* stc);
    void setDisplayThreshold(float threshold);

    void invertSpikes(bool shouldInvert)
    {
        spikesInverted = shouldInvert;
        repaint();
    }

private:

    Colour waveColour;
    Colour thresholdColour;
    Colour gridColour;

    bool drawGrid;

    float displayThresholdLevel;
    float detectorThresholdLevel;

    void drawWaveformGrid(Graphics& g);

    void drawThresholdSlider(Graphics& g);

    int spikesReceivedSinceLastRedraw;

    Font font;

   OwnedArray<Spike> spikeBuffer;

    int spikeIndex;
    int bufferSize;

    float range;

    bool isOverThresholdSlider;
    bool isDraggingThresholdSlider;

    MouseCursor::StandardCursorType cursorType;
    SpikeThresholdCoordinator* thresholdCoordinator;

    bool spikesInverted;

};


/**

  Class for drawing the peak projections of spike waveforms.

*/

class ProjectionAxes : public GenericAxes
{
public:

    /** Constructor */
    ProjectionAxes(Projection proj);

    /** Destructor*/
    ~ProjectionAxes() { }

    /** Called when a new spike is received */
    bool updateSpikeData(const Spike* s);

    /** Displays the projection image*/
    void paint(Graphics& g);

    /** Removes the projection image*/
    void clear();

    void setRange(float, float);

    static void n2ProjIdx(Projection proj, int* p1, int* p2);

    Projection getProjection() { return proj; }

private:

    Projection proj;

    void updateProjectionImage(float, float, float, Colour);

    void calcWaveformPeakIdx(const Spike*, int, int, int*, int*);

    int ampDim1, ampDim2;

    Image projectionImage;

    Colour pointColour;
    Colour gridColour;

    int imageDim;

    int rangeX;
    int rangeY;

    int spikesReceivedSinceLastRedraw;

};


#endif  // SPIKEPLOTS_H_
