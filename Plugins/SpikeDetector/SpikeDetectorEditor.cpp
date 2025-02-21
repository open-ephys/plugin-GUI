/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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
#include "PopupConfigurationWindow.h"

#include "SpikeDetectorActions.h"

#include <stdio.h>

SpikeDetectorEditor::SpikeDetectorEditor (GenericProcessor* parentNode)
    : GenericEditor (parentNode),
      currentConfigWindow (nullptr)

{
    desiredWidth = 220;

    configureButton = std::make_unique<UtilityButton> ("CONFIGURE");
    configureButton->setFont (FontOptions (14.0f));
    configureButton->setComponentID ("config_spikes");
    configureButton->addListener (this);
    configureButton->setBounds (70, 60, 80, 30);
    addAndMakeVisible (configureButton.get());
}

SpikeDetectorEditor::~SpikeDetectorEditor()
{
    if (currentConfigWindow != nullptr)
    {
        currentConfigWindow->removeComponentListener (this);
        currentConfigWindow->tableModel->update (Array<SpikeChannel*> ());
    }
}

void SpikeDetectorEditor::selectedStreamHasChanged()
{
    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    Array<SpikeChannel*> spikeChannels = processor->getSpikeChannelsForStream (getCurrentStream());
}

void SpikeDetectorEditor::buttonClicked (Button* button)
{
    if (button == configureButton.get())
    {
        SpikeDetector* processor = (SpikeDetector*) getProcessor();

        Array<SpikeChannel*> spikeChannels = processor->getSpikeChannelsForStream (getCurrentStream());

        currentConfigWindow = new PopupConfigurationWindow (this,
                                                            configureButton.get(),
                                                            spikeChannels,
                                                            acquisitionIsActive);

        CoreServices::getPopupManager()->showPopup (std::unique_ptr<PopupComponent> (currentConfigWindow), button);

        currentConfigWindow->addComponentListener (this);
    }
}

void SpikeDetectorEditor::updateConfigurationWindow()
{
    if (currentConfigWindow != nullptr)
    {
        SpikeDetector* processor = (SpikeDetector*) getProcessor();
        currentConfigWindow->update (processor->getSpikeChannelsForStream (getCurrentStream()));
    }
}

void SpikeDetectorEditor::addSpikeChannels (PopupConfigurationWindow* window, SpikeChannel::Type type, int count, Array<int> startChannels)
{
    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    if (auto stream = processor->getDataStream (getCurrentStream()))
    {
        int nextAvailableChannel = processor->getNextAvailableChannelForStream (stream->getStreamId());

        AddSpikeChannels* action = new AddSpikeChannels (processor, stream, type, count, startChannels, nextAvailableChannel);

        CoreServices::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
        CoreServices::getUndoManager()->perform ((UndoableAction*) action);

        if (window != nullptr)
            window->update (processor->getSpikeChannelsForStream (getCurrentStream()));
    }
}

void SpikeDetectorEditor::removeSpikeChannels (PopupConfigurationWindow* window, Array<SpikeChannel*> spikeChannelsToRemove, Array<int> indeces)
{
    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    if (auto stream = processor->getDataStream (getCurrentStream()))
    {
        RemoveSpikeChannels* action = new RemoveSpikeChannels (processor, stream, spikeChannelsToRemove, indeces);

        CoreServices::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
        CoreServices::getUndoManager()->perform ((UndoableAction*) action);

        if (window != nullptr)
            window->update (processor->getSpikeChannelsForStream (getCurrentStream()));
    }
}

int SpikeDetectorEditor::getNumChannelsForCurrentStream()
{
    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    uint16 currentStream = getCurrentStream();

    if (currentStream == 0)
        return 0;

    DataStream* stream = processor->getDataStream (getCurrentStream());

    if (stream != nullptr)
        return stream->getChannelCount();
    else
        return 0;
}

void SpikeDetectorEditor::componentBeingDeleted (Component& component)
{
    if (currentConfigWindow != nullptr)
    {
        currentConfigWindow->removeComponentListener (this);
        currentConfigWindow = nullptr;
    }
}
