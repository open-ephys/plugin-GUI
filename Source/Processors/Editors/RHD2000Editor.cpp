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

#include "RHD2000Editor.h"
#include "../../UI/EditorViewport.h"

#include "ChannelSelector.h"

#include "../DataThreads/RHD2000Thread.h"

RHD2000Editor::RHD2000Editor(GenericProcessor* parentNode,
                             RHD2000Thread* board_,
                             bool useDefaultParameterEditors
                            )
    : GenericEditor(parentNode, useDefaultParameterEditors), board(board_)
{
    desiredWidth = 260;

    // add headstage-specific controls (currently just an enable/disable button)
    for (int i = 0; i < 4; i++)
    {
        HeadstageOptionsInterface* hsOptions = new HeadstageOptionsInterface(board, this, i);
        headstageOptionsInterfaces.add(hsOptions);
        addAndMakeVisible(hsOptions);
        hsOptions->setBounds(3, 28+i*20, 70, 18);
    }

    // add sample rate selection
    sampleRateInterface = new SampleRateInterface(board, this);
    addAndMakeVisible(sampleRateInterface);
    sampleRateInterface->setBounds(80, 25, 100, 50);

    // add Bandwidth selection
    bandwidthInterface = new BandwidthInterface(board, this);
    addAndMakeVisible(bandwidthInterface);
    bandwidthInterface->setBounds(80, 65, 80, 50);

    // add rescan button
    rescanButton = new UtilityButton("RESCAN", Font("Small Text", 13, Font::plain));
    rescanButton->setRadius(3.0f);
    rescanButton->setBounds(6, 108,65,18);
    rescanButton->addListener(this);
    rescanButton->setTooltip("Check for connected headstages");
    addAndMakeVisible(rescanButton);

    for (int i = 0; i < 2; i++)
    {
        ElectrodeButton* button = new ElectrodeButton(-1);
        electrodeButtons.add(button);

        button->setBounds(190+i*25, 40, 25, 15);
        button->setChannelNum(-1);
        button->setToggleState(false,false);
        button->setRadioGroupId(999);

        addAndMakeVisible(button);
        button->addListener(this);
        
        if (i == 0)
        {
            button->setTooltip("Audio monitor left channel");
        } else {
            button->setTooltip("Audio monitor right channel");
        }
    }
    

    audioLabel = new Label("audio label", "Audio out");
    audioLabel->setBounds(180,25,75,15);
    audioLabel->setFont(Font("Small Text", 10, Font::plain));
    audioLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(audioLabel);

    // add HW audio parameter selection
    audioInterface = new AudioInterface(board, this);
    addAndMakeVisible(audioInterface);
    audioInterface->setBounds(165, 65, 65, 50);
    
    
    adcButton = new UtilityButton("ADC 1-8", Font("Small Text", 13, Font::plain));
    adcButton->setRadius(3.0f);
    adcButton->setBounds(165,100,65,18);
    adcButton->addListener(this);
    adcButton->setClickingTogglesState(true);
    adcButton->setTooltip("Enable/disable ADC channels");
    addAndMakeVisible(adcButton);


}

RHD2000Editor::~RHD2000Editor()
{

}

void RHD2000Editor::scanPorts()
{
    rescanButton->triggerClick();
}

void RHD2000Editor::buttonEvent(Button* button)
{

    if (button == rescanButton && !acquisitionIsActive)
    {
        board->scanPorts();

        for (int i = 0; i < 4; i++)
        {
            headstageOptionsInterfaces[i]->checkEnabledState();
        }

    }
    else if (button == electrodeButtons[0])
    {
        channelSelector->setRadioStatus(true);
    }
    else if (button == electrodeButtons[1])
    {
        channelSelector->setRadioStatus(true);
    }
    else if (button == adcButton && !acquisitionIsActive)
    {
        board->enableAdcs(button->getToggleState());
        getEditorViewport()->makeEditorVisible(this, false, true);
    }

}

void RHD2000Editor::channelChanged(int chan)
{
    for (int i = 0; i < 2; i++)
    {
        if (electrodeButtons[i]->getToggleState())
        {
            electrodeButtons[i]->setChannelNum(chan);
            electrodeButtons[i]->repaint();

            board->assignAudioOut(i, chan);
        }
    }
}

void RHD2000Editor::startAcquisition()
{

    channelSelector->startAcquisition();

    rescanButton->setEnabledState(false);
    adcButton->setEnabledState(false);

    acquisitionIsActive = true;

}

void RHD2000Editor::stopAcquisition()
{

    channelSelector->stopAcquisition();

    rescanButton->setEnabledState(true);
    adcButton->setEnabledState(true);

    acquisitionIsActive = false;

}

