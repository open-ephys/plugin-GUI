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

#include "PeriStimulusTimeHistogramEditor.h"
#include "../../UI/EditorViewport.h"
#include "../PSTH/TrialCircularBuffer.h"
#include "../PSTH/tictoc.h"
#include <stdio.h>


#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif


#ifndef min
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

//FileSearchPathListComponent::paintListBoxItem
PeriStimulusTimeHistogramEditor::PeriStimulusTimeHistogramEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, useDefaultParameterEditors) ,periStimulusTimeHistogramCanvas(nullptr)
{
    showSortedUnits = true;
    showLFP = true;
    showCompactView = false;
    showSmooth = true;
    showRasters = false;
    showAutoRescale = true;
    showMatchRange = false;
    TTLchannelTrialAlignment = -1;
    smoothingMS = 10;


    tabText = "PSTH";
    desiredWidth = 300;


    saveOptions = new UtilityButton("Save Options",Font("Default", 15, Font::plain));
    saveOptions->addListener(this);
    saveOptions->setColour(Label::textColourId, Colours::white);
    saveOptions->setBounds(180,30,100,20);
    addAndMakeVisible(saveOptions);


    clearDisplay = new UtilityButton("Clear all",Font("Default", 15, Font::plain));
    clearDisplay->addListener(this);
    clearDisplay->setColour(Label::textColourId, Colours::white);
    clearDisplay->setBounds(180,60,90,20);
    addAndMakeVisible(clearDisplay);


    visualizationOptions =  new UtilityButton("Visualization Options",Font("Default", 15, Font::plain));
    visualizationOptions->addListener(this);
    visualizationOptions->setColour(Label::textColourId, Colours::white);
    visualizationOptions->setBounds(10,30,160,20);
    addAndMakeVisible(visualizationOptions);

    hardwareTrigger = new Label("hardwareTrigger","TTL Trial Alignment:");
    hardwareTrigger->setFont(Font("Default", 14, Font::plain));
    hardwareTrigger->setEditable(false);
    hardwareTrigger->setJustificationType(Justification::centredLeft);
    hardwareTrigger->setBounds(10,90,120,20);
    addAndMakeVisible(hardwareTrigger);

    hardwareTrialAlignment = new ComboBox("Hardware Trial Alignment");
    hardwareTrialAlignment->setEditableText(false);
    hardwareTrialAlignment->setJustificationType(Justification::centredLeft);
    hardwareTrialAlignment->addListener(this);
    hardwareTrialAlignment->setBounds(130,90,70,20);
    hardwareTrialAlignment->addItem("Not set",1);
    for (int k=0; k<8; k++)
    {
        hardwareTrialAlignment->addItem("TTL "+String(k+1),k+2);
    }
    if (TTLchannelTrialAlignment == -1)
        hardwareTrialAlignment->setSelectedId(1, sendNotification);
    else
        hardwareTrialAlignment->setSelectedId(TTLchannelTrialAlignment+2, sendNotification);

    addAndMakeVisible(hardwareTrialAlignment);


}



void PeriStimulusTimeHistogramEditor::saveVisualizerParameters(XmlElement* xml)
{

    XmlElement* xmlNode = xml->createNewChildElement("PSTH_EDITOR");
    xmlNode->setAttribute("showSortedUnits",showSortedUnits);

    xmlNode->setAttribute("showLFP",showLFP);
    xmlNode->setAttribute("showCompactView",showCompactView);
    xmlNode->setAttribute("showSmooth",showSmooth);
    xmlNode->setAttribute("showAutoRescale",showAutoRescale);
    xmlNode->setAttribute("showRasters",showRasters);
    xmlNode->setAttribute("showMatchRange",showMatchRange);
    xmlNode->setAttribute("TTLchannelTrialAlignment",TTLchannelTrialAlignment);
    xmlNode->setAttribute("smoothingMS",smoothingMS);
}

void PeriStimulusTimeHistogramEditor::loadVisualizerParameters(XmlElement* xml)
{
    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("PSTH_EDITOR"))
        {

            showLFP = xmlNode->getBoolAttribute("showLFP");
            showCompactView = xmlNode->getBoolAttribute("showCompactView");
            showSmooth = xmlNode->getBoolAttribute("showSmooth");
            showAutoRescale = xmlNode->getBoolAttribute("showAutoRescale");
            showRasters= xmlNode->getBoolAttribute("showRasters",false);
            showMatchRange = xmlNode->getBoolAttribute("showMatchRange");
            TTLchannelTrialAlignment = xmlNode->getIntAttribute("TTLchannelTrialAlignment");
            smoothingMS = xmlNode->getIntAttribute("smoothingMS");

            if (periStimulusTimeHistogramCanvas != nullptr)
            {
                periStimulusTimeHistogramCanvas->setLFPvisibility(showLFP);
                periStimulusTimeHistogramCanvas->setSpikesVisibility(showSortedUnits);
                periStimulusTimeHistogramCanvas->setCompactView(showCompactView);
                periStimulusTimeHistogramCanvas->setAutoRescale(showAutoRescale);
                periStimulusTimeHistogramCanvas->setMatchRange(showMatchRange);
                periStimulusTimeHistogramCanvas->setSmoothing(smoothingMS,true);
                periStimulusTimeHistogramCanvas->setRasterMode(showRasters);
            }
        }
    }
}


void PeriStimulusTimeHistogramEditor::comboBoxChanged(ComboBox* comboBox)
{

    if (comboBox == hardwareTrialAlignment)
    {
        std::cout << "Setting hardware trigger alignment channel to " << comboBox->getSelectedId()-2 << std::endl;
        PeriStimulusTimeHistogramNode* processor = (PeriStimulusTimeHistogramNode*) getProcessor();
        processor->setHardwareTriggerAlignmentChannel(comboBox->getSelectedId()-2);
    }
}

