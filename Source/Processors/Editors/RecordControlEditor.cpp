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

#include "RecordControlEditor.h"
#include "../Utilities/RecordControl.h"
#include "ChannelSelector.h"
#include <stdio.h>

RecordControlEditor::RecordControlEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)
{
    desiredWidth = 170;

    //channelSelector->eventsOnly = true;

    chanSel = new Label("Chanel Text","Trigger Channel:");
    chanSel->setEditable(false);
    chanSel->setJustificationType(Justification::centredLeft);
    chanSel->setBounds(15,35,120,20);

    addAndMakeVisible(chanSel);


    availableChans = new ComboBox("Event Channels");

    availableChans->setEditableText(false);
    availableChans->setJustificationType(Justification::centredLeft);
    availableChans->addListener(this);
    availableChans->setBounds(20,60,120,20);
    availableChans->setSelectedId(0);

    addAndMakeVisible(availableChans);
    
    newFileToggleButton = new UtilityButton("SPLIT FILES", Font("Small Text", 13, Font::plain));
    newFileToggleButton->setRadius(3.0f);
    newFileToggleButton->setBounds(35, 95, 90, 18);
    newFileToggleButton->addListener(this);
    newFileToggleButton->setClickingTogglesState(true);
    addAndMakeVisible(newFileToggleButton);

}

RecordControlEditor::~RecordControlEditor()
{
    //deleteAllChildren();
}

void RecordControlEditor::comboBoxChanged(ComboBox* comboBox)
{

    if (comboBox->getSelectedId() > 0)
        getProcessor()->setParameter(0, (float) comboBox->getSelectedId()-1);
    else
        getProcessor()->setParameter(0, -1);
}

void RecordControlEditor::buttonEvent(Button* button)
{

    if (button->getToggleState())
    {
        getProcessor()->setParameter(1, 1.0f);
    } else {
        getProcessor()->setParameter(1, 0.0f);
    }
}

void RecordControlEditor::updateSettings()
{
    availableChans->clear();
    //GenericProcessor* processor = getProcessor();
    for (int i = 0; i < 10 ; i++)
    {
        String channelName = "Channel ";
        channelName += i + 1;
        availableChans->addItem(channelName,i+1);
    }

}