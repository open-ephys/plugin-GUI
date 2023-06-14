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

    desiredWidth = 240;

    vector <ofSerialDeviceInfo> devices = serial.getDeviceList();

    deviceSelector = std::make_unique<ComboBox>();
    deviceSelector->setBounds(15, 40, 110, 20);
    deviceSelector->addListener(this);
    deviceSelector->addItem("Device",1);
    
    for (int i = 0; i < devices.size(); i++)
    {
        deviceSelector->addItem(devices[i].getDevicePath(),i+2);
    }

    deviceSelector->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(deviceSelector.get());

    addComboBoxParameterEditor(Parameter::GLOBAL_SCOPE, "output_pin", 15, 70);
    addComboBoxParameterEditor(Parameter::STREAM_SCOPE, "input_line", 144, 35);
    addComboBoxParameterEditor(Parameter::STREAM_SCOPE, "gate_line", 144, 80);
}


void ArduinoOutputEditor::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == deviceSelector.get())
    {
        ArduinoOutput* processor = (ArduinoOutput*) getProcessor();
        processor->setDevice(deviceSelector->getText());
        CoreServices::updateSignalChain(this);
    }
}

void ArduinoOutputEditor::updateDevice(String deviceName)
{
    for (int i = 0; i < deviceSelector->getNumItems(); i++)
    {
        if (deviceSelector->getItemText(i).equalsIgnoreCase(deviceName))
            deviceSelector->setSelectedId(deviceSelector->getItemId(i), dontSendNotification);
    }
}
