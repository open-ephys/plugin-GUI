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

#include "NetworkEventsEditor.h"
#include "NetworkEvents.h"

#include <stdio.h>

NetworkEventsEditor::NetworkEventsEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
	desiredWidth = 180;

    urlLabel = new Label("Port", "Port:");
    urlLabel->setBounds(20,80,140,25);
    addAndMakeVisible(urlLabel);
	NetworkEvents *p= (NetworkEvents *)getProcessor();

	restartConnection = new UtilityButton("Restart Connection",Font("Default", 15, Font::plain));
    restartConnection->setBounds(20,45,150,18);
    restartConnection->addListener(this);
    addAndMakeVisible(restartConnection);

	
	/*
	trialSimulation = new UtilityButton("Trial",Font("Default", 15, Font::plain));
    trialSimulation->setBounds(20,25,80,18);
    trialSimulation->addListener(this);
    addAndMakeVisible(trialSimulation);

	
	startRecord = new UtilityButton("Start Record",Font("Default", 15, Font::plain));
    startRecord->setBounds(20,55,100,18);
    startRecord->addListener(this);
    addAndMakeVisible(startRecord);
	*/

	labelPort = new Label("Port", String(p->urlport));
    labelPort->setBounds(70,85,80,18);
    labelPort->setFont(Font("Default", 15, Font::plain));
    labelPort->setColour(Label::textColourId, Colours::white);



//		NetworkEvents *processor  = (NetworkEvents*) getProcessor();

	//if (processor->threadRunning)
		labelPort->setColour(Label::backgroundColourId, Colours::grey);
//	else
//		labelPort->setColour(Label::backgroundColourId, Colours::red);


    labelPort->setEditable(true);
    labelPort->addListener(this);
    addAndMakeVisible(labelPort);

    setEnabledState(false);

}



void NetworkEventsEditor::buttonEvent(Button* button)
{
			//NetworkEvents *processor  = (NetworkEvents*) getProcessor();
	if (button == restartConnection)
	{
		NetworkEvents *p= (NetworkEvents *)getProcessor();
		p->setNewListeningPort(p->urlport);
	}
			/*
	if (button == trialSimulation)
	{
		processor->simulateSingleTrial();

	} else if (button == startRecord)
	{
		if (startRecord->getLabel() == "Start Record") 
		{
			processor->simulateStartRecord();
			startRecord->setLabel("Stop Record");
		} else if (startRecord->getLabel() == "Stop Record") 
		{
			processor->simulateStopRecord();
			startRecord->setLabel("Start Record");
		}
	}
    */

}

void NetworkEventsEditor::setLabelColor(juce::Colour color)
{
	labelPort->setColour(Label::backgroundColourId, color);
}


void NetworkEventsEditor::labelTextChanged(juce::Label *label)
{
	if (label == labelPort)
	{
	   Value val = label->getTextValue();

		NetworkEvents *p= (NetworkEvents *)getProcessor();
		p->setNewListeningPort(val.getValue());
	}
}


NetworkEventsEditor::~NetworkEventsEditor()
{

}


