/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys
Copyright (C) 2014 Michael Borisov

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

#include "EcubeThread.h"
#include "EcubeEditor.h"


#include <stdio.h>

#ifdef ECUBE_COMPILE

EcubeEditor::EcubeEditor(GenericProcessor* parentNode, EcubeThread* npThread, bool useDefaultParameterEditors = true)
: GenericEditor(parentNode, useDefaultParameterEditors), pThread(npThread)

{
    desiredWidth = 180;

    if (!pThread->getNumHeadstageOutputs())
        return; // No audio monitoring for this input source

    volLabel = new Label("Volume text label", "Volume");
    volLabel->setBounds(35, 20, 180, 20);
    volLabel->setFont(Font("Small Text", 12, Font::plain));
    addAndMakeVisible(volLabel);

    volSlider = new Slider("Volume slider");
    volSlider->setBounds(20, 40, 80, 20);
    //ipValue->setFont(Font("Default", 15, Font::plain));
    //ipValue->setColour(Label::textColourId, Colours::white);
    //ipValue->setColour(Label::backgroundColourId, Colours::grey);
    //ipValue->setEditable(true);
    //ipValue->addListener(this);
    volSlider->setSliderStyle(Slider::LinearHorizontal);
    volSlider->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    volSlider->setRange(0.0, 1.0);
    volSlider->addListener(this);
    addAndMakeVisible(volSlider);

    chanLabel = new Label("Channel text label", "Channel");
    chanLabel->setBounds(35, 60, 180, 20);
    chanLabel->setFont(Font("Small Text", 12, Font::plain));
    addAndMakeVisible(chanLabel);

    chanComboBox = new ComboBox("Channel combo box");
    chanComboBox->setBounds(20, 80, 100, 20);
    chanComboBox->setEditableText(false);
    for (int i = 0; i < pThread->getNumChannels(); i++)
    {
        String s;
        s << i + 1;
        chanComboBox->addItem(s, i+1);
    }
    chanComboBox->setSelectedId(1, false);
    chanComboBox->setTooltip("Channel to monitor on the embedded speaker");
    chanComboBox->setScrollWheelEnabled(true);
    chanComboBox->addListener(this);
    addAndMakeVisible(chanComboBox);

}

EcubeEditor::~EcubeEditor()
{

}

/*void EcubeEditor::labelTextChanged(Label* label)
{/*
    FilterNode* fn = (FilterNode*)getProcessor();

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
}*/

void EcubeEditor::sliderValueChanged(Slider* slider)
{
    pThread->setSpeakerVolume(slider->getValue());
}

void EcubeEditor::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    pThread->setSpeakerChannel(comboBoxThatHasChanged->getSelectedId() - 1);
}


void EcubeEditor::saveEditorParameters(XmlElement* xml)
{

    // XmlElement* fileName = xml->createNewChildElement("FILENAME");
    // fileName->addTextElement(lastFilePath.getFullPathName());

}

void EcubeEditor::loadEditorParameters(XmlElement* xml)
{

    // forEachXmlChildElement(*xml, xmlNode)
    //    {
    //       if (xmlNode->hasTagName("FILENAME"))
    //       {

    //           lastFilePath = File(xmlNode->getText());
    //           thread->setFile(lastFilePath.getFullPathName());
    //           fileNameLabel->setText(lastFilePath.getFullPathName(),false);
    //       }
    //   }

}

#endif //ECUBE_COMPILE
