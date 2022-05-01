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

#include "SpikeDisplayEditor.h"

#include "SpikeDisplayNode.h"
#include "SpikeDisplayCanvas.h"

#include <string>

SpikeDisplayEditor::SpikeDisplayEditor(GenericProcessor* parentNode)
    : VisualizerEditor(parentNode, "Spikes", 180)

{

    scaleLabel = std::make_unique<Label>("Display size: ", "Display size:");
    scaleLabel->setBounds(50, 40, 100, 25);
    addAndMakeVisible(scaleLabel.get());

    scaleDownBtn = std::make_unique<UtilityButton>("-", Font("Small Text", 30, Font::plain));
    scaleDownBtn->setBounds(65, 75, 25, 25);
    scaleDownBtn->addListener(this);
    addAndMakeVisible(scaleDownBtn.get());

    scaleUpBtn = std::make_unique<UtilityButton>("+", Font("Small Text", 30, Font::plain));
    scaleUpBtn->setBounds(100, 75, 25, 25);
    scaleUpBtn->addListener(this);
    addAndMakeVisible(scaleUpBtn.get());

    scaleFactors.add(0.5f);
    scaleFactors.add(0.6f);
    scaleFactors.add(0.7f);
    scaleFactors.add(0.8f);
    scaleFactors.add(0.9f);
    scaleFactors.add(1.0f);
    scaleFactors.add(1.1f);
    scaleFactors.add(1.3f);
    scaleFactors.add(1.5f);

    selectedScaleFactor = 5;

}


Visualizer* SpikeDisplayEditor::createNewCanvas()
{

    SpikeDisplayNode* processor = (SpikeDisplayNode*) getProcessor();

    SpikeDisplayCanvas* sdc = new SpikeDisplayCanvas(processor);
    sdc->setPlotScaleFactor(scaleFactors[selectedScaleFactor]);

    return sdc;

}

void SpikeDisplayEditor::buttonClicked(Button* button)
{
    
    if (button == scaleUpBtn.get())
    {
        selectedScaleFactor += 1;

        if (selectedScaleFactor == scaleFactors.size() - 1)
            scaleUpBtn->setEnabled(false);
        
        if (selectedScaleFactor > 0)
            scaleDownBtn->setEnabled(true);

        
    }  
    else if (button == scaleDownBtn.get())
    {
        selectedScaleFactor -= 1;

        if (selectedScaleFactor == 0)
            scaleDownBtn->setEnabled(false);

        if (selectedScaleFactor < scaleFactors.size() - 1)
            scaleUpBtn->setEnabled(true);
    }

    if (canvas != nullptr)
    {
        SpikeDisplayCanvas* sdc = (SpikeDisplayCanvas*)canvas.get();
        sdc->setPlotScaleFactor(scaleFactors[selectedScaleFactor]);
    }
}


void SpikeDisplayEditor::saveVisualizerEditorParameters(XmlElement* xml)
{
    xml->setAttribute("scale_factor_index", selectedScaleFactor);
}


void SpikeDisplayEditor::loadVisualizerEditorParameters(XmlElement* xml)
{
    selectedScaleFactor = xml->getIntAttribute("scale_factor_index", 5);

    if (selectedScaleFactor < 0 || selectedScaleFactor >= scaleFactors.size())
        selectedScaleFactor = 5;

    if (canvas != nullptr)
    {
        SpikeDisplayCanvas* sdc = (SpikeDisplayCanvas*)canvas.get();
        sdc->setPlotScaleFactor(scaleFactors[selectedScaleFactor]);
    }
}
