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
#include <string>


ArduinoOutputEditor::ArduinoOutputEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)
    , cachedInputChannel    ("bogus")
    , cachedOutputChannel   ("bogus")
    , cachedGateChannel     ("bogus")
    , cachedDevice          ("bogus")
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
    inputChannelSelector->setBounds(10,30,125,20);
    inputChannelSelector->addListener(this);
    inputChannelSelector->addItem("(no trigger)",-1);

    inputChannelSelector->setSelectedItemIndex(0, dontSendNotification);
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
    gateChannelSelector->setBounds(10,55,125,20);
    gateChannelSelector->addListener(this);
    gateChannelSelector->addItem("(no gate)",-1);

    gateChannelSelector->setSelectedItemIndex(0, dontSendNotification);
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
        cachedDevice = deviceSelector->getText();
    } else if (comboBoxThatHasChanged == outputChannelSelector)
    {
        arduino->setParameter(ARDOUT_PARAM_DIGOUT,
            outputChannelSelector->getSelectedId()-1);
        cachedOutputChannel = outputChannelSelector->getText();
    } else if (comboBoxThatHasChanged == inputChannelSelector)
    {
        int lutidx = inputChannelSelector->getSelectedId();
        cachedInputChannel = inputChannelSelector->getText();
        // Entry 0 is special; valid values are 1..(size-1).
        if ( (lutidx > 0) && (lutidx < ttlBankIdxLUT.size()) )
        {
            arduino->setParameter(ARDOUT_PARAM_INBANKIDX,
                ttlBankIdxLUT[lutidx]);
            arduino->setParameter(ARDOUT_PARAM_INBIT, ttlBitLUT[lutidx]);
        }
        else
        {
            arduino->setParameter(ARDOUT_PARAM_INBANKIDX, -1);
            arduino->setParameter(ARDOUT_PARAM_INBIT, -1);
        }
    } else if (comboBoxThatHasChanged == gateChannelSelector)
    {
        int lutidx = gateChannelSelector->getSelectedId();
        cachedGateChannel = gateChannelSelector->getText();
        // Entry 0 is special; valid values are 1..(size-1).
        if ( (lutidx > 0) && (lutidx < ttlBankIdxLUT.size()) )
        {
            arduino->setParameter(ARDOUT_PARAM_GATEBANKIDX,
                ttlBankIdxLUT[lutidx]);
            arduino->setParameter(ARDOUT_PARAM_GATEBIT, ttlBitLUT[lutidx]);
        }
        else
        {
            arduino->setParameter(ARDOUT_PARAM_GATEBANKIDX, -1);
            arduino->setParameter(ARDOUT_PARAM_GATEBIT, -1);
        }
    }
}

void ArduinoOutputEditor::updateSettings()
{
    // Rebuild the comboboxes and the TTL channel lookup tables.

    ttlBankIdxLUT.clear();
    ttlBitLUT.clear();

    inputChannelSelector->clear(dontSendNotification);
    gateChannelSelector->clear(dontSendNotification);


    // The first entry is the "no signal selected" entry.
    // Store a combobox value of -1, since JUCE doesn't like 0.
    // Special-case that when looking up LUT entries.

    ttlBankIdxLUT.add(-1);
    ttlBitLUT.add(-1);
    int lutidx = 0;

    inputChannelSelector->addItem("(no trigger)", -1);
    gateChannelSelector->addItem("(no gate)", -1);


    int chanCount = arduino->getTotalEventChannels();

    for (int cidx = 0; cidx < chanCount; cidx++)
    {
        const EventChannel* theChannel = arduino->getEventChannel(cidx);
        EventChannel::EventChannelTypes thisType = theChannel->getChannelType();
        int thisBitCount = theChannel->getNumChannels();
        String thisName = theChannel->getName();

        if (EventChannel::TTL == thisType)
        {
            for (int bidx = 0; bidx < thisBitCount; bidx++)
            {
                // Name labels have the form "Bank:Bit (bank name)".
                std::string thisChanLabel = std::to_string(cidx) + ":"
                    + std::to_string(bidx) + " (" + thisName.toStdString()
                    + ")";

                lutidx++;
                ttlBankIdxLUT.add(cidx);
                ttlBitLUT.add(bidx);

                // The comboboxes store lookup table indices.
                inputChannelSelector->addItem(thisChanLabel, lutidx);
                gateChannelSelector->addItem(thisChanLabel, lutidx);
            }
        }
    }

    // Force reasonable defaults for the GUI.
    inputChannelSelector->setSelectedItemIndex(0, dontSendNotification);
    gateChannelSelector->setSelectedItemIndex(0, dontSendNotification);

    // Set reasonable defaults for the parent in case our cached values
    // are no longer valid. Don't touch the cache itself.
    arduino->setParameter(ARDOUT_PARAM_INBANKIDX, -1);
    arduino->setParameter(ARDOUT_PARAM_INBIT, -1);
    arduino->setParameter(ARDOUT_PARAM_GATEBANKIDX, -1);
    arduino->setParameter(ARDOUT_PARAM_GATEBIT, -1);

    // Restore any previously configured items that remain valid.
    RestoreSelectionsFromCache();
}

void ArduinoOutputEditor::saveCustomParameters(XmlElement *xml)
{
    xml->setAttribute("Type", "ArduinoOutput");
    xml->setAttribute("InputChannel", cachedInputChannel);
    xml->setAttribute("OutputChannel", cachedOutputChannel);
    xml->setAttribute("GateChannel", cachedGateChannel);
    xml->setAttribute("Device", cachedDevice);
}

void ArduinoOutputEditor::loadCustomParameters(XmlElement *xml)
{
    cachedInputChannel = xml->getStringAttribute("InputChannel");
    cachedOutputChannel = xml->getStringAttribute("OutputChannel");
    cachedGateChannel = xml->getStringAttribute("GateChannel");
    cachedDevice = xml->getStringAttribute("Device");

    // Adjust the comboboxes to reflect the config's selections, if possible.
    RestoreSelectionsFromCache();
}

void ArduinoOutputEditor::RestoreSelectionsFromCache()
{
    // This is intended to update selections after updateSettings() has
    // reset the comboboxes, or after we've loaded config from XML.

    // We do want to send notifications, to propagate configuration to
    // the processor.

    // If the cached item isn't found in the combobox, both are left as-is.

    int idx;

    for (idx = 0; idx < inputChannelSelector->getNumItems(); idx++)
        if ( inputChannelSelector->getItemText(idx) == cachedInputChannel )
            inputChannelSelector->setSelectedItemIndex( idx,
                sendNotificationAsync);

    for (idx = 0; idx < outputChannelSelector->getNumItems(); idx++)
        if ( outputChannelSelector->getItemText(idx) == cachedOutputChannel )
            outputChannelSelector->setSelectedItemIndex( idx,
                sendNotificationAsync);

    for (idx = 0; idx < gateChannelSelector->getNumItems(); idx++)
        if ( gateChannelSelector->getItemText(idx) == cachedGateChannel )
            gateChannelSelector->setSelectedItemIndex( idx,
                sendNotificationAsync);

    for (idx = 0; idx < deviceSelector->getNumItems(); idx++)
        if ( deviceSelector->getItemText(idx) == cachedDevice )
            deviceSelector->setSelectedItemIndex( idx,
                sendNotificationAsync);
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
