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

#include "RecordNodeEditor.h"
#include "../../CoreServices.h"
#include "RecordNode.h"
#include <stdio.h>

StreamMonitor::StreamMonitor (RecordNode* rn, uint64 id)
    : LevelMonitor (rn),
      streamId (id)
{
    startTimerHz (10);
}

StreamMonitor::~StreamMonitor() {}

void StreamMonitor::timerCallback()
{
    if (((RecordNode*) processor)->recordThread->isThreadRunning())
        setFillPercentage (((RecordNode*) processor)->fifoUsage[streamId]);
    else
        setFillPercentage (0.0);
}

DiskMonitor::DiskMonitor (RecordNode* rn)
    : LevelMonitor (rn),
      lastFreeSpace (0.0),
      recordingTimeLeftInSeconds (0),
      dataRate (0.0)
{
    rn->getDiskSpaceChecker()->addListener (this);
    startTimerHz (1);
}

DiskMonitor::~DiskMonitor()
{
    if (((RecordNode*) processor)->getDiskSpaceChecker() != nullptr)
        ((RecordNode*) processor)->getDiskSpaceChecker()->removeListener (this);
}

void DiskMonitor::update (float dataRate, int64 bytesFree, float timeLeft)
{
    if (((RecordNode*) processor)->recordThread->isThreadRunning())
    {
        String msg = String (bytesFree / pow (2, 30)) + " GB available\n";
        msg += String (int (timeLeft / 60.0f)) + " minutes remaining\n";
        msg += "Data rate: " + String (dataRate * 1000 / pow (2, 20), 2) + " MB/s";
        setTooltip (msg);
    }
    else
    {
        setTooltip (String (bytesFree / pow (2, 30)) + " GB available");
    }
}

void DiskMonitor::updateDiskSpace (float percentage)
{
    setFillPercentage (1.0f - percentage);
}

void DiskMonitor::directoryInvalid()
{
    setFillPercentage (95.0);
    setTooltip ("Invalid directory");
}

void DiskMonitor::lowDiskSpace()
{
    String msg = "Recording stopped. Less than 5 minutes of disk space remaining.";
    AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "WARNING", msg);
}

void DiskMonitor::timerCallback()
{
    repaint();
}

RecordChannelsParameterEditor::RecordChannelsParameterEditor (RecordNode* rn, Parameter* param, int rowHeightPixels, int rowWidthPixels)
    : ParameterEditor (param), recordNode (rn)
{
    int numChannels = int(((MaskChannelsParameter*) param)->getChannelStates().size());
    int selected = 0;
    for (auto chan : ((MaskChannelsParameter*) param)->getChannelStates())
        if (chan)
            selected++;

    uint64 streamId = ((DataStream*) param->getOwner())->getStreamId();
    String streamName = ((DataStream*) param->getOwner())->getName();
    String sourceNodeId = String (((DataStream*) param->getOwner())->getSourceNodeId());

    monitor = std::make_unique<StreamMonitor> (recordNode, streamId);
    monitor->setTooltip (sourceNodeId + " | " + streamName);
    monitor->setComponentID (sourceNodeId + " | " + streamName);
    monitor->addListener (this);
    monitor->setBounds (0, 0, 15, 73);
    addAndMakeVisible (monitor.get());

    setBounds (0, 0, 15, 73);

    editor = monitor.get();
};

void RecordChannelsParameterEditor::channelStateChanged (Array<int> newChannels)
{
    Array<var> newArray;

    for (int i = 0; i < newChannels.size(); i++)
        newArray.add (newChannels[i]);

    param->setNextValue (newArray);

    updateView();
}

Array<int> RecordChannelsParameterEditor::getSelectedChannels()
{
    MaskChannelsParameter* p = (MaskChannelsParameter*) param;
    std::vector<bool> channelStates = p->getChannelStates();

    Array<int> selectedChannels;

    for (int i = 0; i < channelStates.size(); i++)
        if (channelStates[i])
            selectedChannels.add (i);

    return selectedChannels;
}

