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

#include "SpikeSorterCanvas.h"


SpikeSorterCanvas::SpikeSorterCanvas(SpikeSorter* n) :
    processor(n), newSpike(false)
{
    electrode = nullptr;
    viewport = new Viewport();
    spikeDisplay = new SpikeThresholdDisplay(n,this, viewport);

    viewport->setViewedComponent(spikeDisplay, false);
    viewport->setScrollBarsShown(true, true);

    inDrawingPolygonMode = false;
    scrollBarThickness = viewport->getScrollBarThickness();

    addUnitButton = new UtilityButton("New box unit", Font("Small Text", 13, Font::plain));
    addUnitButton->setRadius(3.0f);
    addUnitButton->addListener(this);
    addAndMakeVisible(addUnitButton);

    addPolygonUnitButton = new UtilityButton("New polygon", Font("Small Text", 13, Font::plain));
    addPolygonUnitButton->setRadius(3.0f);
    addPolygonUnitButton->addListener(this);
    addAndMakeVisible(addPolygonUnitButton);

    addBoxButton = new UtilityButton("Add box", Font("Small Text", 13, Font::plain));
    addBoxButton->setRadius(3.0f);
    addBoxButton->addListener(this);
    addAndMakeVisible(addBoxButton);

    delUnitButton = new UtilityButton("Delete", Font("Small Text", 13, Font::plain));
    delUnitButton->setRadius(3.0f);
    delUnitButton->addListener(this);
    addAndMakeVisible(delUnitButton);

    rePCAButton = new UtilityButton("Re-PCA", Font("Small Text", 13, Font::plain));
    rePCAButton->setRadius(3.0f);
    rePCAButton->addListener(this);
    addAndMakeVisible(rePCAButton);

    newIDbuttons = new UtilityButton("New IDs", Font("Small Text", 13, Font::plain));
    newIDbuttons->setRadius(3.0f);
    newIDbuttons->addListener(this);
    addAndMakeVisible(newIDbuttons);

    deleteAllUnits = new UtilityButton("Delete All", Font("Small Text", 13, Font::plain));
    deleteAllUnits->setRadius(3.0f);
    deleteAllUnits->addListener(this);
    addAndMakeVisible(deleteAllUnits);

    nextElectrode = new UtilityButton("Next Electrode", Font("Small Text", 13, Font::plain));
    nextElectrode->setRadius(3.0f);
    nextElectrode->addListener(this);
    addAndMakeVisible(nextElectrode);

    prevElectrode = new UtilityButton("Prev Electrode", Font("Small Text", 13, Font::plain));
    prevElectrode->setRadius(3.0f);
    prevElectrode->addListener(this);
    addAndMakeVisible(prevElectrode);
    
    editAllThresholds = new UtilityButton("Edit All Thresholds",Font("Small Text", 13, Font::plain));
    editAllThresholds->addListener(this);
    editAllThresholds->setBounds(140,30,60,20);
    editAllThresholds->setClickingTogglesState(true);
    addAndMakeVisible(editAllThresholds);
    //
    
    addAndMakeVisible(viewport);

    setWantsKeyboardFocus(true);

    update();

}

SpikeSorterCanvas::~SpikeSorterCanvas()
{

}

void SpikeSorterCanvas::beginAnimation()
{
    std::cout << "SpikeSorterCanvas beginning animation." << std::endl;

    startCallbacks();
}

void SpikeSorterCanvas::endAnimation()
{
    std::cout << "SpikeSorterCanvas ending animation." << std::endl;

    stopCallbacks();
}

void SpikeSorterCanvas::update()
{

    std::cout << "Updating SpikeSorterCanvas" << std::endl;

    int nPlots = processor->getNumElectrodes();
    processor->removeSpikePlots();
    spikeDisplay->removePlots();

    if (nPlots > 0)
    {
        // Plot only active electrode
        int currentElectrode = processor->getCurrentElectrodeIndex();
        electrode = processor->getActiveElectrode();
        SpikeHistogramPlot* sp = spikeDisplay->addSpikePlot(processor->getNumberOfChannelsForElectrode(currentElectrode), electrode->electrodeID,
                                                            processor->getNameForElectrode(currentElectrode));
        processor->addSpikePlotForElectrode(sp, currentElectrode);
        electrode->spikePlot->setFlipSignal(processor->getFlipSignalState());
        electrode->spikePlot->updateUnitsFromProcessor();

    }
    spikeDisplay->resized();
    spikeDisplay->repaint();
}


void SpikeSorterCanvas::refreshState()
{
    // called when the component's tab becomes visible again
    resized();
}

void SpikeSorterCanvas::resized()
{
    viewport->setBounds(130,0,getWidth()-140,getHeight());

    spikeDisplay->setBounds(0,0,getWidth()-140, spikeDisplay->getTotalHeight());

    nextElectrode->setBounds(0, 20, 120,30);
    prevElectrode->setBounds(0, 60, 120,30);

    addUnitButton->setBounds(0, 120, 120,20);
    addPolygonUnitButton->setBounds(0, 150, 120,20);
    addBoxButton->setBounds(0, 180, 120,20);
    delUnitButton->setBounds(0, 210, 120,20);

    rePCAButton->setBounds(0, 240, 120,20);

    newIDbuttons->setBounds(0, 270, 120,20);
    deleteAllUnits->setBounds(0, 300, 120,20);
    
    editAllThresholds->setBounds(0, 330, 120,20);

}

void SpikeSorterCanvas::paint(Graphics& g)
{

    g.fillAll(Colours::darkgrey);

}

void SpikeSorterCanvas::refresh()
{
    // called every 10 Hz
    processSpikeEvents();

    repaint();
}


void SpikeSorterCanvas::processSpikeEvents()
{


    //Electrode* e = ((SpikeSorter*) processor)->getActiveElectrode();

    //	e->spikeSort->lstLastSpikes
    //    processor->setParameter(2, 0.0f); // request redraw

}





void SpikeSorterCanvas::removeUnitOrBox()
{
    int electrodeID = processor->getActiveElectrode()->electrodeID;
    int unitID, boxID;
    processor->getActiveElectrode()->spikePlot->getSelectedUnitAndBox(unitID, boxID);
    bool selectNewBoxUnit = false;
    bool selectNewPCAUnit = false;
    if (unitID > 0)
    {
        if (boxID >= 0)
        {
            // box unit
            int numBoxes = processor->getActiveElectrode()->spikeSort->getNumBoxes(unitID);
            if (numBoxes > 1)
            {
                // delete box, but keep unit
                processor->getActiveElectrode()->spikeSort->removeBoxFromUnit(unitID, boxID);
                electrode->spikePlot->updateUnitsFromProcessor();
                electrode->spikePlot->setSelectedUnitAndBox(unitID,0);
            }
            else
            {
                // delete unit
                processor->getActiveElectrode()->spikeSort->removeUnit(unitID);
                electrode->spikePlot->updateUnitsFromProcessor();
                processor->removeUnit(electrodeID, unitID);

                std::vector<BoxUnit> boxunits = processor->getActiveElectrode()->spikeSort->getBoxUnits();
                std::vector<PCAUnit> pcaunits = processor->getActiveElectrode()->spikeSort->getPCAUnits();
                if (boxunits.size() > 0)
                {
                    selectNewBoxUnit = true;
                }
                else if (pcaunits.size() > 0)
                {
                    selectNewPCAUnit = true;
                }
                else
                {
                    electrode->spikePlot->setSelectedUnitAndBox(-1,-1);
                }
            }
        }
        else
        {
            // pca unit
            processor->getActiveElectrode()->spikeSort->removeUnit(unitID);
            electrode->spikePlot->updateUnitsFromProcessor();
            processor->removeUnit(electrodeID, unitID);

            std::vector<BoxUnit> boxunits = processor->getActiveElectrode()->spikeSort->getBoxUnits();
            std::vector<PCAUnit> pcaunits = processor->getActiveElectrode()->spikeSort->getPCAUnits();
            if (pcaunits.size() > 0)
            {
                selectNewPCAUnit = true;
            }
            else if (boxunits.size() > 0)
            {
                selectNewBoxUnit = true;
            }
            else
            {
                electrode->spikePlot->setSelectedUnitAndBox(-1,-1);
            }


        }
        if (selectNewPCAUnit)
        {
            // set new selected unit to be the last existing unit
            std::vector<PCAUnit> u = processor->getActiveElectrode()->spikeSort->getPCAUnits();
            if (u.size() > 0)
            {
                electrode->spikePlot->setSelectedUnitAndBox(u[u.size()-1].getUnitID(),-1);
            }
            else
            {
                electrode->spikePlot->setSelectedUnitAndBox(-1,-1);
            }
        }
        if (selectNewBoxUnit)
        {
            // set new selected unit to be the last existing unit
            std::vector<BoxUnit> u = processor->getActiveElectrode()->spikeSort->getBoxUnits();
            if (u.size() > 0)
            {
                electrode->spikePlot->setSelectedUnitAndBox(u[u.size()-1].getUnitID(),0);
            }
            else
            {
                electrode->spikePlot->setSelectedUnitAndBox(-1,-1);
            }
        }



    }

}

