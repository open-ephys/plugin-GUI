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

    desiredWidth = 400;

    configureButton = std::make_unique< UtilityButton>("configure", titleFont);
    configureButton->addListener(this);
    configureButton->setRadius(3.0f);
    configureButton->setBounds(15, 42, 74, 20);
    addAndMakeVisible(configureButton.get());
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
    }

}

void SpikeDetectorEditor::updateSettings()
{
    SpikeDetector* processor = (SpikeDetector*)getProcessor();


    if (currentConfigWindow != nullptr)
        currentConfigWindow->update(processor->getSpikeChannelsForStream(getCurrentStream()));

}

void SpikeDetectorEditor::addSpikeChannels(SpikeChannel::Type type, int count)
{
    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    std::cout << "Adding " << count << " spike channels with " << SpikeChannel::getNumChannels(type) << " electrodes." << std::endl;

    //for (int i = 0; i < count; i++)
    //    processor->addSpikeChannel(type, getCurrentStream());

    currentConfigWindow->update(processor->getSpikeChannelsForStream(getCurrentStream()));

    CoreServices::updateSignalChain(this);

}


void SpikeDetectorEditor::removeSpikeChannel(int index)
{
    std::cout << "Deleting electrode number " << index << std::endl;
    SpikeDetector* processor = (SpikeDetector*)getProcessor();
    
    //if (processor->removeSpikeChannel(index, getCurrentStream()))
    //{
    //    currentConfigWindow->update(processor->getSpikeChannelsForStream(getCurrentStream()));
    //}
}

