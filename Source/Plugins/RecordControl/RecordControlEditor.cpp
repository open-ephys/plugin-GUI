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
#include "RecordControl.h"
#include <stdio.h>

RecordControlEditor::RecordControlEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)
{
    desiredWidth = 170;

    //channelSelector->eventsOnly = true;


    triggerLabel = new Label("Trigger Text", "Trigger Type:");
    triggerLabel->setEditable(false);
    triggerLabel->setJustificationType(Justification::centredLeft);
    triggerLabel->setBounds(15, 20, 120, 20);

    addAndMakeVisible(triggerLabel);

    chanSel = new Label("Channel Text","Trigger Channel:");
    chanSel->setEditable(false);
    chanSel->setJustificationType(Justification::centredLeft);
    chanSel->setBounds(15,60,120,20);

    addAndMakeVisible(chanSel);

    polLabel = new Label("Polarity Text", "Edge:");
    polLabel->setEditable(false);
    polLabel->setJustificationType(Justification::centredLeft);
    polLabel->setBounds(15, 105, 120, 20);

    addAndMakeVisible(polLabel);

    triggerMode = new ComboBox("Mode");

    triggerMode->setEditableText(false);
    triggerMode->setJustificationType(Justification::centredLeft);
    triggerMode->addListener(this);
    triggerMode->setBounds(20, 40, 120, 20);
    triggerMode->setSelectedId(0);

    addAndMakeVisible(triggerMode);


    availableChans = new ComboBox("Event Channels");

    availableChans->setEditableText(false);
    availableChans->setJustificationType(Justification::centredLeft);
    availableChans->addListener(this);
    availableChans->setBounds(20,80,120,20);
    availableChans->setSelectedId(0);

    addAndMakeVisible(availableChans);

    triggerPol = new ComboBox("Polarity");

    triggerPol->setEditableText(false);
    triggerPol->setJustificationType(Justification::centredLeft);
    triggerPol->addListener(this);
    triggerPol->setBounds(60, 105, 80, 20);
    triggerPol->setSelectedId(0);

    addAndMakeVisible(triggerPol);

    availableChans->addItem("None",1);
  /*  for (int i = 0; i < 10 ; i++)
    {
        String channelName = "Channel ";
        channelName += i + 1;
        availableChans->addItem(channelName,i+2);
    }*/
    availableChans->setSelectedId(1, sendNotification);

    triggerMode->addItem("Edge set", 1);
    triggerMode->addItem("Edge toggle", 2);
    triggerMode->setSelectedId(1, sendNotification);

    triggerPol->addItem("Rising", 1);
    triggerPol->addItem("Falling", 2);
    triggerPol->setSelectedId(1, sendNotification);
}

RecordControlEditor::~RecordControlEditor()
{

}

void RecordControlEditor::comboBoxChanged(ComboBox* comboBox)
{

    if (comboBox == availableChans)
    {
		if (comboBox->getSelectedId() > 1)
		{
			int index = comboBox->getSelectedId() - 2;
			//invalidate the input first to ensure there are no race conditions
			getProcessor()->setParameter(0, -1);
			getProcessor()->setParameter(1, eventSourceArray[index].channel);
			getProcessor()->setParameter(0, eventSourceArray[index].eventIndex);
		}
        else
            getProcessor()->setParameter(0, -1);
    }
    else if (comboBox == triggerMode)
    {
        getProcessor()->setParameter(2, comboBox->getSelectedId());
    }
    else if (comboBox == triggerPol)
    {
        getProcessor()->setParameter(3, comboBox->getSelectedId());
    }
}


void RecordControlEditor::updateSettings()
{
	EventSources s;
	String name;
	int oldId = availableChans->getSelectedId();
    availableChans->clear();
	eventSourceArray.clear();
    GenericProcessor* processor = getProcessor();
	availableChans->addItem("None", 1);
	int nextItem = 2;
	int nEvents = processor->getTotalEventChannels();
	for (int i = 0; i < nEvents; i++)
	{
		const EventChannel* event = processor->getEventChannel(i);
		if (event->getChannelType() == EventChannel::TTL)
		{
			s.eventIndex = i;
			int nChans = event->getNumChannels();
			for (int c = 0; c < nChans; c++)
			{
				s.channel = c;
				name = event->getSourceName() + " (TTL" + String(c+1) + ")";
				eventSourceArray.add(s);
				availableChans->addItem(name, nextItem++);
			}
		}
	}
	if (oldId > availableChans->getNumItems())
	{
		oldId = 1;
	}
	availableChans->setSelectedId(oldId, sendNotification);
}

void RecordControlEditor::saveCustomParameters(XmlElement* xml)
{

    XmlElement* info = xml->createNewChildElement("PARAMETERS");

    info->setAttribute("Type", "RecordControlEditor");
    info->setAttribute("Channel",availableChans->getSelectedId());
    info->setAttribute("Mode", triggerMode->getSelectedId());
    info->setAttribute("Edge", triggerPol->getSelectedId());

}

void RecordControlEditor::loadCustomParameters(XmlElement* xml)
{

    forEachXmlChildElement(*xml, xmlNode)
    {

        if (xmlNode->hasTagName("PARAMETERS"))
        {
            availableChans->setSelectedId(xmlNode->getIntAttribute("Channel"), sendNotification);
            triggerMode->setSelectedId(xmlNode->getIntAttribute("Mode", 1), sendNotification);
            triggerPol->setSelectedId(xmlNode->getIntAttribute("Edge", 1), sendNotification);
        }

    }
}