bool SpikeSorterCanvas::keyPressed(const KeyPress& key)
{

    KeyPress c = KeyPress::createFromDescription("c");
    KeyPress e = KeyPress::createFromDescription("escape");
    KeyPress d = KeyPress::createFromDescription("delete");

    if (key.isKeyCode(c.getKeyCode())) // C
    {
        spikeDisplay->clear();

        std::cout << "Clearing display" << std::endl;
        return true;
    }
    else  if (key.isKeyCode(e.getKeyCode()))   // ESC
    {
        spikeDisplay->setPolygonMode(false);
        return true;
    }
    else  if (key.isKeyCode(d.getKeyCode()))   // Delete
    {
        removeUnitOrBox();
        return true;
    }

    return false;

}

void SpikeSorterCanvas::buttonClicked(Button* button)
{
    int channel = 0;
    int unitID = -1;
    int boxID = -1;
    Time t;

    if (button == addPolygonUnitButton)
    {
        inDrawingPolygonMode = true;
        addPolygonUnitButton->setToggleState(true, dontSendNotification);
        electrode->spikePlot->setPolygonDrawingMode(true);
    }
    else if (button == addUnitButton)
    {
        Electrode* e = processor->getActiveElectrode();

        if (e != nullptr)
        {
            int electrodeID = processor->getActiveElectrode()->electrodeID;

            std::cout << "Adding box unit to electrode " << e->electrodeID << std::endl;
            int newUnitID = processor->getActiveElectrode()->spikeSort->addBoxUnit(0);

            uint8 r, g, b;
            processor->getActiveElectrode()->spikeSort->getUnitColor(newUnitID, r, g, b);
            electrode->spikePlot->updateUnitsFromProcessor();
            electrode->spikePlot->setSelectedUnitAndBox(newUnitID, 0);
            processor->addNewUnit(electrodeID, newUnitID, r, g, b);
        }

    }
    else if (button == delUnitButton)
    {
        removeUnitOrBox();

    }
    else if (button == addBoxButton)
    {

        processor->getActiveElectrode()->spikePlot->getSelectedUnitAndBox(unitID, boxID);

        if (unitID > 0)
        {
            std::cout << "Adding box to channel " << channel << " with unitID " << unitID << std::endl;
            processor->getActiveElectrode()->spikeSort->addBoxToUnit(channel, unitID);
            electrode->spikePlot->updateUnitsFromProcessor();
        }
    }
    else if (button == rePCAButton)
    {
        processor->getActiveElectrode()->spikeSort->RePCA();
    }
    else if (button == nextElectrode)
    {
        SpikeSorterEditor* ed = (SpikeSorterEditor*)processor->getEditor();
        ed->setElectrodeComboBox(1);
    }
    else if (button == prevElectrode)
    {

        SpikeSorterEditor* ed = (SpikeSorterEditor*)processor->getEditor();

        ed->setElectrodeComboBox(-1);

    }
    else if (button == newIDbuttons)
    {
        // generate new IDs
        processor->getActiveElectrode()->spikeSort->generateNewIDs();
        electrode->spikePlot->updateUnitsFromProcessor();
        //processor->updateSinks(electrode->electrodeID,false);
    }
    else if (button == deleteAllUnits)
    {
        // delete unit
        processor->getActiveElectrode()->spikeSort->removeAllUnits();
        electrode->spikePlot->updateUnitsFromProcessor();
        processor->removeAllUnits(electrode->electrodeID);
    }
    else if (button == editAllThresholds){
        
    }
    
    // new
    if (button == editAllThresholds){
        processor->setEditAllState(button->getToggleState());
    }
    
    repaint();
}





// ----------------------------------------------------------------

SpikeThresholdDisplay::SpikeThresholdDisplay(SpikeSorter* p, SpikeSorterCanvas* sdc, Viewport* v) :
    processor(p),canvas(sdc), viewport(v)
{

    totalHeight = 1000;

}

SpikeThresholdDisplay::~SpikeThresholdDisplay()
{

}

void SpikeThresholdDisplay::clear()
{
    if (spikePlots.size() > 0)
    {
        for (int i = 0; i < spikePlots.size(); i++)
        {
            spikePlots[i]->clear();
        }
    }

}


void SpikeThresholdDisplay::removePlots()
{
    spikePlots.clear();

}

SpikeHistogramPlot* SpikeThresholdDisplay::addSpikePlot(int numChannels, int electrodeID, String name_)
{

    std::cout << "Adding new spike plot." << std::endl;

    SpikeHistogramPlot* spikePlot = new SpikeHistogramPlot(processor,canvas, electrodeID, 1000 + numChannels, name_);
    spikePlots.add(spikePlot);
    addAndMakeVisible(spikePlot);

    return spikePlot;
}

void SpikeThresholdDisplay::paint(Graphics& g)
{

    g.fillAll(Colours::grey);

}

void SpikeThresholdDisplay::setPolygonMode(bool on)
{
    if (spikePlots.size() > 0)
        spikePlots[0]->setPolygonDrawingMode(on);
}

void SpikeThresholdDisplay::resized()
{
    // this is kind of a mess -- is there any way to optimize it?

    if (spikePlots.size() > 0)
    {

        int w = getWidth();
        int h = 430;//getHeight();

        spikePlots[0]->setBounds(0, 0, w, h);


        setBounds(0, 0, w, h);
    }

}

void SpikeThresholdDisplay::mouseDown(const MouseEvent& event)
{

}

void SpikeThresholdDisplay::plotSpike(SorterSpikePtr spike, int electrodeNum)
{
    spikePlots[electrodeNum]->processSpikeObject(spike);

}






// ----------------------------------------------------------------

SpikeHistogramPlot::SpikeHistogramPlot(SpikeSorter* prc,SpikeSorterCanvas* sdc, int electrodeID_, int p, String name_) :
    canvas(sdc), isSelected(false), plotType(p), electrodeID(electrodeID_),
    limitsChanged(true), processor(prc), name(name_)

{

    font = Font("Default", 15, Font::plain);

    switch (p)
    {
        case SINGLE_PLOT:
            // std::cout<<"SpikePlot as SINGLE_PLOT"<<std::endl;
            nWaveAx = 1;
            nProjAx = 1;
            nChannels = 1;
            minWidth = 600;
            aspectRatio = 0.5f;
            break;
        case STEREO_PLOT:
            //  std::cout<<"SpikePlot as STEREO_PLOT"<<std::endl;
            nWaveAx = 2;
            nProjAx = 1;
            nChannels = 2;
            minWidth = 300;
            aspectRatio = 0.5f;
            break;
        case TETRODE_PLOT:
            // std::cout<<"SpikePlot as TETRODE_PLOT"<<std::endl;
            nWaveAx = 4;
            nProjAx = 1;
            nChannels = 4;
            minWidth = 400;
            aspectRatio = 0.5f;
            break;
        //        case HIST_PLOT:
        //            nWaveAx = 1;
        //            nProjAx = 0;
        //            nHistAx = 1;
        //            break;
        default: // unsupported number of axes provided
            std::cout << "SpikePlot as UNKNOWN, defaulting to SINGLE_PLOT" << std::endl;
            nWaveAx = 1;
            nProjAx = 0;
            plotType = SINGLE_PLOT;
            nChannels = 1;
    }

    std::vector<float> scales = processor->getElectrodeVoltageScales(electrodeID);
    initAxes(scales);

    for (int i = 0; i < nChannels; i++)
    {
        UtilityButton* rangeButton = new UtilityButton(String(scales[i],0), Font("Small Text", 10, Font::plain));
        rangeButton->setRadius(3.0f);
        rangeButton->addListener(this);
        addAndMakeVisible(rangeButton);

        rangeButtons.add(rangeButton);
    }

}


