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

#include "ArduinoOutputEditor.h"
#include <stdio.h>


ArduinoOutputEditor::ArduinoOutputEditor(GenericProcessor* parentNode)
    : GenericEditor(parentNode)

{

    desiredWidth = 150;

    vector <ofSerialDeviceInfo> devices = serial.getDeviceList();

    deviceSelector = new ComboBox();
    deviceSelector->setBounds(10, 105, 125, 20);
    deviceSelector->addListener(this);
    deviceSelector->addItem("Device",1);
    
    for (int i = 0; i < devices.size(); i++)
    {
        deviceSelector->addItem(devices[i].getDevicePath(),i+2);
    }

    deviceSelector->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(deviceSelector);

    addComboBoxParameterEditor("output_pin", 10, 10);
    addComboBoxParameterEditor("input_bit", 10, 30);
    addComboBoxParameterEditor("gate_bit", 10, 50);
}

ArduinoOutputEditor::~ArduinoOutputEditor()
{
}


void ArduinoOutputEditor::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == deviceSelector)
    {
        ArduinoOutput* processor = (ArduinoOutput*) getProcessor();
        processor->setDevice(deviceSelector->getText());
    }
}
