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

    intputChannelLabel = new Label("input", "Input channel:");
    intputChannelLabel->setBounds(15,25,180,20);
    intputChannelLabel->setFont(Font("Small Text", 12, Font::plain));
    intputChannelLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(intputChannelLabel);

    outputChannelLabel = new Label("output", "Output channel:");
    outputChannelLabel->setBounds(15,75,180,20);
    outputChannelLabel->setFont(Font("Small Text", 12, Font::plain));
    outputChannelLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(outputChannelLabel);

    inputChannelSelectionBox = new ComboBox();
    inputChannelSelectionBox->setBounds(15,45,150,25);
    inputChannelSelectionBox->addListener(this);
    inputChannelSelectionBox->addItem("None", 1);
    inputChannelSelectionBox->setSelectedId(1, false);
    addAndMakeVisible(inputChannelSelectionBox);

    outputChannelSelectionBox = new ComboBox();
    outputChannelSelectionBox->setBounds(15,95,150,25);
    outputChannelSelectionBox->addListener(this);
    outputChannelSelectionBox->addItem("None", 1);

    for (int chan = 0; chan < 10; chan++)
    {
        outputChannelSelectionBox->addItem(String(chan+1), chan+2);
    }

    outputChannelSelectionBox->setSelectedId(5, false); // default output channel is 4
    addAndMakeVisible(outputChannelSelectionBox);

}

PhaseDetectorEditor::~PhaseDetectorEditor()
{

}

void PhaseDetectorEditor::updateSettings()
{

    if (getProcessor()->getNumInputs() != previousChannelCount)
    {
        inputChannelSelectionBox->clear();

        inputChannelSelectionBox->addItem("None", 1);

        for (int i = 0; i < getProcessor()->getNumInputs(); i++)
        {
            inputChannelSelectionBox->addItem("Channel " + String(i+1), i+2);

        }

        previousChannelCount = getProcessor()->getNumInputs();

        inputChannelSelectionBox->setSelectedId(1, false);

        getProcessor()->setParameter(1,-1.0f);

    }

}

void PhaseDetectorEditor::comboBoxChanged(ComboBox* c)
{

    float channel;

     int id = c->getSelectedId();

     if (id == 1)
     {
        channel = -1.0f;
     }
        else
     {
        channel = float(id) - 2.0f;
     }

    if (c == inputChannelSelectionBox)
    {
        getProcessor()->setParameter(1,channel);
    } else if (c == outputChannelSelectionBox) {

        getProcessor()->setParameter(2,channel);

    }

}

void PhaseDetectorEditor::buttonEvent(Button* button)
{


}

void PhaseDetectorEditor::saveEditorParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "PhaseDetectorEditor");

    XmlElement* selectedChannel = xml->createNewChildElement("SELECTEDID");

    selectedChannel->setAttribute("INPUTCHANNEL",inputChannelSelectionBox->getSelectedId());
    selectedChannel->setAttribute("OUTPUTCHANNEL",outputChannelSelectionBox->getSelectedId());

}

void PhaseDetectorEditor::loadEditorParameters(XmlElement* xml)
{

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("SELECTEDID"))
        {

            inputChannelSelectionBox->setSelectedId(xmlNode->getIntAttribute("INPUTCHANNEL"));
            outputChannelSelectionBox->setSelectedId(xmlNode->getIntAttribute("OUTPUTCHANNEL"));

        }
    }
}