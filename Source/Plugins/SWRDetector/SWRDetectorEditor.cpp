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


#include "SWRDetectorEditor.h"
#include "SWRDetector.h"

#include <stdio.h>
#include <cmath>



SWRDetectorEditor::SWRDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors = true)
    : GenericEditor(parentNode, useDefaultParameterEditors), previousChannelCount(-1)

{
    desiredWidth = 250;

    detectorSelector = new ComboBox();
    detectorSelector->setBounds(35, 30, 150, 20);
    detectorSelector->addListener(this);

    addAndMakeVisible(detectorSelector);

    plusButton = new UtilityButton("+", titleFont);
    plusButton->addListener(this);
    plusButton->setRadius(3.0f);
    plusButton->setBounds(10, 30, 20, 20);

    addAndMakeVisible(plusButton);

    backgroundColours.add(Colours::green);
    backgroundColours.add(Colours::red);
    backgroundColours.add(Colours::orange);
    backgroundColours.add(Colours::magenta);
    backgroundColours.add(Colours::blue);

    plusButton->setToggleState(true, sendNotification);

    thresholdConstLabel = new Label("thresholdConst", "3.0");
    thresholdConstLabel->setBounds(85, 60, 40, 20);
    thresholdConstLabel->setFont(Font("Small Text", 12, Font::plain));
    thresholdConstLabel->setColour(Label::textColourId, Colours::white);
    thresholdConstLabel->setColour(Label::backgroundColourId, Colours::grey);
    thresholdConstLabel->setEditable(true);
    thresholdConstLabel->addListener(this);
    addAndMakeVisible(thresholdConstLabel);

    eventStimulationTimeLabel = new Label("thresholdTime", "0.05");
    eventStimulationTimeLabel->setBounds(85, 90, 40, 20);
    eventStimulationTimeLabel->setFont(Font("Small Text", 12, Font::plain));
    eventStimulationTimeLabel->setColour(Label::textColourId, Colours::white);
    eventStimulationTimeLabel->setColour(Label::backgroundColourId, Colours::grey);
    eventStimulationTimeLabel->setEditable(true);
    eventStimulationTimeLabel->addListener(this);
    addAndMakeVisible(eventStimulationTimeLabel);


    setEnabledState(false);

}

SWRDetectorEditor::~SWRDetectorEditor()
{

}


void SWRDetectorEditor::labelTextChanged(juce::Label* label)
{
    SWRDetector* processor = (SWRDetector*)getProcessor();
    int parameterIndex;
    Value val = label->getTextValue();

    if (label == thresholdConstLabel)
    {
        parameterIndex = 5;
    }

    else if (label == eventStimulationTimeLabel)
    {
        parameterIndex = 6;
    }

    processor->setParameter(parameterIndex, float(val.getValue()));


}


void SWRDetectorEditor::updateSettings()
{
    if (getProcessor()->getNumInputs() != previousChannelCount)
    {

        for (int i = 0; i < interfaces.size(); i++)
        {
            interfaces[i]->updateChannels(getProcessor()->getNumInputs());
        }
    }

    previousChannelCount = getProcessor()->getNumInputs();

}


void SWRDetectorEditor::comboBoxChanged(ComboBox* c)
{

    for (int i = 0; i < interfaces.size(); i++)
    {

        if (i == c->getSelectedId() - 1)
        {
            interfaces[i]->setVisible(true);
        }
        else
        {
            interfaces[i]->setVisible(false);
        }

    }

}

void SWRDetectorEditor::buttonEvent(Button* button)
{
    if (button == plusButton && interfaces.size() < 7)
    {
        addDetector();
    }

}

void SWRDetectorEditor::addDetector()
{
    std::cout << "Adding detector" << std::endl;

    SWRDetector* pd = (SWRDetector*)getProcessor();

    int detectorNumber = interfaces.size() + 1;

    DetectorInterface* di = new DetectorInterface(pd, backgroundColours[detectorNumber % 5], detectorNumber - 1);
    di->setBounds(10, 50, 220, 80);

    addAndMakeVisible(di);

    interfaces.add(di);

    String itemName = "Detector ";
    itemName += detectorNumber;

    detectorSelector->addItem(itemName, detectorNumber);
    detectorSelector->setSelectedId(detectorNumber, dontSendNotification);

    for (int i = 0; i < interfaces.size() - 1; i++)
    {
        interfaces[i]->setVisible(false);
    }

}

void SWRDetectorEditor::saveCustomParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "SWRDetectorEditor");

    for (int i = 0; i < interfaces.size(); i++)
    {
        XmlElement* d = xml->createNewChildElement("DETECTOR");
        d->setAttribute("INPUT", interfaces[i]->getInputChan());
        d->setAttribute("GATE", interfaces[i]->getGateChan());
        d->setAttribute("OUTPUT", interfaces[i]->getOutputChan());
        {

        };
    }
}

