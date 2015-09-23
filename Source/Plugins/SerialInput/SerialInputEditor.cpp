/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2014 Florian Franzen

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

#include "SerialInputEditor.h"
#include "SerialInput.h"

#include <string>
#include <iostream>


SerialInputEditor::SerialInputEditor(SerialInput* parentNode)
    : GenericEditor(parentNode, false)
{
    node = parentNode;

    desiredWidth = 180;

    // Add device list
    deviceList = new ComboBox();
    deviceList->setBounds(10,30,150,25);
    deviceList->addListener(this);
    deviceList->addItemList(node->getDevices(), 1);

    addAndMakeVisible(deviceList);


    // Add baudrate list
    baudrateList = new ComboBox();
    baudrateList->setBounds(10,60,80,25);
    baudrateList->addListener(this);

    Array<int> baudrates = node->getBaudrates();

    for (int i = 0; i < baudrates.size(); i++)
    {
        baudrateList->addItem(String(baudrates[i]), baudrates[i]);
    }

    addAndMakeVisible(baudrateList);

    // Add refresh button
    refreshButton = new UtilityButton("REFRESH", Font("Small Text", 13, Font::bold));
    refreshButton->setRadius(3.0f);
    refreshButton->setBounds(95, 60, 65, 25);
    refreshButton->addListener(this);

    addAndMakeVisible(refreshButton);
}

void SerialInputEditor::startAcquisition()
{
    // Disable the whole gui
    deviceList->setEnabled(false);
    baudrateList->setEnabled(false);
    refreshButton->setEnabled(false);
    GenericEditor::startAcquisition();
}

void SerialInputEditor::stopAcquisition()
{
    // Reenable the whole gui
    deviceList->setEnabled(true);
    baudrateList->setEnabled(true);
    refreshButton->setEnabled(true);
    GenericEditor::stopAcquisition();
}


void SerialInputEditor::buttonEvent(Button* button)
{
    // Refresh list of devices
    deviceList->clear();
    deviceList->addItemList(node->getDevices(), 1);
}

void SerialInputEditor::comboBoxChanged(ComboBox* comboBox)
{
    // Push new selection to parent node
    if (comboBox == deviceList)
    {
        node->setDevice(comboBox->getText().toStdString());
    }
    else if (comboBox == baudrateList)
    {
        node->setBaudrate(comboBox->getSelectedId());
    }
}

void SerialInputEditor::saveEditorParameters(XmlElement* xmlNode)
{
    XmlElement* parameters = xmlNode->createNewChildElement("PARAMETERS");

    parameters->setAttribute("device", deviceList->getText().toStdString());
    parameters->setAttribute("baudrate", baudrateList->getSelectedId());
}

void SerialInputEditor::loadEditorParameters(XmlElement* xmlNode)
{
    forEachXmlChildElement(*xmlNode, subNode)
    {
        if (subNode->hasTagName("PARAMETERS"))
        {
            deviceList->setText(subNode->getStringAttribute("device", ""));
            baudrateList->setSelectedId(subNode->getIntAttribute("baudrate"));
        }
    }
}

