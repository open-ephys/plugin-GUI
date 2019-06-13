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

#include "EvntTrigAvgEditor.h"
#include "EvntTrigAvgCanvas.h"
#include "EvntTrigAvg.h"

#include <stdio.h>



EvntTrigAvgEditor::EvntTrigAvgEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, 300, useDefaultParameterEditors), evntTrigAvgCanvas(nullptr)

{
    tabText = "Evnt Trig Avg";
    desiredWidth = 200;

    processor = (EvntTrigAvg*) getProcessor();

    triggerChannel = new ComboBox("triggerChannel");
    triggerChannel->addListener(this);
    triggerChannel->setBounds(100,30,80,20);
    addAndMakeVisible(triggerChannel);
    triggerChannel->addItem("None",1);
    triggerChannel->setSelectedId(1, sendNotification);
    
    binSize = new Label("binSize","bin size");
    binSize->setFont(Font("Default", 12, Font::plain));
    binSize->setEditable(true);
    binSize->setBounds(100,60,80,20);
    binSize->addListener(this);
    binSize->setColour(Label::textColourId, Colours::white);
    binSize->setTooltip("Set the bin size of the histogram in milliseconds");
    binSize->setText(String(10),dontSendNotification);
    addAndMakeVisible(binSize);

    windowSize = new Label("windowSize","windowSize");
    windowSize->setFont(Font("Default", 12, Font::plain));
    windowSize->setEditable(true);
    windowSize->setBounds(100,90,80,20);
    windowSize->addListener(this);
    windowSize->setColour(Label::textColourId, Colours::white);
    windowSize->setTooltip("Set the window size of the histogram in milliseconds");
    windowSize->setText(String(1000),dontSendNotification);
    addAndMakeVisible(windowSize);
    
    channelLabel = new Label("channelLabel","channel label");
    channelLabel->setFont(Font("Default", 12, Font::plain));
    channelLabel->setEditable(false);
    channelLabel->setBounds(10,30,80,20);
    channelLabel->setColour(Label::textColourId, Colours::white);
    channelLabel->setText("Channel: ",dontSendNotification);
    addAndMakeVisible(channelLabel);
    
    binLabel = new Label("binLabel","bin label");
    binLabel->setFont(Font("Default", 12, Font::plain));
    binLabel->setEditable(false);
    binLabel->setBounds(10,60,80,20);
    binLabel->setColour(Label::textColourId, Colours::white);
    binLabel->setText("Bin Size (ms): ",dontSendNotification);
    addAndMakeVisible(binLabel);

    windowLabel = new Label("binLabel","bin label");
    windowLabel->setFont(Font("Default", 12, Font::plain));
    windowLabel->setEditable(false);
    windowLabel->setBounds(10,90,90,20);
    windowLabel->setColour(Label::textColourId, Colours::white);
    windowLabel->setText("Window Size (ms): ",dontSendNotification);
    addAndMakeVisible(windowLabel);

}

Visualizer* EvntTrigAvgEditor::createNewCanvas()
{

    EvntTrigAvg* processor = (EvntTrigAvg*) getProcessor();
    evntTrigAvgCanvas = new EvntTrigAvgCanvas(processor);
    return evntTrigAvgCanvas;
}


EvntTrigAvgEditor::~EvntTrigAvgEditor()
{

}

void EvntTrigAvgEditor::sliderEvent(Slider* slider)
{
    if (canvas!= nullptr)
        canvas->repaint();

}


void EvntTrigAvgEditor::buttonEvent(Button* button)
{
}

void EvntTrigAvgEditor::labelTextChanged(Label* label)
{
    uint64 wS = processor->getWindowSize();
    uint64 bS = processor->getBinSize();
    uint64 wms = wS/(processor->getSampleRate()/1000);
    uint64 bms = bS/(processor->getSampleRate()/1000);
    if (label == binSize){
        if(label->getText().getIntValue() < wms){
            if(label->getText().getIntValue() == 0){
                CoreServices::sendStatusMessage("Cannot have 0 bins");
                label->setText(String(bms),juce::NotificationType::dontSendNotification);
            }
            else if(wms/(label->getText().getIntValue()) <= 1000)
                processor->setParameter(2,label->getText().getIntValue());
            else{
                CoreServices::sendStatusMessage("Maximum of 1000 bins allowed");
                label->setText(String(bms),juce::NotificationType::dontSendNotification);
            }
        }
        else{
            CoreServices::sendStatusMessage("Bin size must be smaller than window size.");
            label->setText(String(bms),juce::NotificationType::dontSendNotification);
        }
    }
    else if (label == windowSize){
        if(label->getText().getIntValue() == 0){
            CoreServices::sendStatusMessage("Cannot have window size of 0");
        }
        else if(label->getText().getIntValue() > bms){
            if(label->getText().getIntValue()/bms<=1000)
                processor->setParameter(3,label->getText().getIntValue());
            else{
                CoreServices::sendStatusMessage("Maximum of 1000 bins allowed");
                label->setText(String(bms),juce::NotificationType::dontSendNotification);
            }
        }
        else{
            CoreServices::sendStatusMessage("Window size must be larer than bin size.");
            label->setText(String(wms),juce::NotificationType::dontSendNotification);
        }
    }
}


void EvntTrigAvgEditor::comboBoxChanged(ComboBox* comboBox)
{
    if (comboBox->getSelectedId() > 1)
    {
        int index = comboBox->getSelectedId() - 2;
        processor->setParameter(1, eventSourceArray[index].channel);
        processor->setParameter(0, eventSourceArray[index].eventIndex);
    }
    else
        getProcessor()->setParameter(0, -1);
}

void EvntTrigAvgEditor::channelChanged (int chan, bool newState){
    
}

void EvntTrigAvgEditor::updateSettings()
{
    EventSources s;
    String name;
    int oldId = triggerChannel->getSelectedId();
    triggerChannel->clear();
    triggerChannel->addItem("None", 1);
    int nextItem = 2;
    int nEvents = processor->getTotalEventChannels();
    for (int i = 0; i < nEvents; i++)
    {
        const EventChannel* event = processor->getEventChannel(i);
        if (event->getChannelType() == EventChannel::TTL)
        {
            s.eventIndex = i;
            int nChans = event->getNumChannels();
            for (int c = 0; c < nChans; c++)
            {
                s.channel = c;
                name = event->getSourceName() + " (TTL" + String(c+1) + ")";
                eventSourceArray.push_back(s);
                triggerChannel->addItem(name, nextItem++);
            }
        }
    }
    if (oldId > triggerChannel->getNumItems())
    {
        oldId = 1;
    }
    triggerChannel->setSelectedId(oldId, sendNotification);
}

//newValue*(getSampleRate()/1000);


void  EvntTrigAvgEditor::setTrigger(int val)
{
    triggerChannel->setSelectedId(val+2);
}
void EvntTrigAvgEditor::setBin(int val)
{
    binSize->setText(String(val),juce::NotificationType::dontSendNotification);
}
void  EvntTrigAvgEditor::setWindow(int val)
{
    windowSize->setText(String(val),juce::NotificationType::dontSendNotification);
}
