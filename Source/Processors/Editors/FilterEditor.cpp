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

#include "FilterEditor.h"
#include "../FilterNode.h"
#include <stdio.h>


FilterEditor::FilterEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
    desiredWidth = 150;

    lastLowCutString = " ";
    lastHighCutString = " ";

    highCutLabel = new Label("high cut label", "High cut:");
    highCutLabel->setBounds(35,80,80,20);
    highCutLabel->setFont(Font("Small Text", 12, Font::plain));
    highCutLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(highCutLabel);

    lowCutLabel = new Label("low cut label", "Low cut:");
    lowCutLabel->setBounds(35,30,80,20);
    lowCutLabel->setFont(Font("Small Text", 12, Font::plain));
    lowCutLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(lowCutLabel);

    lowCutValue = new Label("low cut value", lastLowCutString);
    lowCutValue->setBounds(40,50,60,20);
    lowCutValue->setFont(Font("Default", 15, Font::plain));
    lowCutValue->setColour(Label::textColourId, Colours::white);
    lowCutValue->setColour(Label::backgroundColourId, Colours::grey);
    lowCutValue->setEditable(true);
    lowCutValue->addListener(this);
    addAndMakeVisible(lowCutValue);

    highCutValue = new Label("high cut label", lastHighCutString);
    highCutValue->setBounds(40,100,60,20);
    highCutValue->setFont(Font("Default", 15, Font::plain));
    highCutValue->setColour(Label::textColourId, Colours::white);
    highCutValue->setColour(Label::backgroundColourId, Colours::grey);
    highCutValue->setEditable(true);
    highCutValue->addListener(this);
    addAndMakeVisible(highCutValue);

}

FilterEditor::~FilterEditor()
{
    
}   

void FilterEditor::setDefaults(double lowCut, double highCut)
{
    lastHighCutString = String(roundFloatToInt(highCut));
    lastLowCutString = String(roundFloatToInt(lowCut));

    highCutValue->setText(lastHighCutString, dontSendNotification);
    lowCutValue->setText(lastLowCutString, dontSendNotification);
}


void FilterEditor::labelTextChanged(Label* label)
{
    FilterNode* fn = (FilterNode*) getProcessor();

    Value val = label->getTextValue();
    double requestedValue = double(val.getValue());

    if (requestedValue < 0.01 || requestedValue > 10000) 
    {
        sendActionMessage("Value out of range.");

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

    Array<int> chans = getActiveChannels();

    // This needs to change, since there's not enough feedback about whether
    // or not individual channel settings were altered:

    for (int n = 0; n < chans.size(); n++)
    {

        if (label == highCutValue)
        {
            double minVal = fn->getLowCutValueForChannel(n);

            if (requestedValue > minVal)
            {
                fn->setCurrentChannel(n);
                fn->setParameter(1, requestedValue);
            }

            lastHighCutString = label->getText();

        }
        else
        {
            double maxVal = fn->getHighCutValueForChannel(n);

            if (requestedValue < maxVal)
            {
                fn->setCurrentChannel(n);
                fn->setParameter(0, requestedValue);
            }

            lastLowCutString = label->getText();
        }

    }

}

void FilterEditor::buttonEvent(Button* button)
{
    //std::cout << button->getRadioGroupId() << " " << button->getName() << std::endl;

    //if (!checkDrawerButton(button) && !checkChannelSelectors(button)) {

    // String value = button->getName();
    // float val;

    // val = value.getFloatValue();

    // Array<int> chans = getActiveChannels();

    // GenericProcessor* p = (GenericProcessor*) getAudioProcessor();

    // for (int n = 0; n < chans.size(); n++)
    // {

    //     p->setCurrentChannel(chans[n]);

    //     if (button->getRadioGroupId() == 1)
    //         getAudioProcessor()->setParameter(0,val);
    //     else
    //         getAudioProcessor()->setParameter(1,val*1000.0f);

    // }
    //std::cout << button->getRadioGroupId() << " " << val << std::endl;
    //	}

}


void FilterEditor::saveEditorParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "FilterEditor");

    XmlElement* textLabelValues = xml->createNewChildElement("VALUES");
    textLabelValues->setAttribute("HighCut",lastHighCutString);
    textLabelValues->setAttribute("LowCut",lastLowCutString);
}

void FilterEditor::loadEditorParameters(XmlElement* xml)
{

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("VALUES"))
        {
            highCutValue->setText(xmlNode->getStringAttribute("HighCut"),dontSendNotification);
            lowCutValue->setText(xmlNode->getStringAttribute("LowCut"),dontSendNotification);
        }
    }
}
