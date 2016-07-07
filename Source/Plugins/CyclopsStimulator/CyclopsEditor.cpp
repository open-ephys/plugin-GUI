/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

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

#include "CyclopsEditor.h"

namespace cyclops {
    
CyclopsEditor::CyclopsEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors)
    : VisualizerEditor   (parentNode, 240, useDefaultParameterEditors)
    //, progress(0, 1.0, 1000)
    , serialLED(new IndicatorLED(CyclopsColours::disconnected, Colours::black))
    , readinessLED(new IndicatorLED(CyclopsColours::notReady, Colours::black))
{
    node = (CyclopsProcessor*)parentNode;
    tabText = "Cyclops";
    // Add "port" list
    portList = new ComboBox();
    portList->setBounds(desiredWidth-13-60, 39, 60, 18);
    portList->addListener(this);
    portList->setTooltip("Select the serial port connected to Cyclops.");
    portList->addItemList(node->getDevices(), 1);
    addAndMakeVisible(portList);

    // Add baudrate list
    baudrateList = new ComboBox();
    baudrateList->setBounds(desiredWidth-13-60, 63, 60, 18);
    baudrateList->addListener(this);
    baudrateList->setTooltip("Set the baud rate (115200 recommended).");

    Array<int> baudrates(node->getBaudrates());
    for (int i = 0; i < baudrates.size(); i++)
    {
        baudrateList->addItem(String(baudrates[i]), baudrates[i]);
    }
    addAndMakeVisible(baudrateList);

    // Add refresh button
    refreshButton = new UtilityButton("R", Font("Default", 9, Font::plain));
    refreshButton->setRadius(3.0f);
    refreshButton->setBounds(145, 51, 20, 20);
    refreshButton->addListener(this);
    addAndMakeVisible(refreshButton);

    // Add LEDs
    serialLED->setBounds(169, 6, 12, 12);
    readinessLED->setBounds(183, 6, 12, 12);
    addAndMakeVisible(serialLED);
    addAndMakeVisible(readinessLED);

    // communicate with teensy.
}

CyclopsEditor::~CyclopsEditor()
{
    ;
}


Visualizer* CyclopsEditor::createNewCanvas()
{
    return new CyclopsCanvas(node);
}

/**
The listener methods that reacts to the button click. The same method is called for all buttons
on the editor, so the button variable, which cointains a pointer to the button that called the method
has to be checked to know which function to perform.
*/
void CyclopsEditor::buttonEvent(Button* button)
{
    if (button == refreshButton)
    {
        // Refresh list of devices
        portList->clear();
        portList->addItemList(node->getDevices(), 1);
        GenericEditor::repaint();
    }
}

void CyclopsEditor::comboBoxChanged(ComboBox* comboBox)
{
    // Push new selection to parent node
    if (comboBox == portList)
    {
        node->setDevice(comboBox->getText().toStdString());
    }
    else if (comboBox == baudrateList)
    {
        node->setBaudrate(comboBox->getSelectedId());
    }
}

void CyclopsEditor::timerCallback()
{
}

void CyclopsEditor::paint(Graphics& g)
{
    GenericEditor::paint(g);
    /*
    g.setColour(Colour(193, 208, 69));
    g.fillEllipse(170, 7, 10, 10);
    g.setColour(Colour(0, 0, 0));
    g.drawEllipse(169, 6, 12, 12, 1);

    g.setColour(Colour(193, 208, 69));
    g.fillEllipse(184, 7, 10, 10);
    g.setColour(Colour(0, 0, 0));
    g.drawEllipse(183, 6, 12, 12, 1);*/
}

void CyclopsEditor::disableAllInputWidgets()
{
    // Disable the whole gui
    portList->setEnabled(false);
    baudrateList->setEnabled(false);
    refreshButton->setEnabled(false);
}

void CyclopsEditor::enableAllInputWidgets()
{
    // Reenable the whole gui
    portList->setEnabled(true);
    baudrateList->setEnabled(true);
    refreshButton->setEnabled(true);
}

void CyclopsEditor::startAcquisition()
{
    disableAllInputWidgets();
    GenericEditor::startAcquisition();
}

void CyclopsEditor::stopAcquisition()
{
    enableAllInputWidgets();
    GenericEditor::stopAcquisition();
}

void CyclopsEditor::updateSettings()
{
    ;
}

void CyclopsEditor::saveEditorParameters(XmlElement* xmlNode)
{
    XmlElement* parameters = xmlNode->createNewChildElement("PARAMETERS");

    parameters->setAttribute("device", portList->getText().toStdString());
    parameters->setAttribute("baudrate", baudrateList->getSelectedId());
}

void CyclopsEditor::loadEditorParameters(XmlElement* xmlNode)
{
    forEachXmlChildElement(*xmlNode, subNode)
    {
        if (subNode->hasTagName("PARAMETERS"))
        {
            portList->setText(subNode->getStringAttribute("device", ""));
            baudrateList->setSelectedId(subNode->getIntAttribute("baudrate"));
        }
    }
}

/*
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 INDICATOR LEDS                                |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

IndicatorLED::IndicatorLED(const Colour& fill, const Colour& line)
{
    fillColour = fill;
    lineColour = line;
}

void IndicatorLED::paint(Graphics& g)
{
    g.setColour(fillColour);
    g.fillEllipse(1, 1, getWidth()-2, getHeight()-2);
    g.setColour(lineColour);
    g.drawEllipse(1, 1, getWidth()-2, getHeight()-2, 1.2);
}

void IndicatorLED::update(const Colour& fill, String& tooltip)
{
    fillColour = fill;
    setTooltip(tooltip);
}

void IndicatorLED::update(const Colour& fill, const Colour& line, String& tooltip)
{
    fillColour = fill;
    lineColour = line;
    setTooltip(tooltip);
}

} // NAMESPACE cyclops
