/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

RecordControlEditor::RecordControlEditor (GenericProcessor* parentNode)
	: GenericEditor(parentNode)
{
	desiredWidth = 170;

	channelSelector->eventsOnly = true;

	chanSel = new Label("Chanel Text","Available Event Channels");
	chanSel->setEditable(false);
	chanSel->setJustificationType(Justification::centredLeft);
	chanSel->setBounds(20,30,120,20);

	addAndMakeVisible(chanSel);

	
	availableChans = new ComboBox("Event Channels");

	availableChans->setEditableText(false);
	availableChans->setJustificationType(Justification::centredLeft);
	availableChans->addListener(this);
	availableChans->setBounds(20,60,120,20);
	availableChans->setSelectedId(0);

	addAndMakeVisible(availableChans);

	
}

RecordControlEditor::~RecordControlEditor()
{
	deleteAllChildren();
}

void RecordControlEditor::comboBoxChanged(ComboBox* comboBox)
{
	RecordControl *processor = (RecordControl*)getProcessor();
	if (comboBox->getSelectedId() > 0)
		processor->updateTriggerChannel(processor->eventChannels[comboBox->getSelectedId()-1]->num );
	else
		processor->updateTriggerChannel(-1);
}

void RecordControlEditor::updateSettings()
{
	availableChans->clear();
	GenericProcessor* processor = getProcessor();
	for (int i = 0; i < processor->eventChannels.size() ; i++)
		availableChans->addItem(processor->eventChannels[i]->name,i+1);

}