void RHD2000Editor::saveEditorParameters(XmlElement* xml)
{
     xml->setAttribute("SampleRate", sampleRateInterface->getSelectedId());
     xml->setAttribute("LowCut", bandwidthInterface->getLowerBandwidth());
     xml->setAttribute("HighCut", bandwidthInterface->getUpperBandwidth());
     xml->setAttribute("ADCsOn", adcButton->getToggleState());
}

void RHD2000Editor::loadEditorParameters(XmlElement* xml)
{
    
    sampleRateInterface->setSelectedId(xml->getIntAttribute("SampleRate"));
    bandwidthInterface->setLowerBandwidth(xml->getDoubleAttribute("LowCut"));
    bandwidthInterface->setUpperBandwidth(xml->getDoubleAttribute("HighCut"));
    adcButton->setToggleState(xml->getBoolAttribute("ADCsOn"), true);

}


// Bandwidth Options --------------------------------------------------------------------

BandwidthInterface::BandwidthInterface(RHD2000Thread* board_,
                                       RHD2000Editor* editor_) :
    board(board_), editor(editor_)
{

    name = "Bandwidth";

    lastHighCutString = "7500";
    lastLowCutString = "1";

    actualUpperBandwidth = 7500.0f;
    actualLowerBandwidth = 1.0f;

    upperBandwidthSelection = new Label("UpperBandwidth",lastHighCutString); // this is currently set in RHD2000Thread, the cleaner would be to set it here again
    upperBandwidthSelection->setEditable(true,false,false);
    upperBandwidthSelection->addListener(this);
    upperBandwidthSelection->setBounds(30,30,60,20);
    upperBandwidthSelection->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(upperBandwidthSelection);


    lowerBandwidthSelection = new Label("LowerBandwidth",lastLowCutString);
    lowerBandwidthSelection->setEditable(true,false,false);
    lowerBandwidthSelection->addListener(this);
    lowerBandwidthSelection->setBounds(25,10,60,20);
    lowerBandwidthSelection->setColour(Label::textColourId, Colours::darkgrey);

    addAndMakeVisible(lowerBandwidthSelection);



}

BandwidthInterface::~BandwidthInterface()
{

}


void BandwidthInterface::labelTextChanged(Label* label)
{

    if (!(editor->acquisitionIsActive) && board->foundInputSource())
    {
        if (label == upperBandwidthSelection)
        {

            Value val = label->getTextValue();
            double requestedValue = double(val.getValue());

            if (requestedValue < 100.0 || requestedValue > 20000.0 || requestedValue < lastLowCutString.getFloatValue())
            {
                editor->sendActionMessage("Value out of range.");

                label->setText(lastHighCutString, dontSendNotification);

                return;
            }

            actualUpperBandwidth = board->setUpperBandwidth(requestedValue);

            std::cout << "Setting Upper Bandwidth to " << requestedValue << std::endl;
            std::cout << "Actual Upper Bandwidth:  " <<  actualUpperBandwidth  << std::endl;
            label->setText(String((roundFloatToInt)(actualUpperBandwidth)), dontSendNotification);

        }
        else
        {

            Value val = label->getTextValue();
            double requestedValue = double(val.getValue());

            if (requestedValue < 0.1 || requestedValue > 500.0 || requestedValue > lastHighCutString.getFloatValue())
            {
                editor->sendActionMessage("Value out of range.");

                label->setText(lastLowCutString, dontSendNotification);

                return;
            }

            actualLowerBandwidth = board->setLowerBandwidth(requestedValue);

            std::cout << "Setting Upper Bandwidth to " << requestedValue << std::endl;
            std::cout << "Actual Upper Bandwidth:  " <<  actualLowerBandwidth  << std::endl;
            label->setText(String(roundFloatToInt(actualLowerBandwidth)), dontSendNotification);
        }
    }
    else if (editor->acquisitionIsActive)
    {
        editor->sendActionMessage("Can't change bandwidth while acquisition is active!");
        if (label == upperBandwidthSelection)
            label->setText(lastHighCutString, dontSendNotification);
        else
            label->setText(lastLowCutString, dontSendNotification);
        return;
    }

}

void BandwidthInterface::setLowerBandwidth(double value)
{
    actualLowerBandwidth = board->setLowerBandwidth(value);
    lowerBandwidthSelection->setText(String(roundFloatToInt(actualLowerBandwidth)), dontSendNotification);
}

void BandwidthInterface::setUpperBandwidth(double value)
{
    actualUpperBandwidth = board->setUpperBandwidth(value);
    upperBandwidthSelection->setText(String(roundFloatToInt(actualUpperBandwidth)), dontSendNotification);
}

double BandwidthInterface::getLowerBandwidth()
{
    return actualLowerBandwidth;
}

double BandwidthInterface::getUpperBandwidth()
{
    return actualUpperBandwidth;
}


