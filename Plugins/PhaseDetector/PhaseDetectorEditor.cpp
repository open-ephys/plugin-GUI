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


#include "PhaseDetectorEditor.h"
#include "PhaseDetector.h"

#include <stdio.h>
#include <cmath>


PhaseDetectorEditor::PhaseDetectorEditor(GenericProcessor* parentNode)
    : GenericEditor(parentNode)

{
    desiredWidth = 200;

    Parameter* param = getProcessor()->getStreamParameter("phase");
    addCustomParameterEditor(new DetectorInterface(param), 10, 40);

    addSelectedChannelsParameterEditor(Parameter::STREAM_SCOPE, "Channel", 110, 20);
    getParameterEditor("Channel")->setLayout(ParameterEditor::Layout::nameOnTop);
    ((TextButton*)getParameterEditor("Channel")->getEditor())->setButtonText("1");

    addComboBoxParameterEditor(Parameter::STREAM_SCOPE, "TTL_out", 110, 55);
    getParameterEditor("TTL_out")->setLayout(ParameterEditor::Layout::nameOnTop);

    addComboBoxParameterEditor(Parameter::STREAM_SCOPE, "gate_line", 110, 90);
    getParameterEditor("gate_line")->setLayout(ParameterEditor::Layout::nameOnTop);

}


DetectorInterface::DetectorInterface(Parameter* param) : ParameterEditor(param)
{

    const double PI = 3.14159265;

    sineWave.startNewSubPath(5,35);

    for (double i = 0; i < 2*PI; i += PI/10)
    {
        sineWave.lineTo(i*12 + 5.0f, -sin(i)*20 + 35);
    }

    sineWave.addEllipse(2,35,4,4);

    for (int phase = 0; phase < 4; phase++)
    {
        ElectrodeButton* phaseButton = new ElectrodeButton(-1);

        double theta = PI/2+phase*PI/2;

        phaseButton->setBounds(theta*12+1.0f, -sin(theta)*20 + 31, 8, 8);

        phaseButton->setRadioGroupId(12);

        if (phase == 0)
            phaseButton->setToggleState(true, dontSendNotification);
        else
            phaseButton->setToggleState(false, dontSendNotification);
        
        phaseButton->addListener(this);
        phaseButtons.add(phaseButton);
        addAndMakeVisible(phaseButton);
    }

    setBounds(0, 0, 100, 65);
}

void DetectorInterface::buttonClicked(Button* b)
{

    if (param == nullptr)
        return;

    if (b->getState())
    {
        ElectrodeButton* pb = (ElectrodeButton*)b;

        int i = phaseButtons.indexOf(pb);

        param->setNextValue(i);
    }
 
}

void DetectorInterface::updateView()
{
    for (int i = 0; i < 4; i++)
    {
        if (param != nullptr)
        {
            phaseButtons[i]->setClickingTogglesState(true);

            if (i == (int)param->getValue())
                phaseButtons[i]->setToggleState(true, dontSendNotification);
            else
                phaseButtons[i]->setToggleState(false, dontSendNotification);
        }
        else {
            phaseButtons[i]->setClickingTogglesState(false);
        }
        
    }
    
}


void DetectorInterface::paint(Graphics& g)
{
    g.setColour(Colours::red);
    g.strokePath(sineWave, PathStrokeType(3.0f));
}