void RecordChannelsParameterEditor::buttonClicked (Button* label)
{
    if (param == nullptr)
        return;

    MaskChannelsParameter* p = (MaskChannelsParameter*) param;

    std::vector<bool> channelStates = p->getChannelStates();

    auto* channelSelector = new PopupChannelSelector (label, this, channelStates);

    channelSelector->setChannelButtonColour (param->getColour());

    CoreServices::getPopupManager()->showPopup (std::unique_ptr<PopupComponent> (channelSelector), monitor.get());
}

void RecordChannelsParameterEditor::updateView()
{
    /* Do nothing, handled by StreamMonitor timerCallback */
}

void RecordChannelsParameterEditor::resized()
{
    updateBounds();
}

RecordToggleButton::RecordToggleButton (const String& name) : CustomToggleButton() {}

RecordToggleButton::~RecordToggleButton() {}

void RecordToggleButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    g.setColour (Colour (0, 0, 0));
    g.fillRoundedRectangle (0, 0, getWidth(), getHeight(), 0.2 * getWidth());

    if (! getToggleState())
        g.setColour (findColour (ThemeColours::widgetBackground));
    else
        g.setColour (Colour (255, 0, 0));

    g.fillRoundedRectangle (1, 1, getWidth() - 2, getHeight() - 2, 0.2 * getWidth());

    g.setColour (Colour (0, 0, 0));
    g.fillEllipse (0.35 * getWidth(), 0.35 * getHeight(), 0.3 * getWidth(), 0.3 * getHeight());
}

RecordToggleParameterEditor::RecordToggleParameterEditor (Parameter* param) : ParameterEditor (param)
{
    label = std::make_unique<Label> ("Parameter name", param->getDisplayName());
    label->setFont (FontOptions ("Inter", "Regular", 13.0f));
    addAndMakeVisible (label.get());

    toggleButton = std::make_unique<RecordToggleButton> (param->getDisplayName()); //param->getDisplayName());
    toggleButton->setClickingTogglesState (true);
    toggleButton->setTooltip ("Toggle recording state on/off");
    toggleButton->addListener (this);
    toggleButton->setToggleState (true, dontSendNotification);
    toggleButton->setBounds (50, 0, 15, 15);
    addAndMakeVisible (toggleButton.get());

    label->setBounds (0, 0, 100, 15);
    toggleButton->setBounds (95, 0, 15, 15);

    setBounds (0, 0, 150, 20);

    editor = toggleButton.get();
}

void RecordToggleParameterEditor::buttonClicked (Button*)
{
    param->setNextValue (toggleButton->getToggleState());
}

void RecordToggleParameterEditor::updateView()
{
    if (param != nullptr)
        toggleButton->setToggleState (param->getValue(), dontSendNotification);
}

void RecordToggleParameterEditor::resized()
{
}

RecordNodeEditor::RecordNodeEditor (RecordNode* parentNode)
    : GenericEditor (parentNode)
{
    desiredWidth = 165;

    recordNode = parentNode;

    fifoDrawerButton = std::make_unique<FifoDrawerButton> (getNameAndId() + " Fifo Drawer Button");
    fifoDrawerButton->setBounds (4, 40, 10, 78);
    fifoDrawerButton->addListener (this);
    addAndMakeVisible (fifoDrawerButton.get());

    diskMonitor = std::make_unique<DiskMonitor> (recordNode);
    diskMonitor->setBounds (18, 33, 15, 92);
    addAndMakeVisible (diskMonitor.get());

    addPathParameterEditor (Parameter::PROCESSOR_SCOPE, "directory", 42, 32);
    addComboBoxParameterEditor (Parameter::PROCESSOR_SCOPE, "engine", 42, 57);

    for (auto& p : { "directory", "engine" })
    {
        auto* ed = getParameterEditor (p);
        ed->setLayout (ParameterEditor::Layout::nameHidden);
        ed->setBounds (ed->getX(), ed->getY(), 110, ed->getHeight());
    }

    addCustomParameterEditor (new RecordToggleParameterEditor (parentNode->getParameter ("events")), 40, 85);
    addCustomParameterEditor (new RecordToggleParameterEditor (parentNode->getParameter ("spikes")), 40, 107);

    // Show the fifo monitors by default
    fifoDrawerButton->triggerClick();
}