void BandwidthInterface::paint(Graphics& g)
{

    g.setColour(Colours::darkgrey);

    g.setFont(Font("Small Text",10,Font::plain));

    g.drawText(name, 0, 0, 200, 15, Justification::left, false);

    g.drawText("Low: ", 0, 10, 200, 20, Justification::left, false);

    g.drawText("High: ", 0, 30, 200, 20, Justification::left, false);

}

// Sample rate Options --------------------------------------------------------------------

SampleRateInterface::SampleRateInterface(RHD2000Thread* board_,
                                         RHD2000Editor* editor_) :
    board(board_), editor(editor_)
{

    name = "Sample Rate";

    sampleRateOptions.add("1.00 kS/s");
    sampleRateOptions.add("1.25 kS/s");
    sampleRateOptions.add("1.50 kS/s");
    sampleRateOptions.add("2.00 kS/s");
    sampleRateOptions.add("2.50 kS/s");
    sampleRateOptions.add("3.00 kS/s");
    sampleRateOptions.add("3.33 kS/s");
    sampleRateOptions.add("4.00 kS/s");
    sampleRateOptions.add("5.00 kS/s");
    sampleRateOptions.add("6.25 kS/s");
    sampleRateOptions.add("8.00 kS/s");
    sampleRateOptions.add("10.0 kS/s");
    sampleRateOptions.add("12.5 kS/s");
    sampleRateOptions.add("15.0 kS/s");
    sampleRateOptions.add("20.0 kS/s");
    sampleRateOptions.add("25.0 kS/s");
    sampleRateOptions.add("30.0 kS/s");


    rateSelection = new ComboBox("Sample Rate");
    rateSelection->addItemList(sampleRateOptions, 1);
    rateSelection->setSelectedId(17,false);
    rateSelection->addListener(this);

    rateSelection->setBounds(0,15,300,20);
    addAndMakeVisible(rateSelection);


}

SampleRateInterface::~SampleRateInterface()
{

}

void SampleRateInterface::comboBoxChanged(ComboBox* cb)
{
    if (!(editor->acquisitionIsActive) && board->foundInputSource())
    {
        if (cb == rateSelection)
        {
            board->setSampleRate(cb->getSelectedId()-1);

            std::cout << "Setting sample rate to index " << cb->getSelectedId()-1 << std::endl;

            editor->getEditorViewport()->makeEditorVisible(editor, false, true);
        }
    }
}

int SampleRateInterface::getSelectedId()
{
    return rateSelection->getSelectedId();
}

void SampleRateInterface::setSelectedId(int id)
{
    rateSelection->setSelectedId(id);
}


void SampleRateInterface::paint(Graphics& g)
{

    g.setColour(Colours::darkgrey);

    g.setFont(Font("Small Text",10,Font::plain));

    g.drawText(name, 0, 0, 200, 15, Justification::left, false);

}


// Headstage Options --------------------------------------------------------------------

HeadstageOptionsInterface::HeadstageOptionsInterface(RHD2000Thread* board_,
                                                     RHD2000Editor* editor_,
                                                     int hsNum) :
    isEnabled(false), board(board_), editor(editor_)
{

    switch (hsNum)
    {
        case 0 :
            name = "A";
            break;
        case 1:
            name = "B";
            break;
        case 2:
            name = "C";
            break;
        case 3:
            name = "D";
            break;
        default:
            name = "X";
    }

    hsNumber1 = hsNum*2; // data stream 1
    hsNumber2 = hsNumber1+1; // data stream 2

    channelsOnHs1 = 0;
    channelsOnHs2 = 0;



    hsButton1 = new UtilityButton(" ", Font("Small Text", 13, Font::plain));
    hsButton1->setRadius(3.0f);
    hsButton1->setBounds(23,1,20,17);
    hsButton1->setEnabledState(false);
    hsButton1->setCorners(true, false, true, false);
    hsButton1->addListener(this);
    addAndMakeVisible(hsButton1);

    hsButton2 = new UtilityButton(" ", Font("Small Text", 13, Font::plain));
    hsButton2->setRadius(3.0f);
    hsButton2->setBounds(43,1,20,17);
    hsButton2->setEnabledState(false);
    hsButton2->setCorners(false, true, false, true);
    hsButton2->addListener(this);
    addAndMakeVisible(hsButton2);

    checkEnabledState();
}

HeadstageOptionsInterface::~HeadstageOptionsInterface()
{

}