void PeriStimulusTimeHistogramEditor::visualizationMenu()
{
    PeriStimulusTimeHistogramNode* processor = (PeriStimulusTimeHistogramNode*) getProcessor();

    PopupMenu m;
    m.addItem(1,"Sorted Units",true, showSortedUnits);
    m.addItem(2,"LFP",true, showLFP);
    m.addItem(3,"Compact View",true, showCompactView);

    PopupMenu smoothingSubMenu;
    int SmoothingFactors[8] = {0,1,2,5,10,20,50,100};
    for (int k=0; k<8; k++)
    {
        String s;
        if (SmoothingFactors[k] == 0)
            s = "No Smoothing";
        else
            s = String(SmoothingFactors[k]) + " ms";

        smoothingSubMenu.addItem(40+k,s,true, smoothingMS == SmoothingFactors[k]);
    }

    PopupMenu rangeTimeMenu,preRangeTimeMenu,postRangeTimeMenu;
    double rangeTimes[4] = {0.5,1,1.5,2};
    TrialCircularBufferParams params = processor->trialCircularBuffer->getParams();

    for (int k=0; k<4; k++)
    {
        String s = String(rangeTimes[k],1) + " sec";
        if (processor->trialCircularBuffer == nullptr)
        {
            preRangeTimeMenu.addItem(50+k,s,true, false);
            postRangeTimeMenu.addItem(60+k,s,true, false);
        }
        else
        {
            preRangeTimeMenu.addItem(50+k,s,true, fabs(params.preSec - rangeTimes[k])<0.01);
            postRangeTimeMenu.addItem(60+k,s,true, fabs(params.postSec - rangeTimes[k])<0.01);
        }
    }
    rangeTimeMenu.addSubMenu("pre trial", preRangeTimeMenu,true);
    rangeTimeMenu.addSubMenu("post trial", postRangeTimeMenu,true);

    m.addSubMenu("Smooth Curves", smoothingSubMenu);
    m.addItem(7,"Raster mode",true, showRasters);

    m.addSubMenu("Range", rangeTimeMenu,true);
    m.addItem(5,"Auto Rescale",true, showAutoRescale);
    m.addItem(6,"Match range",true, showMatchRange);
    m.addItem(8,"Bar Graph",false, false);
    m.addItem(9,"2D Heat map",false, false);
    const int result = m.show();
    switch (result)
    {
        case 1:
            showSortedUnits=!showSortedUnits;
            periStimulusTimeHistogramCanvas->setSpikesVisibility(showSortedUnits);
            break;
        case 2:
            showLFP=!showLFP;
            periStimulusTimeHistogramCanvas->setLFPvisibility(showLFP);
            break;
        case 3:
            showCompactView=!showCompactView;
            periStimulusTimeHistogramCanvas->setCompactView(showCompactView);
            break;
        case 5:
            showAutoRescale=!showAutoRescale;
            periStimulusTimeHistogramCanvas->setAutoRescale(showAutoRescale);
            break;
        case 6:
            showMatchRange=!showMatchRange;
            periStimulusTimeHistogramCanvas->setMatchRange(showMatchRange);
            break;
        case 7:
            showRasters = !showRasters;
            periStimulusTimeHistogramCanvas->setRasterMode(showRasters);
            break;
    }
    if (result >= 40 && result <= 47)
    {
        smoothingMS = SmoothingFactors[result-40];
        periStimulusTimeHistogramCanvas->setSmoothing(SmoothingFactors[result-40],result-40>0);
    }
    else if (result >= 50 && result <= 54)
    {
        // this will require killing
        double newPreSec = rangeTimes[result-50];
        processor->modifyTimeRange(newPreSec,params.postSec);
    }
    else if (result >= 60 && result <= 64)
    {
        double newPostSec = rangeTimes[result-60];
        processor->modifyTimeRange(params.preSec,newPostSec);
    }
}

void PeriStimulusTimeHistogramEditor::buttonEvent(Button* button)
{
    VisualizerEditor::buttonEvent(button);
    if (periStimulusTimeHistogramCanvas == nullptr)
        return;

    PeriStimulusTimeHistogramNode* processor = (PeriStimulusTimeHistogramNode*) getProcessor();

    if (button == clearDisplay)
    {
        if (processor->trialCircularBuffer != nullptr)
        {
            processor->trialCircularBuffer->clearAll();
            repaint();
        }
    }
    else if (button == visualizationOptions)
    {
        visualizationMenu();
    }
    else if (button == saveOptions)
    {

        PopupMenu m;


        /*	m.addItem(1,"TTL",true, processor->saveTTLs);
        	m.addItem(2,"Network Events",true, processor->saveNetworkEvents);
        	m.addItem(7,"Network Events [when recording is off]",true, processor->saveNetworkEventsWhenNotRecording);
        	m.addItem(3,"Eye Tracking",true, processor->saveEyeTracking);*/

        //m.addItem(4,"Sorted Spikes: TS only ",true, processor->spikeSavingMode == 1);
        m.addItem(5,"Sorted Spikes: TS+waveform",true, processor->spikeSavingMode == 2);
        m.addItem(6,"All Spikes: TS+waveform",true, processor->spikeSavingMode == 3);

        const int result = m.show();

        if (result == 1)
        {
            processor->saveTTLs = !processor->saveTTLs;
        }
        else if (result == 2)
        {
            processor->saveNetworkEvents = !processor->saveNetworkEvents;
        }
        else if (result == 3)
        {
            processor->saveEyeTracking = !processor->saveEyeTracking;
        }
        else if (result == 4)
        {
            if (processor->spikeSavingMode == 1)
                processor->spikeSavingMode = 0;
            else
                processor->spikeSavingMode = 1;
        }
        else if (result == 5)
        {
            if (processor->spikeSavingMode == 2)
                processor->spikeSavingMode = 0;
            else
                processor->spikeSavingMode = 2;
        }
        else if (result == 6)
        {
            if (processor->spikeSavingMode == 3)
                processor->spikeSavingMode = 0;
            else
                processor->spikeSavingMode = 3;
        }
        else if (result == 7)
        {
            processor->saveNetworkEventsWhenNotRecording = !processor->saveNetworkEventsWhenNotRecording;
        }

    } /*else if (button == visibleConditions)
	{


		if ( processor->trialCircularBuffer != nullptr)
		{
			processor->trialCircularBuffer ->lockConditions();
			PopupMenu m;

			for (int i = 0; i < processor->trialCircularBuffer->conditions.size(); i++)
			{
				{

					String name = processor->trialCircularBuffer->conditions[i].name;
					m.addItem(i+1, name,true,processor->trialCircularBuffer->conditions[i].visible);
				}
			}

			const int result = m.show();

			if (result > 0)
			{
				// update the visibility for all channels and units.
				PeriStimulusTimeHistogramNode* processor = (PeriStimulusTimeHistogramNode*) getProcessor();
				processor->toggleConditionVisibility(result-1);
				if (periStimulusTimeHistogramCanvas != nullptr)
				{
					periStimulusTimeHistogramCanvas->update();
				}

			}
			processor->trialCircularBuffer ->unlockConditions();
		}

	}*/



}


