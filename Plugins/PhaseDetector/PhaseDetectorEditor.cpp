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
    desiredWidth = 220;

    addSelectedChannelsParameterEditor("input_channel", 0, 20);
    addComboBoxParameterEditor("output_bit", 0, 50);
    addComboBoxParameterEditor("gate_bit", 0, 80);

    Parameter* param = getProcessor()->getParameter(0, "phase");
    addCustomParameterEditor(new DetectorInterface(param), 100, 10);

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
        phaseButton->setToggleState(false, dontSendNotification);
        phaseButton->setRadioGroupId(12);
        phaseButton->addListener(this);
        phaseButtons.add(phaseButton);
        addAndMakeVisible(phaseButton);
    }

    setBounds(0, 0, 80, 80);
}

void DetectorInterface::buttonClicked(Button* b)
{

    ElectrodeButton* pb = (ElectrodeButton*) b;

    int i = phaseButtons.indexOf(pb);

    param->setNextValue(i);

}

void DetectorInterface::updateView()
{
    phaseButtons[(int)param->getValue()]->setToggleState(true, dontSendNotification);
}


void DetectorInterface::paint(Graphics& g)
{
    g.setColour(Colours::red);
    g.strokePath(sineWave, PathStrokeType(3.0f));
}
