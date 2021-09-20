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

#include "FilterEditor.h"
#include "FilterNode.h"
#include <stdio.h>


FilterEditor::FilterEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
    desiredWidth = 150;

    lastLowCutString = " ";
    lastHighCutString = " ";

    highCutLabel = new Label("high cut label", "High cut:");
    highCutLabel->setBounds(10,65,80,20);
    highCutLabel->setFont(Font("Small Text", 12, Font::plain));
    highCutLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(highCutLabel);

    lowCutLabel = new Label("low cut label", "Low cut:");
    lowCutLabel->setBounds(10,25,80,20);
    lowCutLabel->setFont(Font("Small Text", 12, Font::plain));
    lowCutLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(lowCutLabel);

    lowCutValue = new Label("low cut value", lastLowCutString);
    lowCutValue->setBounds(15,42,60,18);
    lowCutValue->setFont(Font("Default", 15, Font::plain));
    lowCutValue->setColour(Label::textColourId, Colours::white);
    lowCutValue->setColour(Label::backgroundColourId, Colours::grey);
    lowCutValue->setEditable(true);
    lowCutValue->addListener(this);
    lowCutValue->setTooltip("Set the low cut for the selected channels");
    addAndMakeVisible(lowCutValue);

    highCutValue = new Label("high cut label", lastHighCutString);
    highCutValue->setBounds(15,82,60,18);
    highCutValue->setFont(Font("Default", 15, Font::plain));
    highCutValue->setColour(Label::textColourId, Colours::white);
    highCutValue->setColour(Label::backgroundColourId, Colours::grey);
    highCutValue->setEditable(true);
    highCutValue->addListener(this);
    highCutValue->setTooltip("Set the high cut for the selected channels");
    addAndMakeVisible(highCutValue);

    channelSelectionButton = new UtilityButton("Channels", Font("Default", 10, Font::plain));
    channelSelectionButton->addListener(this);
    channelSelectionButton->setBounds(15,107,60,18);
    channelSelectionButton->setClickingTogglesState(false);
    channelSelectionButton->setTooltip("Select channels to filter within this stream");
    addAndMakeVisible(channelSelectionButton);

}

FilterEditor::~FilterEditor()
{

}

void FilterEditor::setDefaults(double lowCut, double highCut)
{
    lastHighCutString = String(roundFloatToInt(highCut));
    lastLowCutString = String(roundFloatToInt(lowCut));

    resetToSavedText();
}

void FilterEditor::resetToSavedText()
{
    highCutValue->setText(lastHighCutString, dontSendNotification);
    lowCutValue->setText(lastLowCutString, dontSendNotification);
}


void FilterEditor::labelTextChanged(Label* label)
{
    FilterNode* proc = (FilterNode*) getProcessor();

    Value val = label->getTextValue();
    double requestedValue = double(val.getValue());

    if (requestedValue < 0.01 || requestedValue > 10000)
    {
        CoreServices::sendStatusMessage("Value out of range.");

        if (label == highCutValue)
        {
            label->setText(lastHighCutString, dontSendNotification);
            lastHighCutString = label->getText();
        }
        else
        {
            label->setText(lastLowCutString, dontSendNotification);
            lastLowCutString = label->getText();
        }

        return;
    }

    if (label == highCutValue)
    {
        double minVal = proc->getLowCutValue(getCurrentStream());

        if (requestedValue > minVal)
        {
            proc->setHighCutValue(getCurrentStream(), requestedValue);
        }

        lastHighCutString = label->getText();

    }
    else
    {
        double maxVal = proc->getHighCutValue(getCurrentStream());

        if (requestedValue < maxVal)
        {
            proc->setLowCutValue(getCurrentStream(), requestedValue);
        }

        lastLowCutString = label->getText();
    }

}

void FilterEditor::channelStateChanged (Array<int> selectedChannels)
{
    auto processor = static_cast<FilterNode*> (getProcessor());

    processor->setChannelMask(getCurrentStream(), selectedChannels);

}

void FilterEditor::selectedStreamHasChanged()
{

    auto processor = static_cast<FilterNode*> (getProcessor());

    lastHighCutString = String(roundFloatToInt(processor->getHighCutValue(getCurrentStream())));
    lastLowCutString = String(roundFloatToInt(processor->getLowCutValue(getCurrentStream())));

    resetToSavedText();
}


void FilterEditor::buttonEvent(Button* button)
{

    if (button == channelSelectionButton)
    {
        Array<bool> mask = static_cast<FilterNode*> (getProcessor())->getChannelMask(getCurrentStream());

        std::vector<bool> channelStates;

        for (int i = 0; i < mask.size(); i++)
        {
            channelStates.push_back(mask[i]);
        }

        auto* channelSelector = new PopupChannelSelector(this, channelStates);

        channelSelector->setChannelButtonColour(Colour(0, 174, 239));

        CallOutBox& myBox
            = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(channelSelector),
                channelSelectionButton->getScreenBounds(),
                nullptr);

        myBox.setDismissalMouseClicksAreAlwaysConsumed(true);

    }

}


/*void FilterEditor::saveCustomParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "FilterEditor");

    lastHighCutString = highCutValue->getText();
    lastLowCutString = lowCutValue->getText();

    XmlElement* textLabelValues = xml->createNewChildElement("VALUES");
    textLabelValues->setAttribute("HighCut",lastHighCutString);
    textLabelValues->setAttribute("LowCut",lastLowCutString);
    textLabelValues->setAttribute("ApplyToADC",	applyFilterOnADC->getToggleState());
}

void FilterEditor::loadCustomParameters(XmlElement* xml)
{

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("VALUES"))
        {
            lastHighCutString = xmlNode->getStringAttribute("HighCut", lastHighCutString);
            lastLowCutString = xmlNode->getStringAttribute("LowCut", lastLowCutString);
            resetToSavedText();

            applyFilterOnADC->setToggleState(xmlNode->getBoolAttribute("ApplyToADC",false), sendNotification);
        }
    }


}*/