Visualizer* PeriStimulusTimeHistogramEditor::createNewCanvas()
{
    PeriStimulusTimeHistogramNode* processor = (PeriStimulusTimeHistogramNode*) getProcessor();
    periStimulusTimeHistogramCanvas = new PeriStimulusTimeHistogramCanvas(processor);
    //ActionListener* listener = (ActionListener*) periStimulusTimeHistogramCanvas;
    //getUIComponent()->registerAnimatedComponent(listener);
    return periStimulusTimeHistogramCanvas;
}

void PeriStimulusTimeHistogramEditor::updateCanvas()
{
    if (periStimulusTimeHistogramCanvas != nullptr)
    {
        periStimulusTimeHistogramCanvas->updateNeeded = true;
    }
}

PeriStimulusTimeHistogramEditor::~PeriStimulusTimeHistogramEditor()
{


}







/********************************/
#ifndef MAX
#define MAX(x,y)((x)>(y))?(x):(y)
#endif

#ifndef MIN
#define MIN(x,y)((x)<(y))?(x):(y)
#endif


PeriStimulusTimeHistogramCanvas::PeriStimulusTimeHistogramCanvas(PeriStimulusTimeHistogramNode* n) :
    processor(n)
{
    screenWidth = screenHeight = 0;
    conditionWidth = 200;

    inFocusedMode = false;
    showLFP = true;
    showSpikes = true;
    matchRange = false;
    smoothPlots = true;
    autoRescale = true;
    compactView = false;
    rasterMode = false;
    gaussianStandardDeviationMS = 10;
    viewport = new Viewport();
    psthDisplay = new PeriStimulusTimeHistogramDisplay(n, viewport, this);
    viewport->setViewedComponent(psthDisplay, false);
    viewport->setScrollBarsShown(true, true);
    addAndMakeVisible(viewport);


    visualizationButton = new UtilityButton("Visualization Options",Font("Default", 15, Font::plain));
    visualizationButton->addListener(this);
    addAndMakeVisible(visualizationButton);

    clearAllButton = new UtilityButton("Clear all",Font("Default", 15, Font::plain));
    clearAllButton->addListener(this);
    addAndMakeVisible(clearAllButton);

    conditionsViewport = new Viewport();
    conditionsList = new ConditionList(n, conditionsViewport, this);
    conditionsViewport->setViewedComponent(conditionsList, false);
    conditionsViewport->setScrollBarsShown(true, true);
    addAndMakeVisible(conditionsViewport);

    zoomButton = new UtilityButton("Zoom",Font("Default", 15, Font::plain));
    zoomButton->addListener(this);
    zoomButton->setColour(Label::textColourId, Colours::white);
    addAndMakeVisible(zoomButton);
    panButton = new UtilityButton("Pan",Font("Default", 15, Font::plain));
    panButton->addListener(this);
    panButton->setColour(Label::textColourId, Colours::white);
    panButton->setToggleState(true, dontSendNotification);

    addAndMakeVisible(panButton);
    resetAxesButton = new UtilityButton("Reset Axes",Font("Default", 15, Font::plain));
    resetAxesButton->addListener(this);
    resetAxesButton->setColour(Label::textColourId, Colours::white);
    addAndMakeVisible(resetAxesButton);


    resized();
    update();

}


PeriStimulusTimeHistogramCanvas::~PeriStimulusTimeHistogramCanvas()
{

}

void PeriStimulusTimeHistogramCanvas::beginAnimation()
{

    std::cout << "PeriStimulusTimeHistogramCanvas starting animation." << std::endl;
    startCallbacks();

}

void PeriStimulusTimeHistogramCanvas::buttonClicked(Button* button)
{
    if (button == visualizationButton)
    {
        PeriStimulusTimeHistogramEditor* ed = (PeriStimulusTimeHistogramEditor*) processor->getEditor();
        ed->visualizationMenu();
    }
    else if (button == clearAllButton)
    {
        processor->trialCircularBuffer->clearAll();
    }
    else if (button == zoomButton)
    {
        zoomButton->setToggleState(true, dontSendNotification);
        panButton->setToggleState(false, dontSendNotification);
        for (int k=0; k<psthDisplay->psthPlots.size(); k++)
        {
            psthDisplay->psthPlots[k]->setMode(ZOOM);
        }
    }
    else if (button == panButton)
    {
        zoomButton->setToggleState(false, dontSendNotification);
        panButton->setToggleState(true, dontSendNotification);
        for (int k=0; k<psthDisplay->psthPlots.size(); k++)
        {
            psthDisplay->psthPlots[k]->setMode(PAN);
        }
    }
    else if (button == resetAxesButton)
    {
        for (int k=0; k<psthDisplay->psthPlots.size(); k++)
        {
            psthDisplay->psthPlots[k]->resetAxes();
        }
    }
}

