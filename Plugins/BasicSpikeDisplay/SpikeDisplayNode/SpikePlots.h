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
class SpikeThresholdCoordinator;

class SpikeDisplay;
class GenericAxes;
class ProjectionAxes;
class WaveAxes;

enum WaveAxesSubPlots {
    WAVE1 = 0,
    WAVE2 = 1,
    WAVE3 = 2,
    WAVE4 = 3
};

enum Projection {
    PROJ1x2 = 4,
    PROJ1x3 = 5,
    PROJ1x4 = 6,
    PROJ2x3 = 7,
    PROJ2x4 = 8,
    PROJ3x4 = 9
};

enum SpikePlotType {
    WAVE_AXES,
    PROJECTION_AXES
};

#define TETRODE_PLOT 1004
#define STEREO_PLOT  1002
#define SINGLE_PLOT  1001

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
              String name_,
              std::string identifier_);

    /** Destructor */
    virtual ~SpikePlot();

    /** Set unique spike plot identifier */
    void setId(std::string id);

    /** Draws outline and electrode name*/
    void paint(Graphics& g);

    /** Sets bounds of sub-axes*/
    void resized();

    /** Plots latest spikes in buffer*/
    void refresh();

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

    /** Returns the range for a given channel*/
    float getRangeForChannel(int);

    /** Sets the range for a given channel*/
    void setRangeForChannel(int axisNum, float range);

    /**Returns the monitor state for this electrode */
    bool getMonitorState();

    /** Sets the monitor state for this electrode */
    void setMonitorState(bool);

    //For locking the tresholds
    void registerThresholdCoordinator(SpikeThresholdCoordinator* stc);
    void setAllThresholds(float displayThreshold, float range);

    /** Clears audio monitor active state */
    void resetAudioMonitorState();

    SpikeDisplayCanvas* canvas;

    void addSpikeToBuffer(const Spike* spike);

    int electrodeNumber;

    int nChannels;

    float minWidth;

    float aspectRatio;

private:

    std::string identifier;

    int plotType;
    int nWaveAx;
    int nProjAx;

    const int bufferSize = 5;
    int spikesInBuffer;

    OwnedArray<Spike> mostRecentSpikes;

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

    CriticalSection spikeArrayLock;

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
    GenericAxes(SpikeDisplayCanvas* canvas, SpikePlotType type);

    /** Destructor */
    virtual ~GenericAxes() { }

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

    SpikeDisplayCanvas* canvas;

protected:
    double xlims[2];
    double ylims[2];

    bool gotFirstSpike;

    const SpikePlotType type;

    Font font;

    Array<Colour> colours;

    double ad16ToUv(int x, int gain);

};


/**

  Class for drawing spike waveforms.

*/

class WaveAxes : public GenericAxes
{
public:

    /** Constructor*/
    WaveAxes(SpikeDisplayCanvas* canvas, int electrodeIndex, int channel, std::string identifier);

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

    std::string identifier;

    Colour waveColour;
    Colour thresholdColour;
    Colour gridColour;
    
    int electrodeIndex;
    int channel;

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
    ProjectionAxes(SpikeDisplayCanvas* canvas, Projection proj);

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
