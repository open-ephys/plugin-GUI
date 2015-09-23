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


ArduinoOutputEditor::ArduinoOutputEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{

    // accumulator = 0;

    desiredWidth = 150;

    arduino = (ArduinoOutput*) parentNode;

    vector <ofSerialDeviceInfo> devices = serial.getDeviceList();

    // Image im;
    // im = ImageCache::getFromMemory(BinaryData::ArduinoIcon_png,
    //                                BinaryData::ArduinoIcon_pngSize);

    // icon = new ImageIcon(im);
    // addAndMakeVisible(icon);
    // icon->setBounds(75,15,50,50);

    // icon->setOpacity(0.3f);

    deviceSelector = new ComboBox();
    deviceSelector->setBounds(10,105,125,20);
    deviceSelector->addListener(this);
    deviceSelector->addItem("Device",1);
    
    for (int i = 0; i < devices.size(); i++)
    {
        deviceSelector->addItem(devices[i].getDevicePath(),i+2);
    }

    deviceSelector->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(deviceSelector);

    inputChannelSelector = new ComboBox();
    inputChannelSelector->setBounds(10,30,55,20);
    inputChannelSelector->addListener(this);
    inputChannelSelector->addItem("Trig",1);

    for (int i = 0; i < 16; i++)
        inputChannelSelector->addItem(String(i+1),i+2); // start numbering at one for
    // user-visible channels

    inputChannelSelector->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(inputChannelSelector);

    outputChannelSelector = new ComboBox();
    outputChannelSelector->setBounds(10,80,80,20);
    outputChannelSelector->addListener(this);
    outputChannelSelector->addItem("Output CH",1);

    for (int i = 1; i < 13; i++)
        outputChannelSelector->addItem(String(i+1),i+2);

    outputChannelSelector->setSelectedId(14, dontSendNotification);
    addAndMakeVisible(outputChannelSelector);

    gateChannelSelector = new ComboBox();
    gateChannelSelector->setBounds(10,55,55,20);
    gateChannelSelector->addListener(this);
    gateChannelSelector->addItem("Gate",1);

    for (int i = 0; i < 16; i++)
        gateChannelSelector->addItem(String(i+1),i+2); // start numbering at one for
    // user-visible channels

    gateChannelSelector->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(gateChannelSelector);

}

ArduinoOutputEditor::~ArduinoOutputEditor()
{
}

void ArduinoOutputEditor::receivedEvent()
{

    icon->setOpacity(0.8f);
    startTimer(50);

}

void ArduinoOutputEditor::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == deviceSelector)
    {
        arduino->setDevice(deviceSelector->getText());
    } else if (comboBoxThatHasChanged == outputChannelSelector)
    {
        arduino->setOutputChannel(outputChannelSelector->getSelectedId()-1);
    } else if (comboBoxThatHasChanged == inputChannelSelector)
    {
        arduino->setInputChannel(inputChannelSelector->getSelectedId()-1);
    } else if (comboBoxThatHasChanged == gateChannelSelector)
    {
        arduino->setGateChannel(gateChannelSelector->getSelectedId()-1);
    }
}

void ArduinoOutputEditor::timerCallback()
{

    repaint();

    accumulator++;

    if (isFading)
    {

        if (accumulator > 15.0)
        {
            stopTimer();
            isFading = false;
        }

    }
    else
    {

        if (accumulator < 10.0)
        {
            icon->setOpacity(0.8f-(0.05*float(accumulator)));
            accumulator++;
        }
        else
        {
            icon->setOpacity(0.3f);
            stopTimer();
            accumulator = 0;
        }
    }
}