void PeriStimulusTimeHistogramCanvas::endAnimation()
{
    std::cout << "PeriStimulusTimeHistogramCanvas ending animation." << std::endl;

    stopCallbacks();
}

void PeriStimulusTimeHistogramCanvas::setRasterMode(bool rasterModeActive)
{
    rasterMode = rasterModeActive;
    update();
}

void PeriStimulusTimeHistogramCanvas::setLFPvisibility(bool visible)
{
    showLFP = visible;
    update();
}

void PeriStimulusTimeHistogramCanvas::setSpikesVisibility(bool visible)
{
    showSpikes = visible;
    update();
}

void PeriStimulusTimeHistogramCanvas::setSmoothing(float _gaussianStandardDeviationMS, bool smooth_enabled)
{
    gaussianStandardDeviationMS=_gaussianStandardDeviationMS;
    for (int k=0; k<	psthDisplay->psthPlots.size(); k++)
    {
        if (smooth_enabled)
        {
            psthDisplay->psthPlots[k]->buildSmoothKernel(gaussianStandardDeviationMS);
            psthDisplay->psthPlots[k]->setSmoothState(true);
        }
        else
        {
            psthDisplay->psthPlots[k]->setSmoothState(false);
        }
        psthDisplay->psthPlots[k]->repaint();
    }

}

void PeriStimulusTimeHistogramCanvas::setSmoothPSTH(bool smooth)
{
    smoothPlots = smooth;
    for (int k=0; k<	psthDisplay->psthPlots.size(); k++)
    {
        psthDisplay->psthPlots[k]->setSmoothState(smoothPlots);
        psthDisplay->psthPlots[k]->repaint();
    }

}

void PeriStimulusTimeHistogramCanvas::setCompactView(bool compact)
{
    compactView = compact;
    update();
}

void PeriStimulusTimeHistogramCanvas::setMatchRange(bool on)
{
    matchRange = on;
    //update();
}

bool PeriStimulusTimeHistogramCanvas::getMatchRange()
{
    return matchRange;
}

void PeriStimulusTimeHistogramCanvas::setAutoRescale(bool state)
{
    autoRescale = state;
    for (int k=0; k<	psthDisplay->psthPlots.size(); k++)
    {
        psthDisplay->psthPlots[k]->setAutoRescale(autoRescale);
        psthDisplay->psthPlots[k]->repaint();
    }
}

void PeriStimulusTimeHistogramCanvas::setRange(double xmin, double xmax, double ymin, double ymax, xyPlotTypes plotType)
{

    for (int k=0; k<	psthDisplay->psthPlots.size(); k++)
    {
        if (psthDisplay->psthPlots[k]->getPlotType() == plotType)
        {
            psthDisplay->psthPlots[k]->setXRange(xmin,xmax);
            psthDisplay->psthPlots[k]->setYRange(ymin,ymax);
        }
        else
        {
            psthDisplay->psthPlots[k]->setXRange(xmin,xmax);
        }
    }
}


void PeriStimulusTimeHistogramCanvas::refreshState()
{
    update();
    resized();
}


