/*
------------------------------------------------------------------

This file is part of a plugin for the Open Ephys GUI
Copyright (C) 2018 Translational NeuroEngineering Laboratory

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

#include "SampleMathEditor.h"
#include <string> // stof
#include <cfloat> // limits

SampleMathEditor::SampleMathEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors)
    : GenericEditor(parentNode, useDefaultParameterEditors)
{
    desiredWidth = 100;
    SampleMath* processor = static_cast<SampleMath*>(parentNode);

    int xCenter = desiredWidth / 2 - 5; // account for drawer button
    int yPos = 35;

    operationBox = new ComboBox("operation");
    operationBox->addItem("+", ADD);
    operationBox->addItem(L"\u2212", SUBTRACT);
    operationBox->addItem(L"\u00d7", MULTIPLY);
    operationBox->addItem(L"\u00f7", DIVIDE);
    operationBox->setSelectedId(processor->operation, dontSendNotification);
    int width = 40;
    operationBox->setBounds(xCenter - width / 2, yPos, width, 20);
    operationBox->addListener(this);
    addAndMakeVisible(operationBox);

    useChannelBox = new ComboBox("useChannel");
    useChannelBox->addItem("CONST", 1);
    useChannelBox->addItem("CHAN", 2);
    useChannelBox->setSelectedId(processor->useChannel ? 2 : 1, dontSendNotification);
    width = 70;
    useChannelBox->setBounds(xCenter - width / 2, yPos += 30, width, 20);
    useChannelBox->addListener(this);
    addAndMakeVisible(useChannelBox);

    channelSelectionBox = new ComboBox("channelSelection");
    width = 50;
    channelSelectionBox->setBounds(xCenter - width / 2, yPos += 30, width, 20);
    channelSelectionBox->addListener(this);
    channelSelectionBox->setTooltip(CHANNEL_SELECT_TOOLTIP);
    channelSelectionBox->setVisible(processor->useChannel);
    addChildComponent(channelSelectionBox);

    constantEditable = new Label("constantE");
    constantEditable->setEditable(true);
    width = 60;
    constantEditable->setBounds(xCenter - width / 2, yPos, width, 20);
    constantEditable->setText(String(processor->constant), dontSendNotification);
    constantEditable->setColour(Label::backgroundColourId, Colours::grey);
    constantEditable->setColour(Label::textColourId, Colours::white);
    constantEditable->addListener(this);
    constantEditable->setVisible(!channelSelectionBox->isVisible());
    addChildComponent(constantEditable);
}

SampleMathEditor::~SampleMathEditor() {}

void SampleMathEditor::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    SampleMath* processor = static_cast<SampleMath*>(getProcessor());
    if (comboBoxThatHasChanged == operationBox)
    {
        processor->setParameter(OPERATION, static_cast<float>(operationBox->getSelectedId()));
    }
    else if (comboBoxThatHasChanged == useChannelBox)
    {
        bool useChannel = static_cast<bool>(useChannelBox->getSelectedId() - 1);
        if (useChannel)
        {
            constantEditable->setVisible(false);
            channelSelectionBox->setVisible(true);
        }
        else
        {
            channelSelectionBox->setVisible(false);
            constantEditable->setVisible(true);
        }
        processor->setParameter(USE_CHANNEL, static_cast<float>(useChannel));
    }
    else if (comboBoxThatHasChanged == channelSelectionBox)
    {
        processor->setParameter(CHANNEL, static_cast<float>(channelSelectionBox->getSelectedId() - 1));
    }
}

void SampleMathEditor::labelTextChanged(Label* labelThatHasChanged)
{
    SampleMath* processor = static_cast<SampleMath*>(getProcessor());
    if (labelThatHasChanged == constantEditable)
    {
        float floatInput;
        bool valid = updateFloatLabel(labelThatHasChanged, -FLT_MAX, FLT_MAX,
            processor->constant, &floatInput);

        if (valid)
            processor->setParameter(CONSTANT, floatInput);
    }
}

void SampleMathEditor::updateSettings()
{
    SampleMath* processor = static_cast<SampleMath*>(getProcessor());

    // repopulate channel selection box
    channelSelectionBox->clear();
    int numChannels = processor->getNumInputs();
    for (int i = 1; i <= numChannels; ++i)
        channelSelectionBox->addItem(String(i), i);
    
    // update selected item
    if (processor->selectedChannel >= 0)
        channelSelectionBox->setSelectedId(processor->selectedChannel + 1, dontSendNotification);
}

void SampleMathEditor::channelChanged(int chan, bool newState)
{
    SampleMath* processor = static_cast<SampleMath*>(getProcessor());
    if (newState == true && processor->useChannel)
    {
        bool subProcessorMismatch = processor->chanToFullID(chan) != processor->validSubProcFullID;
        if (subProcessorMismatch)
        {
            // turn channel back off
            CoreServices::sendStatusMessage("Cannot select this channel due to subprocessor mismatch");
            bool p, r, a;
            getChannelSelectionState(chan, &p, &r, &a);
            setChannelSelectionState(chan - 1, false, r, a);
        }
    }
}

void SampleMathEditor::collapsedStateChanged()
{
    switch (useChannelBox->getSelectedId())
    {
    case 2:
        constantEditable->setVisible(false);
        break;

    case 1:
    default:
        channelSelectionBox->setVisible(false);
        break;
    }
}

void SampleMathEditor::saveCustomParameters(XmlElement* xml)
{
    xml->setAttribute("Type", "SampleMath");

    SampleMath* processor = static_cast<SampleMath*>(getProcessor());
    XmlElement* paramValues = xml->createNewChildElement("VALUES");
    paramValues->setAttribute("operation", processor->operation);
    paramValues->setAttribute("useChannel", processor->useChannel);
    paramValues->setAttribute("constant", processor->constant);
    paramValues->setAttribute("selectedChannel", processor->selectedChannel + 1); // one-based!
}

void SampleMathEditor::loadCustomParameters(XmlElement* xml)
{
    forEachXmlChildElementWithTagName(*xml, xmlNode, "VALUES")
    {
        // select channel while useChannel is false to avoid unnecessarily inactivating channels
        int selectedChannel = xmlNode->getIntAttribute("selectedChannel", channelSelectionBox->getSelectedId());
        if (selectedChannel > 0 && selectedChannel <= channelSelectionBox->getNumItems())
        {
            channelSelectionBox->setSelectedId(selectedChannel, sendNotificationSync);
        }

        operationBox->setSelectedId(xmlNode->getIntAttribute("operation", operationBox->getSelectedId()), sendNotificationAsync);
        constantEditable->setText(xmlNode->getStringAttribute("constant", constantEditable->getText()), sendNotificationAsync);
        int useChannelIdx = 1 + xmlNode->getIntAttribute("useChannel", useChannelBox->getSelectedId() - 1);
        useChannelBox->setSelectedId(useChannelIdx, sendNotificationAsync);
    }
}

// static utilities

bool SampleMathEditor::updateFloatLabel(Label* labelThatHasChanged,
    float minValue, float maxValue, float defaultValue, float* result)
{
    String& input = labelThatHasChanged->getText();
    bool valid = parseInput(input, minValue, maxValue, result);
    if (!valid)
        labelThatHasChanged->setText(String(defaultValue), dontSendNotification);
    else
        labelThatHasChanged->setText(String(*result), dontSendNotification);

    return valid;
}

bool SampleMathEditor::parseInput(String& in, float min, float max, float* out)
{
    float parsedFloat;
    try
    {
        parsedFloat = std::stof(in.toRawUTF8());
    }
    catch (...)
    {
        return false;
    }

    if (parsedFloat < min)
        *out = min;
    else if (parsedFloat > max)
        *out = max;
    else
        *out = parsedFloat;

    return true;
}