void SWRDetectorEditor::loadCustomParameters(XmlElement* xml)
{
    int i = 0;

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("DETECTOR"))
        {

            if (i > 0)
            {
                addDetector();
            }

            interfaces[i]->setInputChan(xmlNode->getIntAttribute("INPUT"));
            interfaces[i]->setGateChan(xmlNode->getIntAttribute("GATE"));
            interfaces[i]->setOutputChan(xmlNode->getIntAttribute("OUTPUT"));

            i++;
        }
    }
}


// ===================================================================

DetectorInterface::DetectorInterface(SWRDetector* pd, Colour c, int id) :
    backgroundColour(c), idNum(id), processor(pd)
{

    font = Font("Small Text", 10, Font::plain);

    std::cout << "Creating combo boxes" << std::endl;

    inputSelector = new ComboBox();
    inputSelector->setBounds(170, 5, 50, 20);
    inputSelector->addItem("-", 1);
    inputSelector->setSelectedId(1);
    inputSelector->addListener(this);
    addAndMakeVisible(inputSelector);

    gateSelector = new ComboBox();
    gateSelector->setBounds(170, 30, 50, 20);
    gateSelector->addItem("-", 1);
    gateSelector->addListener(this);

    std::cout << "Populating combo boxes" << std::endl;


    for (int i = 1; i < 9; i++)
    {
        gateSelector->addItem(String(i), i + 1);
    }

    gateSelector->setSelectedId(1);
    addAndMakeVisible(gateSelector);

    outputSelector = new ComboBox();
    outputSelector->setBounds(170, 55, 50, 20);
    outputSelector->addItem("-", 1);
    outputSelector->addListener(this);

    for (int i = 1; i < 9; i++)
    {
        outputSelector->addItem(String(i), i + 1);
    }
    outputSelector->setSelectedId(1);
    addAndMakeVisible(outputSelector);

    std::cout << "Updating channels" << std::endl;

    updateChannels(processor->getNumInputs());

    std::cout << "Updating processor" << std::endl;

    processor->addModule();

}

DetectorInterface::~DetectorInterface()
{

}

void DetectorInterface::comboBoxChanged(ComboBox* c)
{

    processor->setActiveModule(idNum);

    int parameterIndex;

    if (c == inputSelector)
    {
        parameterIndex = 2;
    }
    else if (c == outputSelector)
    {
        parameterIndex = 3;
    }
    else if (c == gateSelector)
    {
        parameterIndex = 4;
    }

    processor->setParameter(parameterIndex, (float)c->getSelectedId() - 2);

}

void DetectorInterface::buttonClicked(Button* b)
{

    ElectrodeButton* pb = (ElectrodeButton*)b;

    int i = phaseButtons.indexOf(pb);

    processor->setActiveModule(idNum);

    processor->setParameter(1, (float)i + 1);

}

void DetectorInterface::updateChannels(int numChannels)
{

    inputSelector->clear();

    inputSelector->addItem("-", 1);

    if (numChannels > 2048) // channel count hasn't been updated yet
    {
        return;
    }

    for (int i = 0; i < numChannels; i++)
    {
        inputSelector->addItem(String(i + 1), i + 2);

    }

    inputSelector->setSelectedId(1, dontSendNotification);
}

void DetectorInterface::paint(Graphics& g)
{
    g.setColour(backgroundColour);

    g.setColour(Colours::darkgrey);
    g.setFont(font);
    g.drawText("INPUT", 80, 10, 85, 10, Justification::right, true);
    g.drawText("GATE", 80, 35, 85, 10, Justification::right, true);
    g.drawText("OUTPUT", 80, 60, 85, 10, Justification::right, true);

    g.drawText("THRESHOLD", 0, 10, 85, 10, Justification::left, true);
    g.drawText("CONSTANT", 0, 20, 85, 10, Justification::left, true);

    g.drawText("EVENT", 0, 40, 85, 10, Justification::left, true);
    g.drawText("STIMULATION", 0, 50, 85, 10, Justification::left, true);
    g.drawText("TIME (s)", 0, 60, 85, 10, Justification::left, true);
}


void DetectorInterface::setInputChan(int chan)
{
    inputSelector->setSelectedId(chan + 2);

    processor->setParameter(2, (float)chan);
}

void DetectorInterface::setOutputChan(int chan)
{
    outputSelector->setSelectedId(chan + 2);

    processor->setParameter(3, (float)chan);
}

void DetectorInterface::setGateChan(int chan)
{
    gateSelector->setSelectedId(chan + 2);

    processor->setParameter(4, (float)chan);
}


int DetectorInterface::getInputChan()
{
    return inputSelector->getSelectedId() - 2;
}

int DetectorInterface::getOutputChan()
{
    return outputSelector->getSelectedId() - 2;
}

int DetectorInterface::getGateChan()
{
    return gateSelector->getSelectedId() - 2;
}



