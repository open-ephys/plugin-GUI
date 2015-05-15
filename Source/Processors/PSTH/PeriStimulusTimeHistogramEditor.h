/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2015 Open Ephys

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

#ifndef __PSTHEDITOR_H
#define __PSTHEDITOR_H


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Visualization/MatlabLikePlot.h"
#include "../PSTH/PeriStimulusTimeHistogramNode.h"
#include "../Editors/GenericEditor.h"
#include "../../UI/UIComponent.h"
#include "../../UI/DataViewport.h"
#include "../Visualization/DataWindow.h"
#include "../Editors/VisualizerEditor.h"

class Condition;

class PeriStimulusTimeHistogramNode;
class PeriStimulusTimeHistogramDisplay;
class TrialCircularBuffer;
class MatlabLikePlot;

struct zoom
{
    float xmin,xmax,ymin,ymax;
};


enum xyPlotTypes
{
    SPIKE_PLOT = 0,
    LFP_PLOT = 1,
    EYE_PLOT = 2
};

class PeriStimulusTimeHistogramCanvas;
class GenericPlot;



// this component holds all the individual PSTH plots
class PeriStimulusTimeHistogramDisplay : public Component
{
public:
    PeriStimulusTimeHistogramDisplay(PeriStimulusTimeHistogramNode* n, Viewport* p, PeriStimulusTimeHistogramCanvas* c);
    ~PeriStimulusTimeHistogramDisplay();

    void setAutoRescale(bool state);
    void resized();

    std::vector<GenericPlot*> psthPlots;
    void paint(Graphics& g);
    void refresh();
    void focusOnPlot(int plotIndex);

    PeriStimulusTimeHistogramNode* processor;
    Viewport* viewport;
    PeriStimulusTimeHistogramCanvas* canvas;

    juce::Font font;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PeriStimulusTimeHistogramDisplay);

};


class ProcessorListItem;
class UIComponent;

class ConditionList : public Component,
    public Button::Listener

{
public:

    ConditionList(PeriStimulusTimeHistogramNode* n, Viewport* p, PeriStimulusTimeHistogramCanvas* c);
    ~ConditionList();

    /** Draws the ConditionList. */
    void paint(Graphics& g);
    void buttonClicked(Button* btn);
    void updateConditionButtons();

private:
    PeriStimulusTimeHistogramNode* processor;
    Viewport* viewport;
    PeriStimulusTimeHistogramCanvas* canvas;

    ScopedPointer<ColorButton> titleButton;
    ScopedPointer<ColorButton> allButton,noneButton;
    OwnedArray<ColorButton> conditionButtons;
    /** The main method for drawing the ProcessorList.*/
    //   void drawItems(Graphics& g);

    /** Called when a mouse click begins within the boundaries of the ProcessorList.*/
    //void mouseDown(const MouseEvent& e);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConditionList);

};


class PeriStimulusTimeHistogramCanvas: public Visualizer, public Button::Listener
{
public:
    PeriStimulusTimeHistogramCanvas(PeriStimulusTimeHistogramNode* n);
    ~PeriStimulusTimeHistogramCanvas();

    void paint(Graphics& g);

    void refresh();

    void beginAnimation();
    void endAnimation();

    void refreshState();
    void update();

    void resized();
    void buttonClicked(Button* button);

    void setRasterMode(bool rasterModeActive);
    void setLFPvisibility(bool visible);
    void setSpikesVisibility(bool visible);
    void setSmoothPSTH(bool smooth);
    void setSmoothing(float _gaussianStandardDeviationMS, bool state);
    void setAutoRescale(bool state);
    void setCompactView(bool compact);
    void setMatchRange(bool on);
    bool getMatchRange();
    void setParameter(int, float) {}
    void setParameter(int, int, int, float) {}

    void setRange(double xmin, double xmax, double ymin, double ymax, xyPlotTypes plotType);

    void startRecording() { } // unused
    void stopRecording() { } // unused

    int numElectrodes;
    int maxUnitsPerElectrode;
    int heightPerElectrodePix;
    int widthPerUnit;
    bool updateNeeded;
    int screenHeight, screenWidth;

private:
    int conditionWidth;

    bool showLFP, showSpikes, smoothPlots, autoRescale,compactView, matchRange, inFocusedMode,rasterMode;
    PeriStimulusTimeHistogramNode* processor;
    ScopedPointer<Viewport> viewport, conditionsViewport;
    ScopedPointer<PeriStimulusTimeHistogramDisplay> psthDisplay;
    ScopedPointer<ConditionList> conditionsList;
    ScopedPointer<UtilityButton> visualizationButton, clearAllButton,zoomButton,panButton,resetAxesButton;
    float gaussianStandardDeviationMS;
    int numRows,numCols;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PeriStimulusTimeHistogramCanvas);

};


class PeriStimulusTimeHistogramEditor : public VisualizerEditor,
    public ComboBox::Listener
{
public:
    PeriStimulusTimeHistogramEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~PeriStimulusTimeHistogramEditor();
    Visualizer* createNewCanvas();
    void comboBoxChanged(ComboBox* comboBox);
    void updateCanvas();
    void buttonEvent(Button* button);
    bool showSortedUnits,showLFP,showCompactView,showSmooth,showAutoRescale,showMatchRange,showRasters;
    int TTLchannelTrialAlignment;
    int smoothingMS;

    void saveVisualizerParameters(XmlElement* xml);
    void loadVisualizerParameters(XmlElement* xml);
    void visualizationMenu();
private:
    PeriStimulusTimeHistogramCanvas* periStimulusTimeHistogramCanvas;
    Font font;
    ScopedPointer<ComboBox> hardwareTrialAlignment;
    ScopedPointer<UtilityButton> visibleConditions, saveOptions, clearDisplay,visualizationOptions ;
    ScopedPointer<Label> hardwareTrigger;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PeriStimulusTimeHistogramEditor);

};


class GenericPlot : public Component
{
public:
    GenericPlot(String name,	PeriStimulusTimeHistogramDisplay* dsp, int plotID_, xyPlotTypes plotType,
                TrialCircularBuffer* tcb_, int electrodeID_, int subID_, int row_, int col_, bool _rasterMode, bool inPanMode);
    void resized();
    void paint(Graphics& g);
    int getRow()
    {
        return row;
    }
    int getCol()
    {
        return col;
    }
    int getPlotID()
    {
        return plotID;
    }
    bool isFullScreen()
    {
        return fullScreenMode;
    }
    void toggleFullScreen(bool state)
    {
        fullScreenMode = state;
    }
    void setSmoothState(bool state);
    void setAutoRescale(bool state);
    void buildSmoothKernel(float gaussianStandardDeviationMS);
    xyPlotTypes getPlotType();
    void setMode(DrawComponentMode mode);

    void setXRange(double xmin, double xmax);
    void setYRange(double ymin,double ymax);

    void handleEventFromMatlabLikePlot(String event);
    void resetAxes();
private:
    void paintSpikeRaster(Graphics& g);
    void paintSpikes(Graphics& g);
    void paintLFPraster(Graphics& g);
    void paintLFP(Graphics& g);

    ScopedPointer<MatlabLikePlot> mlp;
    PeriStimulusTimeHistogramDisplay* display;
    TrialCircularBuffer* tcb;

    int plotID;
    xyPlotTypes plotType;
    int electrodeID;
    int subID;
    int row, col;
    bool rasterMode;
    bool fullScreenMode;
    bool smoothPlot;
    bool autoRescale;
    bool inPanMode;
    float guassianStandardDeviationMS;
    String plotName;
    std::vector<float> smoothKernel;
};




#endif  // PSTHEDITOR