void SpikeHistogramPlot::setSelectedUnitAndBox(int unitID, int boxID)
{
    const ScopedLock myScopedLock(mut);
    processor->getActiveElectrode()->spikeSort->setSelectedUnitAndBox(unitID, boxID);
}

void SpikeHistogramPlot::getSelectedUnitAndBox(int& unitID, int& boxID)
{
    const ScopedLock myScopedLock(mut);
    processor->getActiveElectrode()->spikeSort->getSelectedUnitAndBox(unitID, boxID);
}


SpikeHistogramPlot::~SpikeHistogramPlot()
{
    pAxes.clear();
    wAxes.clear();

}

void SpikeHistogramPlot::paint(Graphics& g)
{

    //const MessageManagerLock mmLock;

    g.setColour(Colours::white);
    g.drawRect(0,0,getWidth(),getHeight());

    g.setFont(font);

    g.drawText(name,10,0,200,20,Justification::left,false);

}

void SpikeHistogramPlot::setFlipSignal(bool state)
{
    for (int i = 0; i < wAxes.size(); i++)
    {
        wAxes[i]->setSignalFlip(state);
    }
}

void SpikeHistogramPlot::setPolygonDrawingMode(bool on)
{
    const ScopedLock myScopedLock(mut);
    pAxes[0]->setPolygonDrawingMode(on);
}

void SpikeHistogramPlot::updateUnitsFromProcessor()
{
    const ScopedLock myScopedLock(mut);
    boxUnits = processor->getActiveElectrode()->spikeSort->getBoxUnits();
    pcaUnits = processor->getActiveElectrode()->spikeSort->getPCAUnits();

    if (nWaveAx > 0)
    {
        wAxes[0]->updateUnits(boxUnits);
    }
    pAxes[0]->updateUnits(pcaUnits);


    int selectedUnitID, selectedBoxID;
    processor->getActiveElectrode()->spikeSort->getSelectedUnitAndBox(selectedUnitID, selectedBoxID);




}

void SpikeHistogramPlot::setPCARange(float p1min, float p2min, float p1max, float p2max)
{
    const ScopedLock myScopedLock(mut);
    pAxes[0]->setPCARange(p1min, p2min, p1max, p2max);
}

void SpikeHistogramPlot::processSpikeObject(SorterSpikePtr s)
{
    const ScopedLock myScopedLock(mut);
    if (nWaveAx > 0)
    {
        for (int i = 0; i < nWaveAx; i++)
        {
            wAxes[i]->updateSpikeData(s);
        }

        pAxes[0]->updateSpikeData(s);


    }
}

void SpikeHistogramPlot::select()
{
    isSelected = true;
}

void SpikeHistogramPlot::deselect()
{
    isSelected = false;
}

void SpikeHistogramPlot::initAxes(std::vector<float> scales)
{
    const ScopedLock myScopedLock(mut);
    initLimits();

    for (int i = 0; i < nWaveAx; i++)
    {
        WaveformAxes* wAx = new WaveformAxes(this,processor, electrodeID, i);
        wAx->setDetectorThreshold(processor->getActiveElectrode()->thresholds[i]);
        wAxes.add(wAx);
        addAndMakeVisible(wAx);
        ranges.add(scales[i]);
    }

    PCAProjectionAxes* pAx = new PCAProjectionAxes(processor);
    float p1min,p2min, p1max,  p2max;
    processor->getActiveElectrode()->spikeSort->getPCArange(p1min,p2min, p1max,  p2max);
    pAx->setPCARange(p1min,p2min, p1max,  p2max);

    pAxes.add(pAx);
    addAndMakeVisible(pAx);

    setLimitsOnAxes(); // initialize the ranges
}

void SpikeHistogramPlot::resized()
{
    const ScopedLock myScopedLock(mut);

    float width = (float)getWidth()-10;
    float height = (float) getHeight()-25;

    float axesWidth = 0;
    float axesHeight = 0;

    // to compute the axes positions we need to know how many columns of proj and wave axes should exist
    // using these two values we can calculate the positions of all of the sub axes
    int nProjCols = 0;
    int nWaveCols = 0;

    switch (plotType)
    {
        case SINGLE_PLOT:
            nProjCols = 1;
            nWaveCols = 1;
            axesWidth = width/2;
            axesHeight = height;
            break;

        case STEREO_PLOT:
            nProjCols = 1;
            nWaveCols = 2;
            axesWidth = width/2;
            axesHeight = height;
            break;
        case TETRODE_PLOT:
            nProjCols = 1;
            nWaveCols = 2;
            axesWidth = width/2;
            axesHeight = height/2;
            break;
    }

    for (int i = 0; i < nWaveAx; i++)
    {
        wAxes[i]->setBounds(5 + (i % nWaveCols) * axesWidth/nWaveCols, 20 + (i/nWaveCols) * axesHeight, axesWidth/nWaveCols, axesHeight);
        rangeButtons[i]->setBounds(8 + (i % nWaveCols) * axesWidth/nWaveCols,
                                   20 + (i/nWaveCols) * axesHeight + axesHeight - 18,
                                   35, 15);
    }
    pAxes[0]->setBounds(5 +  axesWidth, 20 + 0, width/2, height);


}



void SpikeHistogramPlot::modifyRange(std::vector<float> values)
{
    const int NUM_RANGE = 7;
    float range_array[NUM_RANGE] = {100,250,500,750,1000,1250,1500};
    String label;
    int newIndex = 0;

    for (int index = 0; index < nChannels; index++)
    {
        for (int k = 0; k < NUM_RANGE; k++)
        {
            if (std::abs(values[index] - range_array[k]) < 0.1)
            {
                newIndex = k;
                break;
            }
        }

        ranges.set(index, range_array[newIndex]);
        String label = String(range_array[newIndex],0);
        rangeButtons[index]->setLabel(label);
    }
    setLimitsOnAxes();
}


void SpikeHistogramPlot::modifyRange(int index,bool up)
{
    const int NUM_RANGE = 7;
    float range_array[NUM_RANGE] = {100,250,500,750,1000,1250,1500};
    String label;
    for (int k = 0; k < NUM_RANGE; k++)
    {
        if (std::abs(ranges[index] - range_array[k]) < 0.1)
        {
            int newIndex;
            if (up)
                newIndex  = (k + 1) % NUM_RANGE;
            else
            {
                newIndex  = (k - 1);
                if (newIndex < 0)
                    newIndex  = NUM_RANGE-1;
            }
            ranges.set(index, range_array[newIndex]);
            String label = String(range_array[newIndex],0);
            rangeButtons[index]->setLabel(label);
            setLimitsOnAxes();

            processor->setElectrodeVoltageScale(electrodeID, index, range_array[newIndex]);
            return;
        }

    }
    // we shoudl never reach here.
    jassert(false);
    return ;

}

void SpikeHistogramPlot::buttonClicked(Button* button)
{
    UtilityButton* buttonThatWasClicked = (UtilityButton*) button;

    int index = rangeButtons.indexOf(buttonThatWasClicked);
    modifyRange(index,true);

}

void SpikeHistogramPlot::setLimitsOnAxes()
{
    const ScopedLock myScopedLock(mut);
    for (int i = 0; i < nWaveAx; i++)
        wAxes[i]->setRange(ranges[i]);
}

void SpikeHistogramPlot::initLimits()
{
    for (int i = 0; i < nChannels; i++)
    {
        limits[i][0] = 1209;//-1*pow(2,11);
        limits[i][1] = 11059;//pow(2,14)*1.6;
    }

}

void SpikeHistogramPlot::getBestDimensions(int* w, int* h)
{
    switch (plotType)
    {
        case TETRODE_PLOT:
            *w = 4;
            *h = 2;
            break;
        case STEREO_PLOT:
            *w = 2;
            *h = 1;
            break;
        case SINGLE_PLOT:
            *w = 1;
            *h = 1;
            break;
        default:
            *w = 1;
            *h = 1;
            break;
    }
}

