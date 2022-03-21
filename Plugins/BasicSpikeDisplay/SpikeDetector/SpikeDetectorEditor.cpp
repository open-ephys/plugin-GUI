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

    desiredWidth = 220;
    
    configureButton = std::make_unique<UtilityButton>("configure", titleFont);
    configureButton->addListener(this);
    configureButton->setRadius(3.0f);
    configureButton->setBounds(70, 60, 80, 30);
    addAndMakeVisible(configureButton.get());
    
}

void SpikeDetectorEditor::selectedStreamHasChanged()
{
    SpikeDetector* processor = (SpikeDetector*)getProcessor();

    Array<SpikeChannel*> spikeChannels = processor->getSpikeChannelsForStream(getCurrentStream());
}

void SpikeDetectorEditor::buttonClicked(Button* button)
{

    if (button == configureButton.get())
    {

        SpikeDetector* processor = (SpikeDetector*)getProcessor();
        
        Array<SpikeChannel*> spikeChannels = processor->getSpikeChannelsForStream(getCurrentStream());
        //std::cout << spikeChannels.size() << " spike channels found." << std::endl;

        currentConfigWindow = new PopupConfigurationWindow(this,
                                                           spikeChannels,
                                                           acquisitionIsActive);

        CallOutBox& myBox
            = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(currentConfigWindow), 
                button->getScreenBounds(),
                nullptr);

        myBox.setDismissalMouseClicksAreAlwaysConsumed(true);
        
        return;
    }

}

void SpikeDetectorEditor::addSpikeChannels(PopupConfigurationWindow* window, SpikeChannel::Type type, int count, Array<int> startChannels)
{
    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    for (int i = 0; i < count; i++)
    {
        int startChannel = -1;
        
        if (i < startChannels.size())
            startChannel = startChannels[i];

        processor->addSpikeChannel(type, getCurrentStream(), startChannel);
    }

    CoreServices::updateSignalChain(this);
        
    if (window != nullptr)
        window->update(processor->getSpikeChannelsForStream(getCurrentStream()));

}


void SpikeDetectorEditor::removeSpikeChannels(PopupConfigurationWindow* window, Array<SpikeChannel*> spikeChannelsToRemove)
{

    SpikeDetector* processor = (SpikeDetector*)getProcessor();
    
    for (auto spikeChannel : spikeChannelsToRemove)
        processor->removeSpikeChannel(spikeChannel);
    
    CoreServices::updateSignalChain(this);

    if (window != nullptr)
        window->update(processor->getSpikeChannelsForStream(getCurrentStream()));


}

int SpikeDetectorEditor::getNumChannelsForCurrentStream()
{
    SpikeDetector* processor = (SpikeDetector*)getProcessor();

    uint16 currentStream = getCurrentStream();

    if (currentStream == 0)
        return 0;

    DataStream* stream = processor->getDataStream(getCurrentStream());

    if (stream != nullptr)
        return stream->getChannelCount();
    else
        return 0;
}