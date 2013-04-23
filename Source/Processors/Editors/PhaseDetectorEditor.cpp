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

#include <stdio.h>


PhaseDetectorEditor::PhaseDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors), previousChannelCount(0)

{
    desiredWidth = 180;

    channelSelectionBox = new ComboBox();
    channelSelectionBox->setBounds(15,50,150,25);
    channelSelectionBox->addListener(this);
    channelSelectionBox->addItem("None", 1);
    channelSelectionBox->setSelectedId(1, false);
    addAndMakeVisible(channelSelectionBox);

}

PhaseDetectorEditor::~PhaseDetectorEditor()
{

}

void PhaseDetectorEditor::updateSettings()
{

    if (getProcessor()->getNumInputs() != previousChannelCount)
    {
        channelSelectionBox->clear();

        channelSelectionBox->addItem("None", 1);

        for (int i = 0; i < getProcessor()->getNumInputs(); i++)
        {
            channelSelectionBox->addItem("Channel " + String(i+1), i+2);

        }

        previousChannelCount = getProcessor()->getNumInputs();

    }

    channelSelectionBox->setSelectedId(1, false);

    getProcessor()->setParameter(1,-1.0f);
}

void PhaseDetectorEditor::comboBoxChanged(ComboBox* c)
{
    float channel;

    int id = c->getSelectedId();

    if (id == 1)
    {
        channel = -1.0f; 
    } else {
        channel = float(id) - 2.0f;
    }

    getProcessor()->setParameter(1,channel);

}

void PhaseDetectorEditor::buttonEvent(Button* button)
{


}