void SpikeHistogramPlot::clear()
{
    const ScopedLock myScopedLock(mut);
    std::cout << "SpikePlot::clear()" << std::endl;

    for (int i = 0; i < nWaveAx; i++)
        wAxes[i]->clear();
    for (int i = 0; i < nProjAx; i++)
        pAxes[i]->clear();


}


void SpikeHistogramPlot::setDisplayThresholdForChannel(int i, float f)
{
    const ScopedLock myScopedLock(mut);
    if (i < wAxes.size())
        wAxes[i]->setDetectorThreshold(f);

    return;
}


float SpikeHistogramPlot::getDisplayThresholdForChannel(int i)
{
    const ScopedLock myScopedLock(mut);
    float f= wAxes[i]->getDisplayThreshold();

    return f;
}

/********************************/



// --------------------------------------------------

GenericDrawAxes::GenericDrawAxes(int t)
    : gotFirstSpike(false), type(t)
{
    ylims[0] = 0;
    ylims[1] = 1;

    xlims[0] = 0;
    xlims[1] = 1;

    font = Font("Default", 12, Font::plain);

}

GenericDrawAxes::~GenericDrawAxes()
{

}

bool GenericDrawAxes::updateSpikeData(SorterSpikePtr newSpike)
{
    if (!gotFirstSpike)
    {
        gotFirstSpike = true;
    }

    s = newSpike;
    return true;
}

void GenericDrawAxes::setYLims(double ymin, double ymax)
{

    //std::cout << "setting y limits to " << ymin << " " << ymax << std::endl;
    ylims[0] = ymin;
    ylims[1] = ymax;
}
void GenericDrawAxes::getYLims(double* min, double* max)
{
    *min = ylims[0];
    *max = ylims[1];
}
void GenericDrawAxes::setXLims(double xmin, double xmax)
{
    xlims[0] = xmin;
    xlims[1] = xmax;
}
void GenericDrawAxes::getXLims(double* min, double* max)
{
    *min = xlims[0];
    *max = xlims[1];
}


void GenericDrawAxes::setType(int t)
{
    if (t < WAVE1 || t > PROJ3x4)
    {
        std::cout<<"Invalid Axes type specified";
        return;
    }
    type = t;
}

int GenericDrawAxes::getType()
{
    return type;
}

int GenericDrawAxes::roundUp(int numToRound, int multiple)
{
    if (multiple == 0)
    {
        return numToRound;
    }

    int remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;
    return numToRound + multiple - remainder;
}


void GenericDrawAxes::makeLabel(int val, int gain, bool convert, char* s)
{
    if (convert)
    {
        double volt = ad16ToUv(val, gain)/1000.;
        if (abs(val)>1e6)
        {
            //val = val/(1e6);
            sprintf(s, "%.2fV", volt);
        }
        else if (abs(val)>1e3)
        {
            //val = val/(1e3);
            sprintf(s, "%.2fmV", volt);
        }
        else
            sprintf(s, "%.2fuV", volt);
    }
    else
    {
        sprintf(s,"%d", (int)val);
    }
}

double GenericDrawAxes::ad16ToUv(int x, int gain)
{
    int result = (double)(x * 20e6) / (double)(gain * pow(2.0,16));
    return result;
}


// --------------------------------------------------


WaveformAxes::WaveformAxes(SpikeHistogramPlot* plt, SpikeSorter* p,int electrodeID_, int _channel) : GenericDrawAxes(_channel),  electrodeID(electrodeID_),
    signalFlipped(false),
    channel(_channel),
    drawGrid(true),
    displayThresholdLevel(0.0f),
    spikesReceivedSinceLastRedraw(0),
    spikeIndex(0),
    bufferSize(5),
    range(250.0f),
    isOverThresholdSlider(false),
    isDraggingThresholdSlider(false),
    processor(p),
    spikeHistogramPlot(plt)
{
    bDragging  = false;

    isOverUnit = -1;
    isOverBox = -1;
    //	 selectedUnit = -1;
    //	 selectedBox = -1;
    addMouseListener(this, true);

    thresholdColour = Colours::red;


    font = Font("Small Text",10,Font::plain);
    int numSamples = 40;
    for (int n = 0; n < bufferSize; n++)
    {
        spikeBuffer.add(nullptr);
    }
}

void WaveformAxes::mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel)
{
    // weirdly enough, sometimes we get twice of this event even though a single wheel move was made...
    if (wheel.deltaY < 0)
        spikeHistogramPlot->modifyRange(channel, true);
    else
        spikeHistogramPlot->modifyRange(channel, false);

}

void WaveformAxes::setSignalFlip(bool state)
{
    signalFlipped = state;
    repaint();
}

void WaveformAxes::setRange(float r)
{

    //std::cout << "Setting range to " << r << std::endl;

    range = r;

    repaint();
}

void WaveformAxes::plotSpike(SorterSpikePtr s, Graphics& g)
{
	if (s.get() == nullptr) return;
    float h = getHeight();

	g.setColour(Colour(s->color[0], s->color[1], s->color[2]));
    //g.setColour(Colours::pink);
    //compute the spatial width for each waveform sample
    float dx = getWidth()/float(s->getChannel()->getTotalSamples());

    /*
    float align = 8 * getWidth()/float(spikeBuffer[0].nSamples);
    g.drawLine(align,
                       0,
                       align,
                       h);
    */
	
	int spikeSamples = s->getChannel()->getTotalSamples();
    // type corresponds to channel so we need to calculate the starting
    // sample based upon which channel is getting plotted
	int offset = channel*spikeSamples; //spikeBuffer[0].nSamples * type; //

    //int dSamples = 1;

    float x = 0.0f;


	for (int i = 0; i < spikeSamples - 1; i++)
	{
		//std::cout << s.data[sampIdx] << std::endl;


		float s1 = h - (h / 2 + s->getData()[offset + i] / (range)* h);
		float s2 = h - (h / 2 + s->getData()[offset + i + 1] / (range)* h);

		if (signalFlipped)
		{
			s1 = h - s1;
			s2 = h - s2;
		}
		g.drawLine(x,
			s1,
			x + dx,
			s2);

		x += dx;
	}

}

void WaveformAxes::drawThresholdSlider(Graphics& g)
{

    // draw display threshold (editable)
    g.setColour(thresholdColour);
    if (signalFlipped)
    {
        float h = getHeight()-(getHeight()*(0.5f - displayThresholdLevel/range));
        g.drawLine(0, h, getWidth(), h);
        g.drawText(String(roundFloatToInt(displayThresholdLevel)),2,h,35,10,Justification::left, false);
    }
    else
    {
        float h = getHeight()*(0.5f - displayThresholdLevel/range);
        g.drawLine(0, h, getWidth(), h);
        g.drawText(String(roundFloatToInt(displayThresholdLevel)),2,h,35,10,Justification::left, false);
    }

}


void WaveformAxes::drawWaveformGrid(Graphics& g)
{

    float h = getHeight();
    float w = getWidth();

    g.setColour(Colours::darkgrey);

    for (float y = -range/2; y < range/2; y += 25.0f)
    {
        if (y == 0)
            g.drawLine(0,h/2 + y/range*h, w, h/2+ y/range*h,2.0f);
        else
            g.drawLine(0,h/2 + y/range*h, w, h/2+ y/range*h);
    }

}


bool WaveformAxes::updateSpikeData(SorterSpikePtr s)
{
    if (!gotFirstSpike)
    {
        gotFirstSpike = true;
    }

    if (spikesReceivedSinceLastRedraw < bufferSize)
    {

        spikeIndex++;
        spikeIndex %= bufferSize;

        spikeBuffer.set(spikeIndex, s);

        spikesReceivedSinceLastRedraw++;

    }

    return true;

}

bool WaveformAxes::checkThreshold(SorterSpikePtr s)
{
    int sampIdx = s->getChannel()->getTotalSamples()*type;

	for (int i = 0; i < s->getChannel()->getTotalSamples() - 1; i++)
    {

        if (s->getData()[sampIdx] > displayThresholdLevel)
        {
            return true;
        }

        sampIdx++;
    }

    return false;

}

