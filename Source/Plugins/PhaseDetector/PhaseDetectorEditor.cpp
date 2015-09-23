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



PhaseDetectorEditor::PhaseDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors), previousChannelCount(-1)

{
    desiredWidth = 220;

    // intputChannelLabel = new Label("input", "Input channel:");
    // intputChannelLabel->setBounds(15,25,180,20);
    // intputChannelLabel->setFont(Font("Small Text", 12, Font::plain));
    // intputChannelLabel->setColour(Label::textColourId, Colours::darkgrey);
    // addAndMakeVisible(intputChannelLabel);

    // outputChannelLabel = new Label("output", "Output channel:");
    // outputChannelLabel->setBounds(15,75,180,20);
    // outputChannelLabel->setFont(Font("Small Text", 12, Font::plain));
    // outputChannelLabel->setColour(Label::textColourId, Colours::darkgrey);
    // addAndMakeVisible(outputChannelLabel);

    // inputChannelSelectionBox = new ComboBox();
    // inputChannelSelectionBox->setBounds(15,45,150,25);
    // inputChannelSelectionBox->addListener(this);
    // inputChannelSelectionBox->addItem("None", 1);
    // inputChannelSelectionBox->setSelectedId(1, false);
    // addAndMakeVisible(inputChannelSelectionBox);

    detectorSelector = new ComboBox();
    detectorSelector->setBounds(35,30,150,20);
    detectorSelector->addListener(this);

    addAndMakeVisible(detectorSelector);

    plusButton = new UtilityButton("+", titleFont);
    plusButton->addListener(this);
    plusButton->setRadius(3.0f);
    plusButton->setBounds(10,30,20,20);

    addAndMakeVisible(plusButton);

    backgroundColours.add(Colours::green);
    backgroundColours.add(Colours::red);
    backgroundColours.add(Colours::orange);
    backgroundColours.add(Colours::magenta);
    backgroundColours.add(Colours::blue);

    plusButton->setToggleState(true, sendNotification);

    //interfaces.clear();

}

PhaseDetectorEditor::~PhaseDetectorEditor()
{

}

void PhaseDetectorEditor::updateSettings()
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

void PhaseDetectorEditor::comboBoxChanged(ComboBox* c)
{

    for (int i = 0; i < interfaces.size(); i++)
    {

        if (i == c->getSelectedId()-1)
        {
            interfaces[i]->setVisible(true);
        }
        else
        {
            interfaces[i]->setVisible(false);
        }

    }

}

void PhaseDetectorEditor::buttonEvent(Button* button)
{
    if (button == plusButton && interfaces.size() < 5)
    {

        addDetector();

    }

}

void PhaseDetectorEditor::addDetector()
{
    std::cout << "Adding detector" << std::endl;

    PhaseDetector* pd = (PhaseDetector*) getProcessor();

    int detectorNumber = interfaces.size()+1;

    DetectorInterface* di = new DetectorInterface(pd, backgroundColours[detectorNumber%5], detectorNumber-1);
    di->setBounds(10,50,190,80);

    addAndMakeVisible(di);

    interfaces.add(di);

    String itemName = "Detector ";
    itemName += detectorNumber;

    //jassert detectorNumber == 1;

    //std::cout << itemName << std::endl;

    detectorSelector->addItem(itemName, detectorNumber);
    detectorSelector->setSelectedId(detectorNumber, dontSendNotification);

    for (int i = 0; i < interfaces.size()-1; i++)
    {
        interfaces[i]->setVisible(false);
    }

}

void PhaseDetectorEditor::saveCustomParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "PhaseDetectorEditor");

    for (int i = 0; i < interfaces.size(); i++)
    {
        XmlElement* d = xml->createNewChildElement("DETECTOR");
        d->setAttribute("PHASE",interfaces[i]->getPhase());
        d->setAttribute("INPUT",interfaces[i]->getInputChan());
        d->setAttribute("GATE",interfaces[i]->getGateChan());
        d->setAttribute("OUTPUT",interfaces[i]->getOutputChan());
    }
}