void PeriStimulusTimeHistogramCanvas::update()
{
    //std::cout << "Updating SpikeDisplayCanvas" << std::endl;
    // clear all XY plots and create new ones...
    // delete all existing plots.
    // lock psth
    bool inPanMode = panButton->getToggleState();

    heightPerElectrodePix = 300;
    widthPerUnit = 300;
    int maxUnitsPerRow = (screenWidth-conditionWidth)/ widthPerUnit;
    updateNeeded = false;
    for (int k = 0; k < psthDisplay->psthPlots.size(); k++)
    {
        delete psthDisplay->psthPlots[k];
    }
    psthDisplay->psthPlots.clear();
    if (processor->trialCircularBuffer == nullptr)
        return;

    const ScopedLock myScopedLock(processor->trialCircularBuffer->psthMutex);

    //processor->trialCircularBuffer->lockPSTH();
    numElectrodes = processor->trialCircularBuffer->getNumElectrodes();
    int maxUnitsPerElectrode = 0;
    int row = 0;
    int plotCounter = 0;
    numCols = 0;
    numRows = 0;
    int plotID = 0;
    for (int e = 0; e < numElectrodes; e++)
    {
        int offset = 0;
        bool plottedSomething = false;
        int electrodeID = processor->trialCircularBuffer->getElectrodeID(e);
        String electrodeName = processor->trialCircularBuffer->getElectrodeName(e);

        if (showLFP)
        {
            std::vector<int> channels = processor->trialCircularBuffer->getElectrodeChannels(e);
            offset = channels.size();
            for (int u=0; u<channels.size(); u++)
            {
                GenericPlot* newplot;
                if (compactView)
                {
                    String plotName = electrodeName+" Ch:"+String(1+u);
                    newplot = new GenericPlot(plotName,psthDisplay,++plotID,LFP_PLOT,processor->trialCircularBuffer,
                                              electrodeID,
                                              channels[u],
                                              plotCounter,row,rasterMode,inPanMode);

                    plotCounter++;
                    numCols++;
                    numCols = min(maxUnitsPerRow,numCols);

                    if (plotCounter >= maxUnitsPerRow)
                    {
                        plotCounter = 0;
                        row++;
                    }
                }
                else
                {
                    String plotName = electrodeName+":Ch "+String(1+u);
                    newplot = new GenericPlot(plotName,psthDisplay,++plotID,LFP_PLOT,processor->trialCircularBuffer,
                                              electrodeID,
                                              channels[u],
                                              u,row,rasterMode,inPanMode);
                    numCols = max(numCols,u);

                }
                newplot->setSmoothState(smoothPlots);
                newplot->setAutoRescale(autoRescale);
                newplot->buildSmoothKernel(gaussianStandardDeviationMS);
                psthDisplay->psthPlots.push_back(newplot);
                psthDisplay->addAndMakeVisible(newplot);
                plottedSomething = true;
            }

        }

        if (showSpikes)
        {
            int numUnits = processor->trialCircularBuffer->getNumUnitsInElectrode(e);
            maxUnitsPerElectrode = MAX(maxUnitsPerElectrode,numUnits);
            if (numUnits > 0)
            {
                for (int u=0; u<numUnits; u++)
                {
                    GenericPlot* newplot;
                    if (compactView)
                    {
                        String plotName = electrodeName+" Unit:"+String(processor->trialCircularBuffer->getUnitID(e,u));
                        newplot = new GenericPlot(plotName,psthDisplay,++plotID,SPIKE_PLOT,processor->trialCircularBuffer,
                                                  electrodeID,
                                                  processor->trialCircularBuffer->getUnitID(e,u),
                                                  plotCounter,row,rasterMode,inPanMode);
                        plotCounter++;
                        numCols++;
                        numCols = min(maxUnitsPerRow,numCols);

                        if (plotCounter >= maxUnitsPerRow)
                        {
                            plotCounter = 0;
                            row++;
                        }
                    }
                    else
                    {
                        String plotName = electrodeName+" Unit:"+String(processor->trialCircularBuffer->getUnitID(e,u));
                        newplot = new GenericPlot(plotName,psthDisplay,++plotID,SPIKE_PLOT,processor->trialCircularBuffer,
                                                  electrodeID,
                                                  processor->trialCircularBuffer->getUnitID(e,u),
                                                  offset+u,row,rasterMode,inPanMode);
                        numCols = max(numCols,offset+u);
                    }
                    newplot->setSmoothState(smoothPlots);
                    newplot->setAutoRescale(autoRescale);
                    newplot->buildSmoothKernel(gaussianStandardDeviationMS);

                    psthDisplay->psthPlots.push_back(newplot);
                    psthDisplay->addAndMakeVisible(newplot);
                }
                plottedSomething = true;
            }
        }
        if (!compactView &&  plottedSomething)
            row++;
    }
    if (compactView)
    {
        numRows = row+1;//MAX(1,row);
    }
    else
    {
        numRows = row;
        numCols = numCols+1;
    }

    if (maxUnitsPerElectrode == 0 && !showLFP)
    {
        // nothing to be drawn...
        //processor->trialCircularBuffer->unlockPSTH();
        return;
    }

    psthDisplay->resized();
    psthDisplay->repaint();
    psthDisplay->refresh();
    resized();
    repaint();

    //processor->trialCircularBuffer->unlockPSTH();
    conditionsList->updateConditionButtons();
}


void PeriStimulusTimeHistogramCanvas::resized()
{
    screenWidth = getWidth();
    screenHeight = getHeight();

    //int scrollBarThickness = viewport->getScrollBarThickness();

    viewport->setBounds(0,30,getWidth()-conditionWidth,getHeight()-30);
    int totalHeight = numRows * heightPerElectrodePix;
    int totalWidth = numCols * widthPerUnit;
    psthDisplay->setBounds(0,0,totalWidth, totalHeight);

    int numConditions = 0;
    if (processor->trialCircularBuffer != nullptr)
    {
        numConditions = processor->trialCircularBuffer->getNumConditions();
    }

    conditionsViewport->setBounds(getWidth()-conditionWidth,30,conditionWidth,getHeight());
    conditionsList->setBounds(0,0,conditionWidth, 50+20*numConditions);

    visualizationButton->setBounds(20,5,150,20);
    clearAllButton->setBounds(200,5,150,20);

    zoomButton->setBounds(360,5,60,20);
    panButton->setBounds(440,5,60,20);
    resetAxesButton->setBounds(510,5,150,20);

}

void PeriStimulusTimeHistogramCanvas::paint(Graphics& g)
{
    if (updateNeeded)
        update();
    g.fillAll(Colours::grey);

}

void PeriStimulusTimeHistogramCanvas::refresh()
{
    repaint();
    psthDisplay->refresh();
}

/***********************************************/

PeriStimulusTimeHistogramDisplay::PeriStimulusTimeHistogramDisplay(PeriStimulusTimeHistogramNode* n, Viewport* p, PeriStimulusTimeHistogramCanvas* c) :
    processor(n), viewport(p), canvas(c)
{

    font = Font("Default", 15, Font::plain);
}

PeriStimulusTimeHistogramDisplay::~PeriStimulusTimeHistogramDisplay()
{
    for (int k = 0; k < psthPlots.size(); k++)
    {
        delete(psthPlots[k]);
    }
    psthPlots.clear();
}

void PeriStimulusTimeHistogramDisplay::refresh()
{
    for (int k = 0; k < psthPlots.size(); k++)
    {
        psthPlots[k]->repaint();
    }

}

void PeriStimulusTimeHistogramDisplay::paint(Graphics& g)
{
    g.setColour(Colours::white);
    g.drawRect(0,0,getWidth(),getHeight());
    /*
    	font = Font("Default", 15, Font::plain);

    	g.setFont(font);

    	g.drawText("Test",10,0,200,20,Justification::left,false);
    	*/
}

void PeriStimulusTimeHistogramDisplay::setAutoRescale(bool state)
{
    // draw n by m grid
    PeriStimulusTimeHistogramEditor* ed = (PeriStimulusTimeHistogramEditor*) processor->getEditor();
    ed->showAutoRescale = state;

    for (int k=0; k<psthPlots.size(); k++)
    {
        psthPlots[k]->setAutoRescale(state);
    }

}