void WaveformAxes::clear()
{
    processor->clearRunningStatForSelectedElectrode();
    spikeBuffer.clear();
    spikeIndex = 0;
    int numSamples=40;
    for (int n = 0; n < bufferSize; n++)
    {
        spikeBuffer.add(nullptr);
    }

    repaint();
}

void WaveformAxes::mouseMove(const MouseEvent& event)
{

    // Point<int> pos = event.getPosition();

    float y = event.y;

    float h = getHeight()*(0.5f - displayThresholdLevel/range);
    if (signalFlipped)
    {
        h = getHeight() - h;
    }

    // std::cout << y << " " << h << std::endl;

    if (y > h - 10.0f && y < h + 10.0f && !isOverThresholdSlider)
    {
        thresholdColour = Colours::yellow;
        //  std::cout << "Yes." << std::endl;
        isOverThresholdSlider = true;
        // cursorType = MouseCursor::DraggingHandCursor;
    }
    else if ((y < h - 10.0f || y > h + 10.0f) && isOverThresholdSlider)
    {
        thresholdColour = Colours::red;
        isOverThresholdSlider = false;
    }
    else
    {
        // are we inside a box ?
        isOverUnit = 0;
        isOverBox = -1;
        strOverWhere = "";
        isOverUnitBox(event.x, event.y, isOverUnit, isOverBox, strOverWhere);

    }
    repaint();

}

int WaveformAxes::findUnitIndexByID(int ID)
{
    for (int k = 0; k < units.size(); k++)
        if (units[k].UnitID == ID)
            return k;
    return -1;
}

void WaveformAxes::mouseDown(const juce::MouseEvent& event)
{

    if (event.mods.isRightButtonDown())
    {
        clear();
    }

    float h = getHeight();
    float w = getWidth();
    float microsec_span = 40.0/30000.0 * 1e6;
    float microvolt_span = range/2;
    mouseDownX = event.x/w * microsec_span ;
    if (signalFlipped)
        mouseDownY=(h/2- (h-event.y))/(h/2)*microvolt_span;
    else
        mouseDownY=(h/2- event.y)/(h/2)*microvolt_span;

    if (isOverUnit > 0)
    {
        processor->getActiveElectrode()->spikeSort->setSelectedUnitAndBox(isOverUnit, isOverBox);
        int indx = findUnitIndexByID(isOverUnit);
        jassert(indx >= 0);
        mouseOffsetX = mouseDownX - units[indx].lstBoxes[isOverBox].x;
        mouseOffsetY = mouseDownY - units[indx].lstBoxes[isOverBox].y;
    }
    else
    {
        processor->getActiveElectrode()->spikeSort->setSelectedUnitAndBox(-1, -1);

    }
    //	MouseUnitOffset = ScreenToMS_uV(e.X, e.Y) - new PointD(boxOnDown.x, boxOnDown.y);

    // if (isOverThresholdSlider)
    // {
    //     cursorType = MouseCursor::DraggingHandCursor;
    // }
}


void WaveformAxes::mouseUp(const MouseEvent& event)
{
    if (bDragging)
    {
        bDragging = false;
        // send a message to processor to update its internal structure?
        Electrode* e = processor->getActiveElectrode();
        e->spikeSort->updateBoxUnits(units);
    }
    // if (isOverThresholdSlider)
    // {
    //     cursorType = MouseCursor::DraggingHandCursor;
    // }
}

void WaveformAxes::mouseDrag(const MouseEvent& event)
{
    bDragging = true;

    if (isOverUnit > 0)
    {
        // dragging a box
        // convert position to metric coordinates.
        float h = getHeight();
        float w = getWidth();
        float microsec_span = 40.0/30000.0 * 1e6;
        float microvolt_span = range/2;
        float x = event.x/w * microsec_span ;

        float y;
        if (signalFlipped)
            y=(h/2- (h-event.y))/(h/2)*microvolt_span;
        else
            y=(h/2- event.y)/(h/2)*microvolt_span;

        // update units position....

        for (int k=0; k<units.size(); k++)
        {
            if (units[k].getUnitID() == isOverUnit)
            {
                float oldx = units[k].lstBoxes[isOverBox].x;
                float oldy = units[k].lstBoxes[isOverBox].y;

                float dx = x - oldx;
                float dy = y - oldy;

                if (strOverWhere == "right")
                {
                    units[k].lstBoxes[isOverBox].w = x - oldx;
                }
                else if (strOverWhere == "left")
                {
                    units[k].lstBoxes[isOverBox].w += -dx;
                    units[k].lstBoxes[isOverBox].x = x;
                }
                else if ((!signalFlipped && strOverWhere == "top") || (signalFlipped && strOverWhere == "bottom"))
                {
                    units[k].lstBoxes[isOverBox].y += dy;
                    units[k].lstBoxes[isOverBox].h += dy;
                }
                else if ((!signalFlipped && strOverWhere == "bottom") || (signalFlipped && strOverWhere == "top"))
                {
                    units[k].lstBoxes[isOverBox].h = -dy;
                }
                else if ((!signalFlipped && strOverWhere == "bottomright") || (signalFlipped && strOverWhere == "topright"))
                {
                    units[k].lstBoxes[isOverBox].w = x - oldx;
                    units[k].lstBoxes[isOverBox].h = -dy;

                }
                else if ((!signalFlipped && strOverWhere == "bottomleft") || (signalFlipped && strOverWhere == "topleft"))
                {
                    units[k].lstBoxes[isOverBox].w += -dx;
                    units[k].lstBoxes[isOverBox].x = x;
                    units[k].lstBoxes[isOverBox].h = -dy;
                }
                else if ((!signalFlipped && strOverWhere == "topright") || (signalFlipped && strOverWhere == "bottomright"))
                {
                    units[k].lstBoxes[isOverBox].y += dy;
                    units[k].lstBoxes[isOverBox].h += dy;
                    units[k].lstBoxes[isOverBox].w = x - oldx;

                }
                else if ((!signalFlipped && strOverWhere == "topleft") || (signalFlipped && strOverWhere == "bottomleft"))
                {
                    units[k].lstBoxes[isOverBox].w += -dx;
                    units[k].lstBoxes[isOverBox].x = x;
                    units[k].lstBoxes[isOverBox].y += dy;
                    units[k].lstBoxes[isOverBox].h += dy;

                }
                else if (strOverWhere == "inside")
                {
                    units[k].lstBoxes[isOverBox].x = x-mouseOffsetX;
                    units[k].lstBoxes[isOverBox].y = y-mouseOffsetY;
                }

                if (units[k].lstBoxes[isOverBox].h < 0)
                {
                    units[k].lstBoxes[isOverBox].y -= units[k].lstBoxes[isOverBox].h;
                    units[k].lstBoxes[isOverBox].h *= -1;
                    if (strOverWhere == "top")
                        strOverWhere = "bottom";
                    else if (strOverWhere == "bottom")
                        strOverWhere = "top";
                    else if (strOverWhere == "topleft")
                        strOverWhere = "bottomleft";
                    else if (strOverWhere == "topright")
                        strOverWhere = "bottomright";
                    else if (strOverWhere == "bottomleft")
                        strOverWhere = "topleft";
                    else if (strOverWhere == "bottomright")
                        strOverWhere = "topright";
                }
                if (units[k].lstBoxes[isOverBox].w < 0)
                {
                    units[k].lstBoxes[isOverBox].x += units[k].lstBoxes[isOverBox].w;
                    units[k].lstBoxes[isOverBox].w *= -1;
                    if (strOverWhere == "left")
                        strOverWhere = "right";
                    else if (strOverWhere == "right")
                        strOverWhere = "left";
                    else if (strOverWhere == "topleft")
                        strOverWhere = "topright";
                    else if (strOverWhere == "topright")
                        strOverWhere = "topleft";
                    else if (strOverWhere == "bottomleft")
                        strOverWhere = "bottomright";
                    else if (strOverWhere == "bottomright")
                        strOverWhere = "bottomleft";
                }

            }

        }

        //void WaveformAxes::isOverUnitBox(float x, float y, int &UnitID, int &BoxID, String &where)
    }
    else  if (isOverThresholdSlider)
    {

        float thresholdSliderPosition ;
        if (signalFlipped)
            thresholdSliderPosition = (getHeight()-float(event.y)) / float(getHeight());
        else
            thresholdSliderPosition =  float(event.y) / float(getHeight());

        if (thresholdSliderPosition > 1)
            thresholdSliderPosition = 1;
        else if (thresholdSliderPosition < -1) // Modified to allow negative thresholds.
            thresholdSliderPosition =-1;


        displayThresholdLevel = (0.5f - thresholdSliderPosition) * range;
        // update processor
        
         if (processor->getEditAllState()){
             int numElectrodes = processor->getNumElectrodes();
             for (int electrodeIt = 0 ; electrodeIt < numElectrodes ; electrodeIt++){
             //processor->setChannelThreshold(electrodeList->getSelectedItemIndex(),i,slider->getValue());
                 for (int channelIt = 0 ; channelIt < processor->getNumChannels(electrodeIt) ; channelIt++){
                 processor->setChannelThreshold(electrodeIt,channelIt,displayThresholdLevel);
                 }
             }
         }
        else{
            processor->getActiveElectrode()->thresholds[channel] = displayThresholdLevel;
        }

        SpikeSorterEditor* edt = (SpikeSorterEditor*) processor->getEditor();
        for (int k=0; k<processor->getActiveElectrode()->numChannels; k++)
            edt->electrodeButtons[k]->setToggleState(false, dontSendNotification);

        edt->electrodeButtons[channel]->setToggleState(true, dontSendNotification);

        edt->setThresholdValue(channel,displayThresholdLevel);
    }
    repaint();
}

