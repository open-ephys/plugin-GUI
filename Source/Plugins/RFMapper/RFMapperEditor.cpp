/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

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

#include "RFMapperEditor.h"

RFMapperEditor::RFMapperEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
: VisualizerEditor(parentNode, useDefaultParameterEditors)

{
    
    tabText = "Mapper";
    
    desiredWidth = 180;

    electrodeSelector = new ComboBox();
    electrodeSelector->setBounds(35,40,100,20);
    electrodeSelector->addListener(this);

    addAndMakeVisible(electrodeSelector);

    unitSelector = new ComboBox();
    unitSelector->setBounds(35,70,100,20);
    unitSelector->addListener(this);

    addAndMakeVisible(unitSelector);
    
}

RFMapperEditor::~RFMapperEditor()
{
}


Visualizer* RFMapperEditor::createNewCanvas()
{
    
    RFMapper* processor = (RFMapper*) getProcessor();
    canvas = new RFMapperCanvas(processor);
    return canvas;
    
}

void RFMapperEditor::comboBoxChanged(ComboBox* c)
{

    RFMapperCanvas* rfmc = (RFMapperCanvas*) canvas.get();
    
    if (c == electrodeSelector)
    {
        rfmc->setCurrentElectrode(c->getSelectedId()-1);
    } else if (c == unitSelector)
    {
        rfmc->setCurrentUnit(c->getSelectedId()-1);
    }
    
}

void RFMapperEditor::updateSettings()
{
    RFMapper* processor = (RFMapper*) getProcessor();

    int numElectrodes = processor->getNumElectrodes();
    
    electrodeSelector->clear();
    unitSelector->clear();

    if (numElectrodes > 0)
    {

        for (int i = 1; i <= numElectrodes; i++)
        {
            String itemName = "Electrode ";
            itemName += String(i);
            std::cout << i << " " << itemName << std::endl;
            electrodeSelector->addItem(itemName, i);
        }

        electrodeSelector->setSelectedId(1, dontSendNotification);

        unitSelector->addItem("MUA", 1);
        
        for (int i = 1; i <= 5; i++)
        {
            String itemName = "Unit ";
            itemName += String(i);
            unitSelector->addItem(itemName, i + 1);
        }

        unitSelector->setSelectedId(1, dontSendNotification);
    }

    updateVisualizer();
}


RFMapperCanvas::RFMapperCanvas(RFMapper* sr) : processor(sr), currentMap(0)
{

    rfMaps.clear();
}


RFMapperCanvas::~RFMapperCanvas(){
    
}
    
void RFMapperCanvas::beginAnimation()
{
    startCallbacks();
    std::cout << "RF Mapper beginning animation." << std::endl;

}

void RFMapperCanvas::endAnimation()
{
    stopCallbacks();
}
    
void RFMapperCanvas::refreshState()
{
}

void RFMapperCanvas::update()
{
    int nMaps = processor->getNumElectrodes();

    clearRfMaps();

    for (int i = 0; i < nMaps; i++)
    {
        RFMap* rf = addRFMap(i);
        processor->addRFMapForElectrode(rf, i);
    }

    addAndMakeVisible(rfMaps[currentMap]);

    resized();
    repaint();

}
    
void RFMapperCanvas::setParameter(int, float) {}

void RFMapperCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::grey);
}
    
void RFMapperCanvas::refresh()
{
    processor->setParameter(2, 0.0f); // request redraw
    
    repaint();

    //::cout << "Refresh" << std::endl;
}
    
void RFMapperCanvas::resized()
{
    if (rfMaps.size() > 0)
        rfMaps[currentMap]->setBounds(0, 0, getWidth(), getHeight());

    repaint();
}
    

void RFMapperCanvas::clearRfMaps()
{
    rfMaps.clear();

}

RFMap* RFMapperCanvas::addRFMap(int electrodeNum)
{

    std::cout << "Adding new rf map." << std::endl;

    RFMap* rfMap = new RFMap(this, electrodeNum);
    rfMaps.add(rfMap);

    return rfMap;
}

void RFMapperCanvas::setCurrentElectrode(int electrodeNum)
{
    currentMap = electrodeNum;

    update();
}

void RFMapperCanvas::setCurrentUnit(int unitNum)
{
    rfMaps[currentMap]->setCurrentUnit(unitNum);
}

RFMap::RFMap(RFMapperCanvas*, int elecNum)
{
    map = AudioSampleBuffer(50,30);

    random = Random();

    unitId = 0;

    reset();
}

RFMap::~RFMap()
{

}

void RFMap::reset()
{
    float numXPixels = map.getNumChannels();
    float numYPixels = map.getNumSamples();

    for (int n = 0; n < numXPixels; n++)
    {
        for (int m = 0; m < numYPixels; m++)
        {
            map.setSample(n,m, 0.);
        }
    }

    repaint();
}

void RFMap::paint(Graphics& g)
{
    float numXPixels = map.getNumChannels();
    float numYPixels = map.getNumSamples();

    float xHeight = getWidth()/numXPixels;
    float yHeight = getHeight()/numYPixels;

    for (int n = 0; n < numXPixels; n++)
    {
        for (int m = 0; m < numYPixels; m++)
        {
            float colourIndex = map.getSample(n,m);

            if (colourIndex == 0.)
            {
                g.setColour(Colours::grey);
            } else if (colourIndex == 1.)
            {
                g.setColour(Colours::white);
            } else {
                g.setColour(Colours::black);
            }

            g.fillRect(n*xHeight, m*yHeight, xHeight, yHeight);
        }
    }
}
void RFMap::resized()
{

}

void RFMap::setCurrentUnit(int unit)
{
    unitId = unit;
}

void RFMap::processSpikeObject(const SpikeObject& s)
{

    //std::cout << "Got spike." << std::endl;

    if (s.sortedId == unitId)
    {

        float numXPixels = map.getNumChannels();
        float numYPixels = map.getNumSamples();

        for (int n = 0; n < numXPixels; n++)
        {
            for (int m = 0; m < numYPixels; m++)
            {
                float colourIndex = random.nextFloat();

                if (colourIndex < 0.33)
                {
                    map.setSample(n,m,-1.);
                } else if (colourIndex > 0.33 && colourIndex < 0.66)
                {
                    map.setSample(n,m, 0.);
                } else {
                    map.setSample(n,m,1.);
                }

            }
        }

    }

    

}