void PeriStimulusTimeHistogramDisplay::resized()
{
    // draw n by m grid
    for (int k=0; k<psthPlots.size(); k++)
    {
        if (psthPlots[k]->isFullScreen())
        {
            int newSize = MIN(canvas->screenWidth,canvas->screenHeight);
            setBounds(0,0,newSize,newSize);
            psthPlots[k]->setBounds(0,0,newSize-30,newSize-30);

        }
        else
        {
            psthPlots[k]->setBounds(psthPlots[k]->getRow() * canvas->widthPerUnit,
                                    psthPlots[k]->getCol() * canvas->heightPerElectrodePix,
                                    canvas->widthPerUnit,
                                    canvas->heightPerElectrodePix);
        }

    }
}

void PeriStimulusTimeHistogramDisplay::focusOnPlot(int plotID)
{
    int plotIndex = -1;
    for (int i=0; i<psthPlots.size(); i++)
    {
        if (psthPlots[i]->getPlotID() == plotID)
        {
            plotIndex = i;
            break;
        }

    }
    if (plotIndex == -1)
        return;
    if (psthPlots[plotIndex]->isFullScreen())
    {

        psthPlots[plotIndex]->toggleFullScreen(false);
        psthPlots[plotIndex]->setBounds(psthPlots[plotIndex]->getRow() * canvas->widthPerUnit,
                                        psthPlots[plotIndex]->getCol() * canvas->heightPerElectrodePix,
                                        canvas->widthPerUnit,
                                        canvas->heightPerElectrodePix);
        // hide all other plots.
        for (int k=0; k<psthPlots.size(); k++)
        {
            psthPlots[k]->setVisible(true);
            psthPlots[k]->repaint();
        }

    }
    else
    {
        // hide all other plots.
        for (int k=0; k<psthPlots.size(); k++)
        {
            if (psthPlots[k]->getPlotID() != plotID)
                psthPlots[k]->setVisible(false);
        }
        psthPlots[plotIndex]->toggleFullScreen(true);
        // make sure its rectangular...?
        int newSize = MIN(canvas->screenWidth,canvas->screenHeight);
        setBounds(0,0,newSize,newSize);
        psthPlots[plotIndex]->setBounds(0,0,newSize-30,newSize-30);
        psthPlots[plotIndex]->repaint();
    }

}

/***********************************/
ConditionList::ConditionList(PeriStimulusTimeHistogramNode* n, Viewport* p, PeriStimulusTimeHistogramCanvas* c) :
    processor(n), viewport(p), canvas(c)
{

    titleButton = new ColorButton("CONDITIONS LIST", Font("Default", 24, Font::plain));
    titleButton->setBounds(0,0, 200,25);
    titleButton->addListener(this);
    addAndMakeVisible(titleButton);

    allButton = new ColorButton("All", Font("Default", 20, Font::plain));
    allButton->setBounds(0,25,100,20);
    allButton->addListener(this);
    addAndMakeVisible(allButton);

    noneButton = new ColorButton("None", Font("Default", 20, Font::plain));
    noneButton->setBounds(100,25,100,20);
    noneButton->addListener(this);
    addAndMakeVisible(noneButton);

    updateConditionButtons();

}


void ConditionList::updateConditionButtons()
{
    if (processor->trialCircularBuffer != nullptr)
    {
        const ScopedLock myScopedLock(processor->trialCircularBuffer->psthMutex);
        //processor->trialCircularBuffer->lockConditions();
        conditionButtons.clear();
        for (int k=0; k<processor->trialCircularBuffer->getNumConditions(); k++)
        {
            Condition cond = processor->trialCircularBuffer->getCondition(k);
            ColorButton* conditionButton = new ColorButton(cond.name, Font("Default", 20, Font::plain));
            conditionButton->setBounds(0,50+k*20,200,20);
            conditionButton->setColors(Colours::white,
                                       juce::Colour::fromRGB(cond.colorRGB[0],
                                                             cond.colorRGB[1],
                                                             cond.colorRGB[2]));
            conditionButton->setEnabledState(cond.visible);
            conditionButton->setUserDefinedData(cond.conditionID);
            conditionButton->setShowEnabled(true);
            conditionButton->addListener(this);
            addAndMakeVisible(conditionButton);
            conditionButtons.add(conditionButton);
        }

        //processor->trialCircularBuffer->unlockConditions();
    }
}

ConditionList::~ConditionList()
{

    for (int i = 0; i < conditionButtons.size(); i++)
    {
        removeChildComponent(conditionButtons[i]);
    }
}

void ConditionList::paint(Graphics& g)
{
    g.fillAll(juce::Colours::grey);
    //g.drawText
}

void ConditionList::buttonClicked(Button* btn)
{
    ColorButton* cbtn = (ColorButton*)btn;
    // also inform trial circular buffer about visibility change.
    if (btn == titleButton)
    {
        //int x = 5;
    }
    else if (btn == noneButton)
    {
        if (processor->trialCircularBuffer != nullptr)
        {
            //processor->trialCircularBuffer->lockConditions();
            const ScopedLock myScopedLock(processor->trialCircularBuffer->psthMutex);
            for (int k=0; k<processor->trialCircularBuffer->getNumConditions(); k++)
            {
                processor->trialCircularBuffer->modifyConditionVisibility(k,false);
                conditionButtons[k]->setEnabledState(false);
            }
            //processor->trialCircularBuffer->unlockConditions();
        }

    }
    else if (btn == allButton)
    {
        if (processor->trialCircularBuffer != nullptr)
        {
            const ScopedLock myScopedLock(processor->trialCircularBuffer->psthMutex);
            //processor->trialCircularBuffer->lockConditions();
            for (int k=0; k<processor->trialCircularBuffer->getNumConditions(); k++)
            {
                processor->trialCircularBuffer->modifyConditionVisibility(k,true);
                conditionButtons[k]->setEnabledState(true);
            }
            //processor->trialCircularBuffer->unlockConditions();
        }

    }
    else
    {
        // probably a condition button
        int conditionID = cbtn->getUserDefinedData();
        cbtn->setEnabledState(!cbtn->getEnabledState());
        processor->trialCircularBuffer->modifyConditionVisibilityusingConditionID(conditionID, cbtn->getEnabledState());
    }

    repaint();
}