// MouseCursor WaveAxes::getMouseCursor()
// {
//     MouseCursor c = MouseCursor(cursorType);

//     return c;
// }

void WaveformAxes::mouseExit(const MouseEvent& event)
{
    if (isOverThresholdSlider)
    {
        isOverThresholdSlider = false;
        thresholdColour = Colours::red;
        repaint();
    }
}

float WaveformAxes::getDisplayThreshold()
{
    return displayThresholdLevel;
}

void WaveformAxes::setDetectorThreshold(float t)
{
    displayThresholdLevel = t;
}



void WaveformAxes::isOverUnitBox(float x, float y, int& UnitID, int& BoxID, String& where)
{

    float h = getHeight();
    float w = getWidth();
    // Map box coordinates to screen coordinates.
    // Assume time span is 40 samples at 30 Khz?
    float microsec_span = 40.0/30000.0 * 1e6;
    float microvolt_span = range/2;

    // Typical spike is 40 samples, at 30kHz ~ 1.3 ms or 1300 usecs.
    for (int k = 0; k < units.size(); k++)
    {
        for (int boxiter = 0; boxiter< units[k].lstBoxes.size(); boxiter++)
        {
            Box B = units[k].lstBoxes[boxiter];
            float rectx1 = B.x / microsec_span * w;
            float recty1 = h/2 - (B.y / microvolt_span * h/2);
            float rectx2 = (B.x+B.w) / microsec_span * w;
            float recty2 = h/2 - ((B.y-B.h) / microvolt_span * h/2);

            if (signalFlipped)
            {
                recty1 = h-recty1;
                recty2 = h-recty2;
            }

            if (rectx1 > rectx2)
                swapVariables(rectx1,rectx2);
            if (recty1 > recty2)
                swapVariables(recty1,recty2);

            if (x >= rectx1 - 10 & y >= recty1 -10 & x <= rectx2 + 10 & y <= recty2+10)
            {
                //setMouseCursor(MouseCursor::DraggingHandCursor);
                UnitID = units[k].UnitID;
                BoxID = boxiter;
                if (x >= rectx1 - 10 & x <= rectx1 + 10 && y >= recty1-10 & y <= recty1+10)
                {
                    where = "topleft";
                    setMouseCursor(MouseCursor::TopLeftCornerResizeCursor);
                }
                else if (x >= rectx2 - 10 & x <= rectx2 + 10 && y >= recty1-10 & y <= recty1+10)
                {
                    where = "topright";
                    setMouseCursor(MouseCursor::TopRightCornerResizeCursor);
                }
                else if (x >= rectx1 - 10 & x <= rectx1 + 10 && y >= recty2-10 & y <= recty2+10)
                {
                    where = "bottomleft";
                    setMouseCursor(MouseCursor::BottomLeftCornerResizeCursor);
                }
                else if (x >= rectx2 - 10 & x <= rectx2 + 10 && y >= recty2-10 & y <= recty2+10)
                {
                    where = "bottomright";
                    setMouseCursor(MouseCursor::BottomRightCornerResizeCursor);
                }
                else if (x >= rectx1 - 10 & x <= rectx1 + 10)
                {
                    where = "left";
                    setMouseCursor(MouseCursor::LeftEdgeResizeCursor);
                }
                else if (x >= rectx2 - 10 & x <= rectx2 + 10)
                {
                    where = "right";
                    setMouseCursor(MouseCursor::RightEdgeResizeCursor);
                }
                else if (y >= recty1 - 10 & y <= recty1 + 10)
                {
                    setMouseCursor(MouseCursor::TopEdgeResizeCursor);
                    where = "top";
                }
                else if (y >= recty2 - 10 & y <= recty2 + 10)
                {
                    where = "bottom";
                    setMouseCursor(MouseCursor::BottomEdgeResizeCursor);
                }
                else
                {
                    setMouseCursor(MouseCursor::DraggingHandCursor);
                    where = "inside";
                }
                return;
            }
        }
    }

    setMouseCursor(MouseCursor::NormalCursor); // not inside any boxes
}

void WaveformAxes::drawBoxes(Graphics& g)
{
    // y and h are given in micro volts.
    // x and w and given in micro seconds.

    float h = getHeight();
    float w = getWidth();
    // Map box coordinates to screen coordinates.
    // Assume time span is 40 samples at 30 Khz?
    float microsec_span = 40.0/30000.0 * 1e6;
    float microvolt_span = range/2;

    int selectedUnitID, selectedBoxID;
    processor->getActiveElectrode()->spikeSort->getSelectedUnitAndBox(selectedUnitID, selectedBoxID);

    // Typical spike is 40 samples, at 30kHz ~ 1.3 ms or 1300 usecs.
    for (int k = 0; k < units.size(); k++)
    {
        g.setColour(Colour(units[k].ColorRGB[0],units[k].ColorRGB[1],units[k].ColorRGB[2]));

        for (int boxiter = 0; boxiter < units[k].lstBoxes.size(); boxiter++)
        {
            Box B = units[k].lstBoxes[boxiter];

            float thickness = 2;
            if (units[k].getUnitID() == selectedUnitID && boxiter == selectedBoxID)
                thickness = 3;
            else if (units[k].getUnitID() == isOverUnit && boxiter == isOverBox)
                thickness = 2;
            else
                thickness = 1;


            float rectx1 = B.x / microsec_span * w;
            float recty1 = (h/2 - (B.y / microvolt_span * h/2));

            float rectx2 = (B.x+B.w) / microsec_span * w;
            float recty2 = (h/2 - ((B.y-B.h) / microvolt_span * h/2));

            //std::cout << rectx1 << " " << rectx2 << " " << recty1 << " " << recty2 << std::endl;

            float drawRecty1, drawRecty2;
            if (signalFlipped)
            {
                drawRecty2 = h-recty1;
                drawRecty1 = h-recty2;
            }
            else
            {
                drawRecty1 = recty1;
                drawRecty2 = recty2;
            }
            g.drawRect(rectx1,drawRecty1,rectx2-rectx1, drawRecty2-drawRecty1,thickness);
            g.drawText(String(units[k].UnitID), rectx1,drawRecty1-15,rectx2-rectx1,15,juce::Justification::centred,false);

        }
    }
}



void WaveformAxes::updateUnits(std::vector<BoxUnit> _units)
{
    units = _units;
}