void HeadstageOptionsInterface::checkEnabledState()
{
    isEnabled = (board->isHeadstageEnabled(hsNumber1) ||
                 board->isHeadstageEnabled(hsNumber2));

    if (board->isHeadstageEnabled(hsNumber1))
    {
        channelsOnHs1 = 32;
        hsButton1->setLabel(String(channelsOnHs1));
        hsButton1->setEnabledState(true);
    }
    else
    {
        channelsOnHs1 = 0;
        hsButton1->setLabel(" ");
        hsButton1->setEnabledState(false);
    }

    if (board->isHeadstageEnabled(hsNumber2))
    {
        channelsOnHs2 = 32;
        hsButton2->setLabel(String(channelsOnHs2));
        hsButton2->setEnabledState(true);
    }
    else
    {
        channelsOnHs2 = 0;
        hsButton2->setLabel(" ");
        hsButton2->setEnabledState(false);
    }

    repaint();

}

void HeadstageOptionsInterface::buttonClicked(Button* button)
{

    if (!(editor->acquisitionIsActive) && board->foundInputSource())
    {

        //std::cout << "Acquisition is not active" << std::endl;
        if (button == hsButton1)
        {
            if (channelsOnHs1 == 32)
                channelsOnHs1 = 16;
            else
                channelsOnHs1 = 32;

            //std::cout << "HS1 has " << channelsOnHs1 << " channels." << std::endl;

            hsButton1->setLabel(String(channelsOnHs1));
            board->setNumChannels(hsNumber1, channelsOnHs1);

        }
        else if (button == hsButton2)
        {
            if (channelsOnHs2 == 32)
                channelsOnHs2 = 16;
            else
                channelsOnHs2 = 32;

            hsButton2->setLabel(String(channelsOnHs2));
            board->setNumChannels(hsNumber2, channelsOnHs2);
        }


        editor->getEditorViewport()->makeEditorVisible(editor, false, true);
    }

}


void HeadstageOptionsInterface::paint(Graphics& g)
{
    g.setColour(Colours::lightgrey);

    g.fillRoundedRectangle(5,0,getWidth()-10,getHeight(),4.0f);

    if (isEnabled)
        g.setColour(Colours::black);
    else
        g.setColour(Colours::grey);

    g.setFont(Font("Small Text",15,Font::plain));

    g.drawText(name, 8, 2, 200, 15, Justification::left, false);

}


// (Direct OpalKelly) Audio Options --------------------------------------------------------------------

AudioInterface::AudioInterface(RHD2000Thread* board_,
                                       RHD2000Editor* editor_) :
board(board_), editor(editor_)
{
    
    name = "Noise Slicer";
    
    lastNoiseSlicerString = "0";
    
    actualNoiseSlicerLevel = 0.0f;
    
    noiseSlicerLevelSelection = new Label("Noise Slicer",lastNoiseSlicerString); // this is currently set in RHD2000Thread, the cleaner would be to set it here again
    noiseSlicerLevelSelection->setEditable(true,false,false);
    noiseSlicerLevelSelection->addListener(this);
    noiseSlicerLevelSelection->setBounds(30,10,30,20);
    noiseSlicerLevelSelection->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(noiseSlicerLevelSelection);
    
    
}

AudioInterface::~AudioInterface()
{
    
}


void AudioInterface::labelTextChanged(Label* label)
{
    if (board->foundInputSource())
    {
        if (label == noiseSlicerLevelSelection)
        {
            
            Value val = label->getTextValue();
            int requestedValue = int(val.getValue()); // Note that it might be nice to translate to actual uV levels (16*value)
            
            if (requestedValue < 0 || requestedValue > 127)
            {
                editor->sendActionMessage("Value out of range.");
                
                label->setText(lastNoiseSlicerString, dontSendNotification);
                
                return;
            }
            
            actualNoiseSlicerLevel = board->setNoiseSlicerLevel(requestedValue);
            
            std::cout << "Setting Noise Slicer Level to " << requestedValue << std::endl;
            label->setText(String((roundFloatToInt)(actualNoiseSlicerLevel)), dontSendNotification);

        }
    }
    else {
        Value val = label->getTextValue();
        int requestedValue = int(val.getValue()); // Note that it might be nice to translate to actual uV levels (16*value)
        if (requestedValue < 0 || requestedValue > 127)
        {
            editor->sendActionMessage("Value out of range.");
            label->setText(lastNoiseSlicerString, dontSendNotification);
            return;
        }
    }
}

void AudioInterface::setNoiseSlicerLevel(int value)
{
    actualNoiseSlicerLevel = board->setNoiseSlicerLevel(value);
    noiseSlicerLevelSelection->setText(String(roundFloatToInt(actualNoiseSlicerLevel)), dontSendNotification);
}

int AudioInterface::getNoiseSlicerLevel()
{
    return actualNoiseSlicerLevel;
}


void AudioInterface::paint(Graphics& g)
{
    
    g.setColour(Colours::darkgrey);
    
    g.setFont(Font("Small Text",9,Font::plain));
    
    g.drawText(name, 0, 0, 200, 15, Justification::left, false);
    
    g.drawText("Level: ", 0, 10, 200, 20, Justification::left, false);
    
}


