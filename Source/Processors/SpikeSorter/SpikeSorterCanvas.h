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

#ifndef SPIKESORTERCANVAS_H_
#define SPIKESORTERCANVAS_H_

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "SpikeSorter.h"
#include "../Visualization/SpikeObject.h"

#include "../Visualization/Visualizer.h"
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

class SpikeHistogramPlot;
class SpikeThresholdDisplay;
class SpikeDisplayNode;
class SpikePlot;
class SpikeDisplay;
class GenericAxes;
class ProjectionAxes;
class WaveAxes;
class SpikePlot;
class RecordNode;

/**

  Displays spike waveforms and projections for Spike Sorter

  @see SpikeDisplayNode, SpikeDisplayEditor, Visualizer

*/

class SpikeSorterCanvas : public Visualizer, public Button::Listener

{
public:
    SpikeSorterCanvas(SpikeSorter* n);
    ~SpikeSorterCanvas();

    void paint(Graphics& g);

    void refresh();

    void processSpikeEvents();

    void beginAnimation();
    void endAnimation();

    void refreshState();

    void setParameter(int, float) {}
    void setParameter(int, int, int, float) {}

    void update();

    void resized();

    bool keyPressed(const KeyPress& key);

    void buttonClicked(Button* button);

    void startRecording() { } // unused
    void stopRecording() { } // unused

    SpikeSorter* processor;

    ScopedPointer<UtilityButton> addPolygonUnitButton,
                  addUnitButton, delUnitButton, addBoxButton, delBoxButton, rePCAButton,nextElectrode,prevElectrode,newIDbuttons,deleteAllUnits;

private:
    void removeUnitOrBox();
    ScopedPointer<SpikeThresholdDisplay> spikeDisplay;
    ScopedPointer<Viewport> viewport;

    bool inDrawingPolygonMode;
    bool newSpike;
    SpikeObject spike;
    Electrode* electrode;
    int scrollBarThickness;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeSorterCanvas);

};


class SpikeThresholdDisplay : public Component
{
public:
    SpikeThresholdDisplay(SpikeSorter*, SpikeSorterCanvas*, Viewport*);

    ~SpikeThresholdDisplay();

    void removePlots();
    void clear();
    SpikeHistogramPlot* addSpikePlot(int numChannels, int electrodeNum, String name);

    void paint(Graphics& g);

    void resized();
    void setPolygonMode(bool on);
    void mouseDown(const juce::MouseEvent& event);

    void plotSpike(const SpikeObject& spike, int electrodeNum);

    int getTotalHeight()
    {
        return totalHeight;
    }

private:
    int numColumns;
    int totalHeight;

    SpikeSorter* processor;
    SpikeSorterCanvas* canvas;
    Viewport* viewport;

    OwnedArray<SpikeHistogramPlot> spikePlots;


};



class UnitWaveformAxes : public Component
{
public:
    UnitWaveformAxes();

};

class GenericDrawAxes : public Component
{
public:

    GenericDrawAxes(int t);

    virtual ~GenericDrawAxes();

    virtual bool updateSpikeData(const SpikeObject& s);

    void setXLims(double xmin, double xmax);
    void getXLims(double* xmin, double* xmax);
    void setYLims(double ymin, double ymax);
    void getYLims(double* ymin, double* ymax);

    void setType(int type);
    int getType();

    virtual void paint(Graphics& g) = 0;

    int roundUp(int, int);
    void makeLabel(int val, int gain, bool convert, char* s);

protected:
    double xlims[2];
    double ylims[2];

    SpikeObject s;

    bool gotFirstSpike;

    int type;

    Font font;

    double ad16ToUv(int x, int gain);

};

class WaveformAxes : public GenericDrawAxes
{
public:
    WaveformAxes(SpikeHistogramPlot* plt, SpikeSorter* p, int electrodeID_, int channel);
    ~WaveformAxes() {}


    bool updateSpikeData(const SpikeObject& s);
    bool checkThreshold(const SpikeObject& spike);

    void setSignalFlip(bool state);
    void paint(Graphics& g);
    void isOverUnitBox(float x, float y, int& UnitID, int& BoxID, String& where) ;

    void plotSpike(const SpikeObject& s, Graphics& g);
    void drawBoxes(Graphics& g);

    void clear();
    int findUnitIndexByID(int ID);
    void mouseMove(const MouseEvent& event);
    void mouseExit(const MouseEvent& event);
    void mouseDown(const MouseEvent& event);
    void mouseDrag(const MouseEvent& event);
    void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel);
    void mouseUp(const MouseEvent& event);

    void setRange(float);
    float getRange()
    {
        return range;
    }

    float getDisplayThreshold();
    void setDetectorThreshold(float);

    //MouseCursor getMouseCursor();
    void updateUnits(std::vector<BoxUnit> _units);

    //	int selectedUnit, selectedBox;