void WaveformAxes::paint(Graphics& g)
{
    g.setColour(Colours::black);
    g.fillRect(0,0,getWidth(), getHeight());

    // int chan = 0;

    if (drawGrid)
        drawWaveformGrid(g);

    double noise = processor->getSelectedElectrodeNoise();
    String d = "STD: " + String(noise, 2) + "uV";
    g.setFont(Font("Small Text", 13, Font::plain));
    g.setColour(Colours::white);

    g.drawText(d, 10, 10, 150, 20, Justification::left, false);
    // draw the grid lines for the waveforms

    // draw the threshold line and labels
    drawThresholdSlider(g);
    drawBoxes(g);
    // if no spikes have been received then don't plot anything
    if (!gotFirstSpike)
    {
        return;
    }


    for (int spikeNum = 0; spikeNum < bufferSize; spikeNum++)
    {

        if (spikeNum != spikeIndex && spikeBuffer[spikeNum] != nullptr)
        {
            g.setColour(Colours::grey);
            plotSpike(spikeBuffer[spikeNum], g);
        }

    }

    g.setColour(Colours::white);
    if (spikeBuffer[spikeIndex] != nullptr)
        plotSpike(spikeBuffer[spikeIndex], g);

    spikesReceivedSinceLastRedraw = 0;

}

// --------------------------------------------------


PCAProjectionAxes::PCAProjectionAxes(SpikeSorter* p) : GenericDrawAxes(0), processor(p), imageDim(500),
    rangeX(250), rangeY(250), spikesReceivedSinceLastRedraw(0)
{
    projectionImage = Image(Image::RGB, imageDim, imageDim, true);
    bufferSize = 600;
    pcaMin[0] = pcaMin[1] = 0;
    pcaMax[0] = pcaMax[1] = 0;

    rangeSet = false;
    inPolygonDrawingMode = false;
    clear();
    updateProcessor = false;
    isOverUnit = -1;

    rangeUpButton = new UtilityButton("+", Font("Small Text", 10, Font::plain));
    rangeUpButton->setRadius(3.0f);
    rangeUpButton->addListener(this);
    addAndMakeVisible(rangeUpButton);

    rangeDownButton = new UtilityButton("-", Font("Small Text", 10, Font::plain));
    rangeDownButton->setRadius(3.0f);
    rangeDownButton->addListener(this);
    addAndMakeVisible(rangeDownButton);

    redrawSpikes = true;

}

void PCAProjectionAxes::resized()
{

    rangeDownButton->setBounds(10,10, 20, 15);
    rangeUpButton->setBounds(35,10, 20, 15);
}

void PCAProjectionAxes::setPolygonDrawingMode(bool on)
{
    if (on)
    {
        inPolygonDrawingMode = true;
        setMouseCursor(MouseCursor::CrosshairCursor);

    }
    else
    {
        inPolygonDrawingMode = false;
        setMouseCursor(MouseCursor::NormalCursor);
    }
}

void PCAProjectionAxes::updateUnits(std::vector<PCAUnit> _units)
{
    units = _units;
}

void PCAProjectionAxes::drawUnit(Graphics& g, PCAUnit unit)
{
    float w = getWidth();
    float h = getHeight();

    int selectedUnitID, selectedBoxID;
    processor->getActiveElectrode()->spikeSort->getSelectedUnitAndBox(selectedUnitID, selectedBoxID);
    g.setColour(Colour(unit.ColorRGB[0],unit.ColorRGB[1],unit.ColorRGB[2]));
    if (unit.poly.pts.size() > 2)
    {
        float thickness;
        if (unit.getUnitID() == selectedUnitID)
            thickness = 3;
        else if (unit.getUnitID() == isOverUnit)
            thickness = 2;
        else
            thickness = 1;

        double cx=0,cy=0;
        for (int k=0; k<unit.poly.pts.size()-1; k++)
        {
            // convert projection coordinates to screen coordinates.
            float x1 = (unit.poly.offset.X + unit.poly.pts[k].X - pcaMin[0]) / (pcaMax[0]-pcaMin[0]) * w;
            float y1 = (unit.poly.offset.Y + unit.poly.pts[k].Y - pcaMin[1]) / (pcaMax[1]-pcaMin[1]) * h;
            float x2 = (unit.poly.offset.X + unit.poly.pts[k+1].X - pcaMin[0]) / (pcaMax[0]-pcaMin[0]) * w;
            float y2 = (unit.poly.offset.Y + unit.poly.pts[k+1].Y - pcaMin[1]) / (pcaMax[1]-pcaMin[1]) * h;
            cx+=x1;
            cy+=y1;
            g.drawLine(x1,y1,x2,y2,thickness);
        }
        float x1 = (unit.poly.offset.X + unit.poly.pts[0].X - pcaMin[0]) / (pcaMax[0]-pcaMin[0]) * w;
        float y1 = (unit.poly.offset.Y + unit.poly.pts[0].Y - pcaMin[1]) / (pcaMax[1]-pcaMin[1]) * h;
        float x2 = (unit.poly.offset.X + unit.poly.pts[unit.poly.pts.size()-1].X - pcaMin[0]) / (pcaMax[0]-pcaMin[0]) * w;
        float y2 = (unit.poly.offset.Y + unit.poly.pts[unit.poly.pts.size()-1].Y - pcaMin[1]) / (pcaMax[1]-pcaMin[1]) * h;
        g.drawLine(x1,y1,x2,y2,thickness);

        cx+=x2;
        cy+=y2;

        g.drawText(String(unit.UnitID), (cx/unit.poly.pts.size())-10,(cy/unit.poly.pts.size())-10,20,15,juce::Justification::centred,false);
    }
}

void PCAProjectionAxes::paint(Graphics& g)
{

    spikesReceivedSinceLastRedraw = 0;

    g.drawImage(projectionImage,
                0, 0, getWidth(), getHeight(),
                0, 0, rangeX, rangeY);


    // draw pca units polygons
    for (int k=0; k<units.size(); k++)
    {
        drawUnit(g, units[k]);
    }


    if (inPolygonDrawingMode)
    {
        setMouseCursor(MouseCursor::CrosshairCursor);
        // draw polygon
        bool first = true;
        PointD prev;

        if (drawnPolygon.size() > 0)
        {
            g.setColour(Colour(drawnUnit.ColorRGB[0],drawnUnit.ColorRGB[1],drawnUnit.ColorRGB[2]));

            for (std::list<PointD>::iterator it = drawnPolygon.begin(); it != drawnPolygon.end(); it++)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    g.drawLine((*it).X, (*it).Y, prev.X,prev.Y);
                }
                prev = *it;
            }

            g.drawLine(drawnPolygon.front().X,drawnPolygon.front().Y,drawnPolygon.back().X,drawnPolygon.back().Y);
        }
    }

    //Graphics im(projectionImage);

    if (redrawSpikes)
    {
        // recompute image
        //int w = getWidth();
        //int h = getHeight();
        projectionImage.clear(juce::Rectangle<int>(0, 0, projectionImage.getWidth(), projectionImage.getHeight()),
                              Colours::black);

        bool subsample = false;
        int dk = (subsample) ? 5 : 1;

        for (int k=0; k<bufferSize; k+=dk)
        {
            drawProjectedSpike(spikeBuffer[k]);
        }
        redrawSpikes = false;
    }

}


void PCAProjectionAxes::drawProjectedSpike(SorterSpikePtr s)
{
    if (s != nullptr && rangeSet)
    {
        Graphics g(projectionImage);

        g.setColour(Colour(s->color[0],s->color[1],s->color[2]));

        float x = (s->pcProj[0] - pcaMin[0]) / (pcaMax[0]-pcaMin[0]) * rangeX;
        float y = (s->pcProj[1] - pcaMin[1]) / (pcaMax[1]-pcaMin[1]) * rangeY;
        if (x >= 0 & y >= 0 & x <= rangeX & y <= rangeY)
            g.fillEllipse(x,y,2,2);
    }
}

void PCAProjectionAxes::redraw(bool subsample)
{
    Graphics g(projectionImage);

    // recompute image
    //int w = getWidth();
    //int h = getHeight();
    projectionImage.clear(juce::Rectangle<int>(0, 0, projectionImage.getWidth(), projectionImage.getHeight()),
                          Colours::black);

    int dk = (subsample) ? 5 : 1;

    for (int k=0; k<bufferSize; k+=dk)
    {
        drawProjectedSpike(spikeBuffer[k]);
    }

}

void PCAProjectionAxes::setPCARange(float p1min, float p2min, float p1max, float p2max)
{

    pcaMin[0] = p1min;
    pcaMin[1] = p2min;
    pcaMax[0] = p1max;
    pcaMax[1] = p2max;
    rangeSet = true;
    redrawSpikes = true;
    processor->getActiveElectrode()->spikeSort->setPCArange(p1min,p2min, p1max,  p2max);

}

