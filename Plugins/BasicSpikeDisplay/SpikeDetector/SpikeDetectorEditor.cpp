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

#include "SpikeDetectorEditor.h"
#include "SpikeDetector.h"
#include "PopupConfigurationWindow.h"

#include <stdio.h>


SpikeDetectorEditor::SpikeDetectorEditor(GenericProcessor* parentNode)
    : GenericEditor(parentNode),
    currentConfigWindow(nullptr)

{

    desiredWidth = 250;
    
    lastLabelValue = "1";
    spikeChannelCountLabel = std::make_unique<Label>("Label", lastLabelValue);
    spikeChannelCountLabel->setEditable(true);
    spikeChannelCountLabel->addListener(this);
    spikeChannelCountLabel->setBounds(10, 35, 40, 20);
    addAndMakeVisible(spikeChannelCountLabel.get());
    
    spikeChannelTypeSelector = std::make_unique<ComboBox>("Spike Channel Type");
    spikeChannelTypeSelector->setBounds(40, 35, 125, 20);
    spikeChannelTypeSelector->addItem("Single electrode", SpikeChannel::SINGLE);
    spikeChannelTypeSelector->addItem("Stereotrode", SpikeChannel::STEREOTRODE);
    spikeChannelTypeSelector->addItem("Tetrode", SpikeChannel::TETRODE);
    spikeChannelTypeSelector->setSelectedId(SpikeChannel::SINGLE);
    addAndMakeVisible(spikeChannelTypeSelector.get());
    
    plusButton = std::make_unique<UtilityButton>("+", Font("Default", 16, Font::plain));
    plusButton->addListener(this);
    plusButton->setBounds(170, 35, 20, 20);
    addAndMakeVisible(plusButton.get());
    
    configureButton = std::make_unique< UtilityButton>("configure", titleFont);
    configureButton->addListener(this);
    configureButton->setRadius(3.0f);
    configureButton->setBounds(25, 82, 74, 20);
    configureButton->setEnabled(false);
    addAndMakeVisible(configureButton.get());
    
}

void SpikeDetectorEditor::labelTextChanged(Label* label)
{
    int value = label->getText().getIntValue();
    
    if (value < 1 || value > 64)
    {
        label->setText(lastLabelValue, dontSendNotification);
        return;
    }
        
    label->setText(String(value), dontSendNotification);
    lastLabelValue = label->getText();
    
    if (value == 1)
    {
        
        int currentId = spikeChannelTypeSelector->getSelectedId();
        
        spikeChannelTypeSelector->clear();
        
        spikeChannelTypeSelector->addItem("Single electrode", SpikeChannel::SINGLE);
        spikeChannelTypeSelector->addItem("Stereotrode", SpikeChannel::STEREOTRODE);
        spikeChannelTypeSelector->addItem("Tetrode", SpikeChannel::TETRODE);
        spikeChannelTypeSelector->setSelectedId(currentId, dontSendNotification);
        
    } else {
    
       int currentId = spikeChannelTypeSelector->getSelectedId();
               
       spikeChannelTypeSelector->clear();
       
       spikeChannelTypeSelector->addItem("Single electrodes", SpikeChannel::SINGLE);
       spikeChannelTypeSelector->addItem("Stereotrodes", SpikeChannel::STEREOTRODE);
       spikeChannelTypeSelector->addItem("Tetrodes", SpikeChannel::TETRODE);
       spikeChannelTypeSelector->setSelectedId(currentId, dontSendNotification);
    }

}

void SpikeDetectorEditor::buttonClicked(Button* button)
{

    if (button == configureButton.get())
    {

        SpikeDetector* processor = (SpikeDetector*)getProcessor();
        
        Array<SpikeChannel*> spikeChannels = processor->getSpikeChannelsForStream(getCurrentStream());
        std::cout << spikeChannels.size() << " spike channels found." << std::endl;

        currentConfigWindow = new PopupConfigurationWindow(this,
                                                           spikeChannels);

        CallOutBox& myBox
            = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(currentConfigWindow), 
                button->getScreenBounds(),
                nullptr);

        myBox.setDismissalMouseClicksAreAlwaysConsumed(true);
        
        return;
    }
    else if (button == plusButton.get())
    {
        
        int numSpikeChannelsToAdd = spikeChannelCountLabel->getText().getIntValue();
        SpikeChannel::Type channelType = (SpikeChannel::Type) spikeChannelTypeSelector->getSelectedId();
        
        addSpikeChannels(channelType, numSpikeChannelsToAdd);

    }

}

void SpikeDetectorEditor::updateSettings()
{
    SpikeDetector* processor = (SpikeDetector*)getProcessor();

    if (processor->getSpikeChannelsForStream(getCurrentStream()).size() > 0)
        configureButton->setEnabled(true);
    else
        configureButton->setEnabled(false);
    
    //if (currentConfigWindow != nullptr)
     //   currentConfigWindow->update(processor->getSpikeChannelsForStream(getCurrentStream()));

}

void SpikeDetectorEditor::addSpikeChannels(SpikeChannel::Type type, int count)
{
    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    std::cout << "Editor adding " << count << " spike channels with " << SpikeChannel::getNumChannels(type) << " electrodes." << std::endl;

    for (int i = 0; i < count; i++)
        processor->addSpikeChannel(type, getCurrentStream());

    CoreServices::updateSignalChain(this);

}


void SpikeDetectorEditor::removeSpikeChannels(Array<SpikeChannel*> spikeChannelsToRemove)
{

    SpikeDetector* processor = (SpikeDetector*)getProcessor();
    
    for (auto spikeChannel : spikeChannelsToRemove)
        processor->removeSpikeChannel(spikeChannel);
    
    CoreServices::updateSignalChain(this);

}