/*************************************************************************/
// Generic plot replaces XYPlot class, with better class organization and code encapsulation.
// All low level plotting of curves is handled by MatlabLikePlot object (including zooming / panning / ...)
// All raster plots will be handled by another class (?)
//

GenericPlot::GenericPlot(String name,PeriStimulusTimeHistogramDisplay* dsp, int plotID_, xyPlotTypes plotType_,
                         TrialCircularBuffer* tcb_, int electrodeID_, int subID_, int row_, int col_, bool rasterMode_, bool panM) :  display(dsp),tcb(tcb_), plotID(plotID_),
    plotType(plotType_), electrodeID(electrodeID_), subID(subID_), row(row_), col(col_), rasterMode(rasterMode_),inPanMode(panM),plotName(name)
{
    fullScreenMode = false;
    mlp = new MatlabLikePlot();
    mlp->setControlButtonsVisibile(false);
    TrialCircularBufferParams params =  tcb->getParams();

    if (inPanMode)
        mlp->setMode(DrawComponentMode::PAN);
    else
        mlp->setMode(DrawComponentMode::ZOOM);

    if (rasterMode)
    {
        mlp->setImageMode(true);
        mlp->setAutoRescale(false);
    }
    else
        mlp->setImageMode(false);


    addAndMakeVisible(mlp);

    mlp->setTitle(plotName);
    if (plotType == SPIKE_PLOT)
    {
        //mlp->setTitle("Unit "+String(electrodeID)+":"+String(subID));
        mlp->setFiringRateMode(true);
        mlp->setBorderColor(tcb->getUnitColor(electrodeID, subID));
        int uniqueIntervalID = tcb->getUnitUniqueInterval(electrodeID, subID);
        mlp->setActivateButtonVisiblilty(true, uniqueIntervalID);
        mlp->setRangeLimit(-params.preSec,params.postSec+params.maxTrialTimeSeconds,0,1e3);
    }
    else if (plotType == LFP_PLOT)
    {
        //	mlp->setTitle("Ch "+String(electrodeID)+":"+String(subID));
        mlp->setBorderColor(juce::Colours::white);
        mlp->setRangeLimit(-params.preSec,params.postSec+params.maxTrialTimeSeconds,-1e3,1e3);
    }

    guassianStandardDeviationMS = 5; // default smoothing
    buildSmoothKernel(guassianStandardDeviationMS);

    smoothPlot = plotType == SPIKE_PLOT; // don't smooth LFPs
    fullScreenMode = false;
}

xyPlotTypes GenericPlot::getPlotType()
{
    return plotType;
}

void GenericPlot::resized()
{
    int w = getWidth();
    int h = getHeight();
    mlp->setBounds(0,0,w,h);
}

void GenericPlot::paintSpikeRaster(Graphics& g)
{
    //tictoc.Tic(16);
    int numTrialTypes = tcb->getNumTrialTypesInUnit(electrodeID, subID);
    if (numTrialTypes > 0)
    {
        int numTrials = tcb->getNumTrialsInUnit(electrodeID, subID);
        mlp->setAuxiliaryString(String(numTrials) + " trials");

        float xmin,xmax,ymin,ymax,maxValue;
        mlp->getRange(xmin,xmax,ymin,ymax);
        juce::Image rasterImage = tcb->getTrialsAverageUnitResponseAsJuceImage(electrodeID, subID,guassianStandardDeviationMS,xmin,xmax,ymin, ymax,  maxValue);
        mlp->drawImage(rasterImage,maxValue);
    }
    //tictoc.Toc(17);
}

void GenericPlot::paintSpikes(Graphics& g)
{
    //tictoc.Tic(15);
    std::vector<XYline> lines = tcb->getUnitConditionCurves(electrodeID, subID);
    int numTrials = tcb->getNumTrialsInUnit(electrodeID, subID);
    mlp->setAuxiliaryString(String(numTrials) + " trials");

    mlp->clearplot();
    for (int k=0; k<lines.size(); k++)
    {
        if (smoothPlot)
        {
            lines[k].smooth(smoothKernel);
        }
        mlp->plotxy(lines[k]);
    }
    //tictoc.Toc(15);
}

void GenericPlot::paintLFPraster(Graphics& g)
{
    //tictoc.Tic(14);
    int numTrialTypes = tcb->getNumTrialTypesInChannel(electrodeID, subID);
    if (numTrialTypes > 0)
    {
        int numTrials = tcb->getNumTrialsInChannel(electrodeID, subID);
        mlp->setAuxiliaryString(String(numTrials) + " trials");

        float xmin,xmax,ymin,ymax,maxValue;
        mlp->getRange(xmin,xmax,ymin,ymax);
        juce::Image rasterImage = tcb->getTrialsAverageChannelResponseAsJuceImage(electrodeID, subID,guassianStandardDeviationMS,xmin,xmax,ymin, ymax,  maxValue);
        mlp->drawImage(rasterImage,maxValue);
    }
    //tictoc.Toc(14);
}

void GenericPlot::paintLFP(Graphics& g)
{
    //tictoc.Tic(13);
    std::vector<XYline> lines = tcb->getElectrodeConditionCurves(electrodeID, subID);
    mlp->clearplot();

    int numTrials = tcb->getNumTrialsInChannel(electrodeID, subID);
    mlp->setAuxiliaryString(String(numTrials) + " trials");

    for (int k=0; k<lines.size(); k++)
    {
        if (smoothPlot)
        {
            lines[k].smooth(smoothKernel);
        }
        mlp->plotxy(lines[k]);
    }
    //tictoc.Toc(13);
}