void RecordNodeEditor::timerCallback()
{
    fifoDrawerButton->triggerClick();
    stopTimer();
}

void RecordNodeEditor::updateFifoMonitors()
{
    auto removeExistingMonitors = [this] (std::vector<ParameterEditor*>& monitors)
    {
        for (auto& monitor : monitors)
        {
            for (auto& ed : this->parameterEditors)
            {
                if (ed == monitor)
                {
                    removeChildComponent (monitor);
                    parameterEditors.removeObject (monitor);
                }
            }
        }
        monitors.clear();
    };

    removeExistingMonitors (streamMonitors);
    removeExistingMonitors (syncMonitors);

    int streamCount = 0;
    for (auto& stream : recordNode->getDataStreams())
    {
        uint64 streamId = stream->getStreamId();

        // Add a recording channel selector for each stream
        Parameter* channels = recordNode->getDataStream (streamId)->getParameter ("channels");
        recordNode->getDataStream (streamId)->setColour ("channels", getLookAndFeel().findColour (ProcessorColour::IDs::RECORD_COLOUR));
        addCustomParameterEditor (new RecordChannelsParameterEditor (recordNode, channels), 18 + streamCount * 20, 32);
        parameterEditors.getLast()->setVisible (! getCollapsedState());
        parameterEditors.getLast()->disableUpdateOnSelectedStreamChanged();
        streamMonitors.push_back (parameterEditors.getLast());

        // Add a sync line selector for each stream
        Parameter* syncLineParam = recordNode->getDataStream (streamId)->getParameter ("sync_line");
        Parameter* mainSyncParam = recordNode->getParameter ("main_sync");
        addSyncLineParameterEditor ((TtlLineParameter*) syncLineParam, (SelectedStreamParameter*) mainSyncParam, 18 + streamCount * 20, 110);
        parameterEditors.getLast()->setVisible (! getCollapsedState());
        parameterEditors.getLast()->disableUpdateOnSelectedStreamChanged();
        syncMonitors.push_back (parameterEditors.getLast());

        streamCount++;
    }
}

void RecordNodeEditor::collapsedStateChanged()
{
    if (getCollapsedState())
    {
        for (auto monitor : streamMonitors)
            monitor->setVisible (false);
        for (auto monitor : syncMonitors)
            monitor->setVisible (false);
    }
    else
    {
        for (auto monitor : streamMonitors)
            monitor->setVisible (fifoDrawerButton.get()->getToggleState());
        for (auto monitor : syncMonitors)
            monitor->setVisible (fifoDrawerButton.get()->getToggleState());
    }
}

void RecordNodeEditor::updateSettings()
{
    updateFifoMonitors();
    showFifoMonitors (fifoDrawerButton->getToggleState());
}

void RecordNodeEditor::buttonClicked (Button* button)
{
    if (button == fifoDrawerButton.get())
    {
        showFifoMonitors (button->getToggleState());
    }
}

void RecordNodeEditor::showFifoMonitors (bool show)
{
    int dX = 20 * (recordNode->getNumDataStreams() + 1);
    dX = show ? dX : 0;

    fifoDrawerButton->setBounds (
        4 + dX, fifoDrawerButton->getY(), fifoDrawerButton->getWidth(), fifoDrawerButton->getHeight());

    diskMonitor->setBounds (
        18 + dX, diskMonitor->getY(), diskMonitor->getWidth(), diskMonitor->getHeight());

    for (auto& p : { "directory", "engine", "events", "spikes" })
    {
        auto* ed = getParameterEditor (p);
        ed->setBounds (42 + dX, ed->getY(), ed->getWidth(), ed->getHeight());
    }

    desiredWidth = 165 + dX;

    if (getCollapsedState())
        return;

    for (auto& monitor : streamMonitors)
        monitor->setVisible (show);
    for (auto monitor : syncMonitors)
        monitor->setVisible (show);

    CoreServices::highlightEditor (this);
    deselect();
}

void FifoDrawerButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    g.setColour (Colour (110, 110, 110));
    if (isMouseOver)
        g.setColour (Colour (210, 210, 210));

    g.drawVerticalLine (5, 0.0f, getHeight());
}