bool PCAProjectionAxes::updateSpikeData(SorterSpikePtr s)
{

    if (spikesReceivedSinceLastRedraw < bufferSize)
    {

        spikeIndex++;
        spikeIndex %= bufferSize;

        spikeBuffer.set(spikeIndex, s);

        spikesReceivedSinceLastRedraw++;
        //drawProjectedSpike(newSpike);
        redrawSpikes = true;

    }
    return true;
}


void PCAProjectionAxes::clear()
{
    projectionImage.clear(juce::Rectangle<int>(0, 0, projectionImage.getWidth(), projectionImage.getHeight()),
                          Colours::black);


    spikeBuffer.clear();
    spikeIndex = 0;

    redrawSpikes = true;
    //repaint();
}


void PCAProjectionAxes::mouseDrag(const juce::MouseEvent& event)
{

    if (!inPolygonDrawingMode)
    {

        setMouseCursor(MouseCursor::DraggingHandCursor);
        int selectedUnitID, selectedBoxID;
        processor->getActiveElectrode()->spikeSort->getSelectedUnitAndBox(selectedUnitID, selectedBoxID);

        if (isOverUnit > 0 && selectedUnitID == isOverUnit)
        {
            // pan unit
            int unitindex=-1;

            for (int k=0; k<units.size(); k++)
            {
                if (units[k].getUnitID() == selectedUnitID)
                {
                    unitindex = k;
                    break;
                }
            }
            jassert(unitindex >= 0);

            int w = getWidth();
            int h = getHeight();
            float range0 = pcaMax[0]-pcaMin[0];
            float range1 = pcaMax[1]-pcaMin[1];

            float dx = float(event.x-prevx) / w*range0;
            float dy = float(event.y-prevy) / h*range1;


            units[unitindex].poly.offset.X += dx;
            units[unitindex].poly.offset.Y += dy;
            updateProcessor = true;
            // draw polygon
            prevx = event.x;
            prevy = event.y;

        }
        else
        {
            // Pan PCA space
            int w = getWidth();
            int h = getHeight();
            float range0 = pcaMax[0]-pcaMin[0];
            float range1 = pcaMax[1]-pcaMin[1];

            float dx = -float(event.x-prevx) / w*range0;
            float dy = -float(event.y-prevy) / h*range1;

            pcaMin[0]+=dx;
            pcaMin[1]+=dy;
            pcaMax[0]+=dx;
            pcaMax[1]+=dy;
            processor->getActiveElectrode()->spikeSort->setPCArange(pcaMin[0],pcaMin[1], pcaMax[0],  pcaMax[1]);

            // draw polygon
            prevx = event.x;
            prevy = event.y;

            redrawSpikes = true;
        }

    }
    else
    {
        int pixel_quantizer = 6;
        float distance = float(event.x-prevx)*float(event.x-prevx)+
                         float(event.y-prevy)*float(event.y-prevy);
        if (distance > pixel_quantizer*pixel_quantizer)  // add a point every n pixels.
        {
            drawnPolygon.push_back(PointD(event.x,event.y));
            // draw polygon
            prevx = event.x;
            prevy = event.y;

            repaint();
        }
    }

}

void PCAProjectionAxes::mouseUp(const juce::MouseEvent& event)
{
    repaint();
    //redraw(false);
    setMouseCursor(MouseCursor::NormalCursor);
    if	(updateProcessor)
    {
        processor->getActiveElectrode()->spikeSort->updatePCAUnits(units);
        updateProcessor = false;

    }

    if (inPolygonDrawingMode)
    {
        inPolygonDrawingMode = false;
        SpikeSorterEditor* edt = (SpikeSorterEditor*)processor->getEditor();
        edt->spikeSorterCanvas->addPolygonUnitButton->setToggleState(false, dontSendNotification);

        // convert pixel coordinates to pca space coordinates and update unit
        cPolygon poly;
        poly.pts.resize(drawnPolygon.size());
        int k=0;

        float w = getWidth();
        float h = getHeight();
        float range0 = pcaMax[0]-pcaMin[0];
        float range1 = pcaMax[1]-pcaMin[1];

        for (std::list<PointD>::iterator it = drawnPolygon.begin(); it != drawnPolygon.end(); it++,k++)
        {
            poly.pts[k].X = (*it).X / w * range0 + pcaMin[0];
            poly.pts[k].Y = (*it).Y / h * range1 + pcaMin[1];
        }
        drawnUnit.poly = poly;
        units.push_back(drawnUnit);
        // add a new PCA unit
        Electrode* e = processor->getActiveElectrode();
        e->spikeSort->addPCAunit(drawnUnit);


        uint8 r,g,b;
        e->spikeSort->getUnitColor(drawnUnit.getUnitID(), r,g,b);

        processor->addNewUnit(e->electrodeID, drawnUnit.getUnitID(),r,g,b);

        drawnPolygon.clear();
    }
}


void PCAProjectionAxes::mouseMove(const juce::MouseEvent& event)
{
    isOverUnit = -1;
    float w = getWidth();
    float h = getHeight();

    for (int k=0; k<units.size(); k++)
    {
        // convert projection coordinates to screen coordinates.
        float x1 = ((float)event.x/w) * (pcaMax[0]-pcaMin[0]) +  pcaMin[0];
        float y1 = ((float)event.y/h) * (pcaMax[1]-pcaMin[1]) +  pcaMin[1];
        if (units[k].isPointInsidePolygon(PointD(x1,y1)))
        {
            isOverUnit = units[k].getUnitID();
            break;
        }

    }


}


void PCAProjectionAxes::mouseDown(const juce::MouseEvent& event)
{
    prevx = event.x;
    prevy = event.y;
    if (event.mods.isRightButtonDown())
    {
        clear();
    }
    if (inPolygonDrawingMode)
    {
        drawnUnit = PCAUnit(processor->getActiveElectrode()->spikeSort->generateUnitID(),processor->getActiveElectrode()->spikeSort->generateLocalID());
        drawnPolygon.push_back(PointD(event.x,event.y));
    }
    else
    {
        if (isOverUnit > 0)
            processor->getActiveElectrode()->spikeSort->setSelectedUnitAndBox(isOverUnit, -1);
        else
            processor->getActiveElectrode()->spikeSort->setSelectedUnitAndBox(-1, -1);
    }
}


bool PCAProjectionAxes::keyPressed(const KeyPress& key)
{
    KeyPress e = KeyPress::createFromDescription("escape");

    if (key.isKeyCode(e.getKeyCode()) && inPolygonDrawingMode) // C
    {
        inPolygonDrawingMode = false;
        setMouseCursor(MouseCursor::NormalCursor);
        return true;
    }
    return false;
}

void PCAProjectionAxes::rangeDown()
{
    float range0 = pcaMax[0]-pcaMin[0];
    float range1 = pcaMax[1]-pcaMin[1];
    pcaMin[0] = pcaMin[0] - 0.1 * range0;
    pcaMax[0] = pcaMax[0] + 0.1 * range0;
    pcaMin[1] = pcaMin[1] - 0.1 * range1;
    pcaMax[1] = pcaMax[1] + 0.1 * range1;
    setPCARange(pcaMin[0], pcaMin[1], pcaMax[0], pcaMax[1]);
}

void PCAProjectionAxes::rangeUp()
{
    float range0 = pcaMax[0]-pcaMin[0];
    float range1 = pcaMax[1]-pcaMin[1];
    pcaMin[0] = pcaMin[0] + 0.1 * range0;
    pcaMax[0] = pcaMax[0] - 0.1 * range0;
    pcaMin[1] = pcaMin[1] + 0.1 * range1;
    pcaMax[1] = pcaMax[1] - 0.1 * range1;

    setPCARange(pcaMin[0], pcaMin[1], pcaMax[0], pcaMax[1]);

}

void PCAProjectionAxes::buttonClicked(Button* button)
{
    if (button == rangeDownButton)
    {
        rangeDown();
    }

    else if (button == rangeUpButton)
    {
        rangeUp();
    }

}

void PCAProjectionAxes::mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel)
{
    if (wheel.deltaY > 0)
        rangeDown();
    else
        rangeUp();
}