void GenericPlot::paint(Graphics& g)
{
    //printf("Entering GenericPlot::paint\n");
    //tictoc.Tic(12);
    if (mlp->eventsAvail())
    {
        String lastEvent = mlp->getLastEvent();
        handleEventFromMatlabLikePlot(lastEvent);
    }

    if (plotType == SPIKE_PLOT)
    {
        if (rasterMode)
            paintSpikeRaster(g);
        else
            paintSpikes(g);
    }
    else if (plotType == LFP_PLOT)
    {
        if (rasterMode)
            paintLFPraster(g);
        else
            paintLFP(g);
    }
    //printf("Exitting GenericPlot::paint\n");
    //tictoc.Toc(12);
}


void GenericPlot::setSmoothState(bool state)
{
    smoothPlot = state;
}

void GenericPlot::setAutoRescale(bool state)
{
    autoRescale = state;
    mlp->setAutoRescale(state);
}

void GenericPlot::setXRange(double xmin, double xmax)
{
    float curr_minx,curr_miny,curr_maxx,curr_maxy;
    mlp->getRange(curr_minx,curr_maxx,curr_miny,curr_maxy);
    mlp->setRange(xmin,xmax,curr_miny,curr_maxy,false);
}

void GenericPlot::setYRange(double ymin,double ymax)
{
    float curr_minx,curr_miny,curr_maxx,curr_maxy;
    mlp->getRange(curr_minx,curr_maxx,curr_miny,curr_maxy);
    mlp->setRange(curr_minx,curr_maxx, ymin,ymax,false);
}


void GenericPlot::setMode(DrawComponentMode mode)
{
    mlp->setMode(mode);
}

void GenericPlot::resetAxes()
{
    TrialCircularBufferParams params =  tcb->getParams();
    if (plotType == SPIKE_PLOT)
    {
        std::vector<XYline> lines = tcb->getUnitConditionCurves(electrodeID, subID);
        double trial_xmin, trial_xmax;
        tcb->getUnitConditionRange(electrodeID, subID, trial_xmin, trial_xmax);

        float xmin=0,xmax=0;
        double ymin=0,ymax=0;
        float highestY=0;
        for (int k=0; k<lines.size(); k++)
        {
            lines[k].getYRange(xmin,xmax,ymin,ymax);
            highestY = MAX(highestY, ymax);
        }
        mlp->setRange(trial_xmin, trial_xmax,0,highestY,false);

    }
    else if (plotType == LFP_PLOT)
    {
        double trial_xmin, trial_xmax;
        tcb->getElectrodeConditionRange(electrodeID, subID, trial_xmin, trial_xmax);

        std::vector<XYline> lines = tcb->getElectrodeConditionCurves(electrodeID, subID);
        float xmin=0,xmax=0;
        double ymin=0,ymax=0;
        float highestY=-1e10,lowestY=1e10;
        for (int k=0; k<lines.size(); k++)
        {
            lines[k].getYRange(xmin,xmax,ymin,ymax);
            highestY = MAX(highestY, ymax);
            lowestY = MIN(lowestY, ymin);
        }

        mlp->setRange(trial_xmin, trial_xmax,lowestY,highestY,false);
    }
}

void GenericPlot::buildSmoothKernel(float gaussianStandardDeviationMS_)
{
    guassianStandardDeviationMS = gaussianStandardDeviationMS_;
    // assume each bin correponds to one millisecond.
    // build the gaussian kernel
    int numKernelBins = 2*(int)(guassianStandardDeviationMS*3.5)+1; // +- 3.5 standard deviations.
    int zeroIndex = (numKernelBins-1)/2;
    smoothKernel.resize(numKernelBins);
    float sumZ = 0;
    for (int k=0; k<numKernelBins; k++)
    {
        float z = float(k-zeroIndex);
        smoothKernel[k] = exp(- (z*z)/(2*guassianStandardDeviationMS*guassianStandardDeviationMS));
        sumZ+=smoothKernel[k];
    }
    // normalize kernel
    for (int k=0; k<numKernelBins; k++)
    {
        smoothKernel[k] /= sumZ;
    }
}

void GenericPlot::handleEventFromMatlabLikePlot(String event)
{
    std::vector<String> command = StringTS(event).splitString(' ');
    //addEvent("NewRange "+String(xmin)+" "+String(xmax)+" "+String(ymin)+" "+String(ymax));
    if (command[0] == "DblkClickRight")
    {

        if (plotType == SPIKE_PLOT)
        {
            tcb->clearUnitStatistics(electrodeID, subID);
        }
        else if (plotType == LFP_PLOT)
        {
            tcb->clearChanneltatistics(electrodeID, subID);
        }

    }
    else if (command[0] == "DblkClickLeft")
    {
        // full screen toggle
        display->focusOnPlot(plotID);
    }
    else if (command[0] == "StartInterval")
    {


        int uniqueIntervalID = tcb->setUnitUniqueInterval(electrodeID, subID,true);
        mlp->setActivateButtonVisiblilty(true, uniqueIntervalID);
        // post this as a network message as well
        StringTS s("UnitIntervalStart "+String(electrodeID)+" "+String(subID)+" "+String(uniqueIntervalID));
        display->processor->handleNetworkMessage(s);
    }
    else if (command[0] == "StopInterval")
    {
        int uniqueIntervalID = tcb->setUnitUniqueInterval(electrodeID, subID,false);
        mlp->setActivateButtonVisiblilty(true, -1);
        // post this as a network message as well
        StringTS s("UnitIntervalStop "+String(electrodeID)+" "+String(subID) + " "+String(uniqueIntervalID));
        display->processor->handleNetworkMessage(s);
    }
    else if (command[0] == "NewRange")
    {
        if (display->canvas->getMatchRange())
        {
            double xmin = command[1].getDoubleValue();
            double xmax = command[2].getDoubleValue();
            double ymin = command[3].getDoubleValue();
            double ymax = command[4].getDoubleValue();
            display->canvas->setRange(xmin,xmax,ymin,ymax,plotType);
        }
    }
}