private:
    int electrodeID;
    bool signalFlipped;
    bool bDragging ;
    Colour waveColour;
    Colour thresholdColour;
    Colour gridColour;
    int channel;
    bool drawGrid;

    float displayThresholdLevel;
    float detectorThresholdLevel;

    void drawWaveformGrid(Graphics& g);

    void drawThresholdSlider(Graphics& g);

    int spikesReceivedSinceLastRedraw;

    Font font;
    float mouseDownX, mouseDownY;
    float mouseOffsetX,mouseOffsetY;
    Array<SpikeObject> spikeBuffer;

    int spikeIndex;
    int bufferSize;

    float range;

    bool isOverThresholdSlider;
    bool isDraggingThresholdSlider;
    int isOverUnit,isOverBox;
    String strOverWhere;

    std::vector<BoxUnit> units;
    SpikeSorter* processor;
    SpikeHistogramPlot* spikeHistogramPlot;
    MouseCursor::StandardCursorType cursorType;

};



class PCAProjectionAxes : public GenericDrawAxes,  Button::Listener
{
public:
    PCAProjectionAxes(SpikeSorter* p);
    ~PCAProjectionAxes() {}

    void setPCARange(float p1min, float p2min, float p1max, float p2max);
    bool updateSpikeData(const SpikeObject& s);
    void resized();
    void paint(Graphics& g);
    void setPolygonDrawingMode(bool on);
    void clear();
    void mouseDown(const juce::MouseEvent& event);
    void mouseUp(const juce::MouseEvent& event);
    void mouseMove(const juce::MouseEvent& event);
    void mouseDrag(const juce::MouseEvent& event);
    bool keyPressed(const KeyPress& key);
    void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel);
    void redraw(bool subsample);

    void updateUnits(std::vector<PCAUnit> _units);

    void buttonClicked(Button* button);

    void drawUnit(Graphics& g, PCAUnit unit);
    void rangeDown();
    void rangeUp();

private:
    float prevx,prevy;
    bool inPolygonDrawingMode;
    void drawProjectedSpike(SpikeObject s);

    bool rangeSet;
    SpikeSorter* processor;
    void updateProjectionImage(uint16_t, uint16_t, uint16_t, const uint8_t* col);
    void updateRange(const SpikeObject& s);
    ScopedPointer<UtilityButton> rangeDownButton, rangeUpButton;

    Array<SpikeObject> spikeBuffer;
    int bufferSize;
    int spikeIndex;
    bool updateProcessor;
    void calcWaveformPeakIdx(const SpikeObject&, int, int, int*, int*);

    Image projectionImage;

    Colour pointColour;
    Colour gridColour;

    int imageDim;

    int rangeX;
    int rangeY;

    int spikesReceivedSinceLastRedraw;

    float pcaMin[2],pcaMax[2];
    std::list<PointD> drawnPolygon;

    std::vector<PCAUnit> units;
    int isOverUnit;
    PCAUnit drawnUnit;

    bool redrawSpikes;
};


class SpikeHistogramPlot : public Component, Button::Listener
{
public:
    SpikeHistogramPlot(SpikeSorter*, SpikeSorterCanvas*, int electrodeID, int plotType, String name_);
    virtual ~SpikeHistogramPlot();

    void paint(Graphics& g);
    void resized();
    void setFlipSignal(bool state);

    void select();
    void deselect();

    void setPolygonDrawingMode(bool on);
    void setPCARange(float p1min, float p2min, float p1max, float p2max);
    void modifyRange(int index,bool up);
    void updateUnitsFromProcessor();
    void processSpikeObject(const SpikeObject& s);

    SpikeSorterCanvas* canvas;

    bool isSelected;

    int electrodeNumber;

    void getSelectedUnitAndBox(int& unitID, int& boxID);
    void setSelectedUnitAndBox(int unitID, int boxID);
    int nChannels;

    void initAxes(std::vector<float> scales);
    void getBestDimensions(int*, int*);

    void clear();

    float minWidth;
    float aspectRatio;

    void buttonClicked(Button* button);

    float getDisplayThresholdForChannel(int);
    void setDisplayThresholdForChannel(int channelNum, float thres);
    //void setDetectorThresholdForChannel(int, float);

private:
    void modifyRange(std::vector<float> values);

    int plotType;
    int nWaveAx;
    int nProjAx;
    int electrodeID;
    bool limitsChanged;

    double limits[MAX_N_CHAN][2];

    std::vector<BoxUnit> boxUnits;
    std::vector<PCAUnit> pcaUnits;
    SpikeSorter* processor;
    OwnedArray<PCAProjectionAxes> pAxes;
    OwnedArray<WaveformAxes> wAxes;
    OwnedArray<UtilityButton> rangeButtons;
    Array<float> ranges;

    void initLimits();
    void setLimitsOnAxes();
    void updateAxesPositions();


    String name;
    CriticalSection mut;
    Font font;



};

#endif  // SPIKESORTERCANVAS_H_