void PhaseDetectorEditor::loadCustomParameters(XmlElement* xml)
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

            interfaces[i]->setPhase(xmlNode->getIntAttribute("PHASE"));
            interfaces[i]->setInputChan(xmlNode->getIntAttribute("INPUT"));
            interfaces[i]->setGateChan(xmlNode->getIntAttribute("GATE"));
            interfaces[i]->setOutputChan(xmlNode->getIntAttribute("OUTPUT"));

            i++;
        }
    }
}


// ===================================================================

DetectorInterface::DetectorInterface(PhaseDetector* pd, Colour c, int id) :
    backgroundColour(c), idNum(id), processor(pd)
{

    font = Font("Small Text", 10, Font::plain);

    const double PI = 3.14159265;

    sineWave.startNewSubPath(5,35);

    std::cout << "Creating sine wave" << std::endl;

    for (double i = 0; i < 2*PI; i += PI/10)
    {
        sineWave.lineTo(i*12 + 5.0f, -sin(i)*20 + 35);
    }

    sineWave.addEllipse(2,35,4,4);

    std::cout << "Creating phase buttons" << std::endl;

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

    std::cout << "Creating combo boxes" << std::endl;


    inputSelector = new ComboBox();
    inputSelector->setBounds(140,5,50,20);
    inputSelector->addItem("-",1);
    inputSelector->setSelectedId(1);
    inputSelector->addListener(this);
    addAndMakeVisible(inputSelector);

    gateSelector = new ComboBox();
    gateSelector->setBounds(140,30,50,20);
    gateSelector->addItem("-",1);
    gateSelector->addListener(this);

    std::cout << "Populating combo boxes" << std::endl;


    for (int i = 1; i < 9; i++)
    {
        gateSelector->addItem(String(i),i+1);
    }

    gateSelector->setSelectedId(1);
    addAndMakeVisible(gateSelector);

    outputSelector = new ComboBox();
    outputSelector->setBounds(140,55,50,20);
    outputSelector->addItem("-",1);
    outputSelector->addListener(this);

    for (int i = 1; i < 9; i++)
    {
        outputSelector->addItem(String(i),i+1);
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


    processor->setParameter(parameterIndex, (float) c->getSelectedId() - 2);

}

void DetectorInterface::buttonClicked(Button* b)
{

    ElectrodeButton* pb = (ElectrodeButton*) b;

    int i = phaseButtons.indexOf(pb);

    processor->setActiveModule(idNum);

    processor->setParameter(1, (float) i+1);

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
        inputSelector->addItem(String(i+1), i+2);

    }

    inputSelector->setSelectedId(1, dontSendNotification);
}

void DetectorInterface::paint(Graphics& g)
{
    g.setColour(backgroundColour);
    g.strokePath(sineWave, PathStrokeType(3.0f));

    g.setColour(Colours::darkgrey);
    g.setFont(font);
    g.drawText("INPUT",50,10,85,10,Justification::right, true);
    g.drawText("GATE",50,35,85,10,Justification::right, true);
    g.drawText("OUTPUT",50,60,85,10,Justification::right, true);

}

int DetectorInterface::getPhase()
{

    for (int i = 0; i < phaseButtons.size(); i++)
    {
        if (phaseButtons[i]->getToggleState())
        {
            return i;
        }
    }

    return -1;
}

void DetectorInterface::setPhase(int p)
{

    if (p >= 0)
        phaseButtons[p]->setToggleState(true, dontSendNotification);


    processor->setActiveModule(idNum);

    processor->setParameter(1, (float) p+1);

}

void DetectorInterface::setInputChan(int chan)
{
    inputSelector->setSelectedId(chan+2);

    processor->setParameter(2, (float) chan);
}

void DetectorInterface::setOutputChan(int chan)
{
    outputSelector->setSelectedId(chan+2);

    processor->setParameter(3, (float) chan);
}

void DetectorInterface::setGateChan(int chan)
{
    gateSelector->setSelectedId(chan+2);

    processor->setParameter(4, (float) chan);
}

int DetectorInterface::getInputChan()
{
    return inputSelector->getSelectedId()-2;
}

int DetectorInterface::getOutputChan()
{
    return outputSelector->getSelectedId()-2;
}

int DetectorInterface::getGateChan()
{
    return gateSelector->getSelectedId()-2;
}