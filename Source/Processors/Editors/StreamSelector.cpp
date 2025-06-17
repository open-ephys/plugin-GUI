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

#include "StreamSelector.h"

#include "../GenericProcessor/GenericProcessor.h"
#include "../RecordNode/RecordNodeEditor.h"
#include "DelayMonitor.h"
#include "GenericEditor.h"
#include "TTLMonitor.h"
#include "VisualizerEditor.h"

#include "../Settings/DataStream.h"

StreamTableModel::StreamTableModel (StreamSelectorTable* owner_)
    : owner (owner_)
{
}

void StreamTableModel::cellClicked (int rowNumber, int columnId, const MouseEvent& event)
{
    if (event.mods.isLeftButtonDown())
    {
        if (owner->viewedStreamIndex != rowNumber)
        {
            owner->viewedStreamIndex = rowNumber;

            bool foundSelectedStreamParam = false;

            for (auto param : owner->editor->getProcessor()->getParameters())
            {
                if (param->getType() == Parameter::ParameterType::SELECTED_STREAM_PARAM
                    && ((SelectedStreamParameter*) param)->shouldSyncWithStreamSelector())
                {
                    param->setNextValue (rowNumber);
                    foundSelectedStreamParam = true;
                    break;
                }
            }

            if (owner->editor->isVisualizerEditor())
            {
                auto* visualizerEditor = dynamic_cast<VisualizerEditor*> (owner->editor);

                if (visualizerEditor->canvas != nullptr)
                {
                    for (auto param : visualizerEditor->canvas->getParameters())
                    {
                        if (param->getType() == Parameter::ParameterType::SELECTED_STREAM_PARAM
                            && ((SelectedStreamParameter*) param)->shouldSyncWithStreamSelector())
                        {
                            param->setNextValue (rowNumber);
                            foundSelectedStreamParam = true;
                            break;
                        }
                    }
                }
            }

            if (! foundSelectedStreamParam)
                owner->editor->updateSelectedStream (streams[rowNumber]->getStreamId());

            table->repaint();
        }

        return;
    }
    else if (event.mods.isRightButtonDown() && owner->editor->getProcessor()->isFilter())
    {
        PopupMenu m;
        bool streamState = owner->checkStream (streams[rowNumber]);

        String enableText = (streamState ? "Disable" : "Enable") + String (" stream");
        m.addItem (1, enableText);

        int result = m.showMenu (PopupMenu::Options().withStandardItemHeight (20));

        if (result == 1)
        {
            if (auto* param = streams[rowNumber]->getParameter ("enable_stream"))
            {
                param->setNextValue (! streamState);
            }
        }

        return;
    }
}

Component* StreamTableModel::refreshComponentForCell (int rowNumber,
                                                      int columnId,
                                                      bool isRowSelected,
                                                      Component* existingComponentToUpdate)
{
    if (columnId == StreamTableModel::Columns::DELAY)
    {
        auto* delayMonitor = dynamic_cast<DelayMonitor*> (existingComponentToUpdate);

        if (delayMonitor == nullptr)
        {
            delayMonitor = new DelayMonitor();
        }

        return delayMonitor;
    }
    else if (columnId == StreamTableModel::Columns::TTL_LINE_STATES)
    {
        auto* ttlMonitor = dynamic_cast<TTLMonitor*> (existingComponentToUpdate);

        if (ttlMonitor == nullptr)
        {
            ttlMonitor = new TTLMonitor (8, 8);
        }

        return ttlMonitor;
    }
    else if (columnId == StreamTableModel::Columns::START_TIME)
    {
        auto* syncStartTimeMonitor = dynamic_cast<SyncStartTimeMonitor*> (existingComponentToUpdate);

        if (syncStartTimeMonitor == nullptr)
        {
            syncStartTimeMonitor = new SyncStartTimeMonitor();
        }

        return syncStartTimeMonitor;
    }
    else if (columnId == StreamTableModel::Columns::LATEST_SYNC)
    {
        auto* lastSyncEventMonitor = dynamic_cast<LastSyncEventMonitor*> (existingComponentToUpdate);

        if (lastSyncEventMonitor == nullptr)
        {
            lastSyncEventMonitor = new LastSyncEventMonitor();
        }

        return lastSyncEventMonitor;
    }
    else if (columnId == StreamTableModel::Columns::SYNC_ACCURACY)
    {
        auto* syncAccuracyMonitor = dynamic_cast<SyncAccuracyMonitor*> (existingComponentToUpdate);

        if (syncAccuracyMonitor == nullptr)
        {
            syncAccuracyMonitor = new SyncAccuracyMonitor();
        }

        return syncAccuracyMonitor;
    }

    jassert (existingComponentToUpdate == nullptr);

    return nullptr;
}

int StreamTableModel::getNumRows()
{
    return streams.size(); // dataStreams.size();
}

void StreamTableModel::update (Array<const DataStream*> dataStreams_)
{
    streams = dataStreams_;
    table->updateContent();
}

void StreamTableModel::paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowNumber >= streams.size())
        return;

    if (owner->checkStream (streams[rowNumber]))
    {
        if (rowNumber % 2 == 0)
            g.fillAll (owner->findColour (ThemeColours::componentBackground));
        else
            g.fillAll (owner->findColour (ThemeColours::componentBackground).darker (0.25f));
    }
    else
    {
        g.fillAll (Colours::red.withAlpha (0.5f));
    }

    if (owner->getViewedIndex() == rowNumber)
    {
        g.setColour (Colours::yellow);
        g.drawRect (0, 0, width, height, 2);
        g.fillEllipse (5, 7, 6, 6);
    }
}

void StreamTableModel::paintCell (Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (rowNumber >= streams.size())
        return;

    g.setColour (owner->editor->findColour (ThemeColours::defaultText));
    g.setFont (FontOptions (12.0f));

    if (columnId == StreamTableModel::Columns::PROCESSOR_ID)
    {
        g.drawText (String (streams[rowNumber]->getSourceNodeId()), 2, 0, width - 4, height, Justification::centredLeft);
    }
    else if (columnId == StreamTableModel::Columns::NAME)
    {
        g.drawText (String (streams[rowNumber]->getName()), 2, 0, width - 5, height, Justification::centredLeft);
    }
    else if (columnId == StreamTableModel::Columns::NUM_CHANNELS)
    {
        g.drawText (String (streams[rowNumber]->getChannelCount()), 2, 0, width - 4, height, Justification::centredLeft);
    }
    else if (columnId == StreamTableModel::Columns::SAMPLE_RATE)
    {
        g.drawText (String (streams[rowNumber]->getSampleRate()), 2, 0, width - 4, height, Justification::centredLeft);
    }
}

String StreamTableModel::getCellTooltip (int rowNumber, int columnId)
{
    if (rowNumber >= streams.size())
        return String();

    if (columnId == StreamTableModel::Columns::NAME)
    {
        return streams[rowNumber]->getName();
    }

    return String();
}

void StreamTableModel::listWasScrolled()
{
    owner->editor->updateDelayAndTTLMonitors();

    //if (owner->isRecordNode)
    //{
    //    RecordNodeEditor* recNodeEditor = (RecordNodeEditor*) owner->editor;
    //    recNodeEditor->updateSyncMonitors();
    //}
}

StreamSelectorTable::StreamSelectorTable (GenericEditor* ed_) : editor (ed_),
                                                                streamInfoViewWidth (130),
                                                                streamInfoViewHeight (80),
                                                                viewedStreamIndex (0)
{
    isRecordNode = editor->getProcessor()->isRecordNode();

    tableModel = std::make_unique<StreamTableModel> (this);
    streamTable.reset (createTableView());
    tableModel->table = streamTable.get();

    addAndMakeVisible (streamTable.get());
    streamTable->setBounds (2, 20, 232, 70);
    streamTable->getViewport()->setScrollBarsShown (true, false, true, false);
    streamTable->getViewport()->setScrollBarThickness (10);

    expanderButton = std::make_unique<ExpanderButton>();
    addAndMakeVisible (expanderButton.get());
    expanderButton->setBounds (222, 4, 15, 15);
    expanderButton->addListener (this);
}

TableListBox* StreamSelectorTable::createTableView (bool expanded)
{
    TableListBox* table = new TableListBox ("Stream Table", tableModel.get());

    table->setHeader (std::make_unique<TableHeaderComponent>());

    table->getHeader().addColumn (" ", StreamTableModel::Columns::SELECTED, 12, 12, 12, TableHeaderComponent::notResizableOrSortable);
    table->getHeader().addColumn ("NAME", StreamTableModel::Columns::NAME, 94, 94, 94, TableHeaderComponent::notResizableOrSortable);
    table->getHeader().addColumn ("DELAY", StreamTableModel::Columns::DELAY, 50, 50, 50, TableHeaderComponent::notResizableOrSortable);
    table->getHeader().addColumn ("TTL", StreamTableModel::Columns::TTL_LINE_STATES, 60, 60, 60, TableHeaderComponent::notResizableOrSortable);

    if (expanded)
    {
        table->getHeader().addColumn ("ID", StreamTableModel::Columns::PROCESSOR_ID, 30, 30, 30, TableHeaderComponent::notResizableOrSortable);
        table->getHeader().addColumn ("# CH", StreamTableModel::Columns::NUM_CHANNELS, 30, 30, 30, TableHeaderComponent::notResizableOrSortable);
        table->getHeader().addColumn ("Hz", StreamTableModel::Columns::SAMPLE_RATE, 40, 40, 40, TableHeaderComponent::notResizableOrSortable);
        //table->getHeader().addColumn ("", StreamTableModel::Columns::ENABLED, 15, 15, 15, TableHeaderComponent::notResizableOrSortable);

        if (isRecordNode)
        {
            table->getHeader().addColumn ("Start", StreamTableModel::Columns::START_TIME, 50, 50, 50, TableHeaderComponent::notResizableOrSortable);
            table->getHeader().addColumn ("Tolerance", StreamTableModel::Columns::SYNC_ACCURACY, 55, 50, 50, TableHeaderComponent::notResizableOrSortable);
            table->getHeader().addColumn ("Latest Sync", StreamTableModel::Columns::LATEST_SYNC, 55, 60, 60, TableHeaderComponent::notResizableOrSortable);
        }
    }

    if (expanded)
        table->setHeaderHeight (24);
    else
        table->setHeaderHeight (0);

    table->setRowHeight (20);
    table->setMultipleSelectionEnabled (false);

    return table;
}

StreamSelectorTable::~StreamSelectorTable()
{
    if (expandedTableComponent != nullptr)
    {
        expandedTableComponent->removeComponentListener (this);
    }
}

void StreamSelectorTable::buttonClicked (Button* button)
{
    if (button == expanderButton.get())
    {
        auto* table = createTableView (true);

        int width = 316;

        if (isRecordNode)
            width += 160;

        table->setBounds (0, 0, width, streams.size() * 20 + 24);
        table->selectRow (viewedStreamIndex);
        tableModel->table = table;

        expandedTableComponent = new ExpandedTableComponent (table, this);

        CoreServices::getPopupManager()->showPopup (std::unique_ptr<PopupComponent> (expandedTableComponent), button);

        expandedTableComponent->addComponentListener (this);

        editor->updateDelayAndTTLMonitors();

        if (isRecordNode)
        {
            RecordNodeEditor* recNodeEditor = dynamic_cast<RecordNodeEditor*> (editor);
            recNodeEditor->updateSyncMonitors();
        }
    }
}

void StreamSelectorTable::componentBeingDeleted (Component& component)
{
    expandedTableComponent = nullptr;
    tableModel->table = streamTable.get();
    streamTable->selectRow (viewedStreamIndex);

    Array<const DataStream*> newStreams;

    for (auto stream : streams)
    {
        newStreams.add (stream);
    }

    tableModel->update (newStreams);

    editor->updateDelayAndTTLMonitors();

    if (isRecordNode)
    {
        RecordNodeEditor* recNodeEditor = dynamic_cast<RecordNodeEditor*> (editor);
        recNodeEditor->updateSyncMonitors();
    }
}

int StreamSelectorTable::getDesiredWidth()
{
    return 240;
}

bool StreamSelectorTable::checkStream (const DataStream* streamToCheck)
{
    //StreamInfoView* siv = getStreamInfoView(streamToCheck);

    std::map<uint16, bool>::iterator it = streamStates.begin();

    while (it != streamStates.end())
    {
        // Accessing KEY from element pointed by it.
        uint16 streamId = it->first;
        // Accessing VALUE from element pointed by it.
        bool state = it->second;
        //LOGD(streamId, " :: ", state);

        // Increment the Iterator to point to next entry
        it++;
    }

    if (streamStates.count (streamToCheck->getStreamId()) > 0)
    {
        //LOGD(" Stream Selector returning ", streamStates[streamToCheck->getStreamId()]);
        return streamStates[streamToCheck->getStreamId()];
    }

    else
    {
        //LOGD(" Stream not found, returning 1.");
        return true;
    }
}

SyncStartTimeMonitor* StreamSelectorTable::getSyncStartTimeMonitor (const DataStream* stream)
{
    TableListBox* currentTable = tableModel->table;
    return dynamic_cast<SyncStartTimeMonitor*> (currentTable->getCellComponent (StreamTableModel::Columns::START_TIME, streams.indexOf (stream)));
}

LastSyncEventMonitor* StreamSelectorTable::getlastSyncEventMonitor (const DataStream* stream)
{
    TableListBox* currentTable = tableModel->table;
    return dynamic_cast<LastSyncEventMonitor*> (currentTable->getCellComponent (StreamTableModel::Columns::LATEST_SYNC, streams.indexOf (stream)));
}

SyncAccuracyMonitor* StreamSelectorTable::getSyncAccuracyMonitor (const DataStream* stream)
{
    TableListBox* currentTable = tableModel->table;
    return dynamic_cast<SyncAccuracyMonitor*> (currentTable->getCellComponent (StreamTableModel::Columns::SYNC_ACCURACY, streams.indexOf (stream)));
}

TTLMonitor* StreamSelectorTable::getTTLMonitor (const DataStream* stream)
{
    TableListBox* currentTable = tableModel->table;
    return dynamic_cast<TTLMonitor*> (currentTable->getCellComponent (StreamTableModel::Columns::TTL_LINE_STATES, streams.indexOf (stream)));
}

DelayMonitor* StreamSelectorTable::getDelayMonitor (const DataStream* stream)
{
    TableListBox* currentTable = tableModel->table;
    return dynamic_cast<DelayMonitor*> (currentTable->getCellComponent (StreamTableModel::Columns::DELAY, streams.indexOf (stream)));
}

void StreamSelectorTable::startAcquisition()
{
    // Reset TTL Monitor states
    for (auto stream : streams)
    {
        TTLMonitor* ttlMonitor = getTTLMonitor (stream);

        if (ttlMonitor != nullptr)
        {
            for (int bit = 0; bit < 8; ++bit)
            {
                ttlMonitor->setState (bit, false);
            }
        }
    }

    startTimer (20);
}

void StreamSelectorTable::stopAcquisition()
{
    stopTimer();
}

void StreamSelectorTable::timerCallback()
{
    for (auto stream : streams)
    {
        TTLMonitor* ttlMonitor = getTTLMonitor (stream);

        if (ttlMonitor != nullptr)
            ttlMonitor->repaint();
    }

    counter++;

    if (counter % 10 == 0)
    {
        for (auto stream : streams)
        {
            DelayMonitor* delayMonitor = getDelayMonitor (stream);

            if (delayMonitor != nullptr)
                delayMonitor->repaint();

            if (isRecordNode && counter % 20 == 0)
            {
                SyncStartTimeMonitor* syncStartTimeMonitor = getSyncStartTimeMonitor (stream);

                if (syncStartTimeMonitor != nullptr)
                    syncStartTimeMonitor->repaint();

                LastSyncEventMonitor* lastSyncEventMonitor = getlastSyncEventMonitor (stream);

                if (lastSyncEventMonitor != nullptr)
                    lastSyncEventMonitor->repaint();

                SyncAccuracyMonitor* syncAccuracyMonitor = getSyncAccuracyMonitor (stream);

                if (syncAccuracyMonitor != nullptr)
                    syncAccuracyMonitor->repaint();
            }
        }
    }

    if (counter > 20)
        counter = 0;
}

void StreamSelectorTable::setStreamEnabledState (uint16 streamId, bool isEnabled)
{
    //LOGD("Setting state for stream ", streamId, ":  ", isEnabled);
    streamStates[streamId] = isEnabled;
    tableModel->table->repaint();
    streamTable->repaint();
}

void StreamSelectorTable::resized()
{
    //viewport->setBounds(5, 5, 300, getHeight() - 10);
}

int StreamSelectorTable::getViewedIndex()
{
    return viewedStreamIndex;
}

void StreamSelectorTable::setViewedIndex (int i)
{
    if (i >= 0 && i < streams.size())
    {
        viewedStreamIndex = i;
        streamTable->selectRow (viewedStreamIndex);
    }
}

void StreamSelectorTable::paint (Graphics& g)
{
    g.setColour (findColour (ThemeColours::widgetBackground));
    g.fillRoundedRectangle (1.0f, 1.0f, (float) getWidth() - 6.0f, (float) getHeight() - 2.0f, 5.0f);
    g.setColour (findColour (ThemeColours::defaultText));
    g.setFont (FontOptions ("Inter", "Medium", 13));
    g.drawText ("   Available data streams: ", Rectangle<float> (150.0f, 20.0f), Justification::left);

    g.setColour (findColour (ThemeColours::outline).withAlpha (0.8f));
    g.drawRoundedRectangle (1.0f, 1.0f, (float) getWidth() - 6.0f, (float) getHeight() - 2.0f, 5.0f, 1.0f);
    g.fillRect (1.0f, 19.0f, (float) getWidth() - 6.0f, 1.0f);
}

const DataStream* StreamSelectorTable::getCurrentStream()
{
    if (streams.size() > 0)
        return streams[viewedStreamIndex];
    else
        return nullptr;
}

void StreamSelectorTable::add (const DataStream* stream)
{
    streams.add (stream);

    if (editor->getProcessor()->isFilter())
    {
        if (auto* param = stream->getParameter ("enable_stream"))
        {
            streamStates[stream->getStreamId()] = param->getValue();
            return;
        }
    }

    streamStates[stream->getStreamId()] = true;
}

void StreamSelectorTable::beginUpdate()
{
    streams.clear();
    streamStates.clear();
}

uint16 StreamSelectorTable::finishedUpdate()
{
    Array<const DataStream*> newStreams;

    for (auto stream : streams)
    {
        newStreams.add (stream);
    }

    if (streams.size() == 0)
    {
        expanderButton->setEnabled (false);
        tableModel->table = streamTable.get();
        tableModel->update (newStreams);
        return 0;
    }
    else
    {
        expanderButton->setEnabled (true);

        tableModel->table = streamTable.get();
        tableModel->update (newStreams);

        if (viewedStreamIndex < streams.size())
        {
            streamTable->selectRow (viewedStreamIndex);
            return streams[viewedStreamIndex]->getStreamId();
        }
        else
        {
            viewedStreamIndex = streams.size() - 1;
            streamTable->selectRow (viewedStreamIndex);
            return streams[viewedStreamIndex]->getStreamId();
        }
    }
}

void StreamSelectorTable::remove (const DataStream* stream)
{
    if (streams.contains (stream))
        streams.remove (streams.indexOf (stream));

    if (streamStates.count (stream->getStreamId()) > 0)
        streamStates.erase (stream->getStreamId());
}

// StreamEnableButton::StreamEnableButton (const String& name) : Button (name),
//                                                               isEnabled (true)
// {
// }

// void StreamEnableButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
// {
//     if (! getToggleState())
//     {
//         g.setColour (Colours::darkgrey);
//         g.drawRect (0, 0, getWidth(), getHeight(), 1.0);
//         g.drawLine (0, 0, getWidth(), getHeight(), 1.0);
//         g.drawLine (0, getHeight(), getWidth(), 0, 1.0);

//         return;
//     }

//     g.setColour (Colours::black);
//     g.drawRect (0, 0, getWidth(), getHeight(), 1.0);
// }

ExpanderButton::ExpanderButton() : Button ("Expander")
{
    const float height = 9.0f;
    const float width = 9.0f;

    // Draw the lower left arrow
    iconPath.startNewSubPath (0.0f, height / 2);
    iconPath.lineTo (0.0f, height);
    iconPath.lineTo (width / 2, height);

    // Draw the upper right arrow
    iconPath.startNewSubPath (width / 2, 0.0f);
    iconPath.lineTo (width, 0.0f);
    iconPath.lineTo (width, height / 2);

    // Draw the diagonal line
    iconPath.startNewSubPath (0.0f, height);
    iconPath.lineTo (width, 0.0f);
}

void ExpanderButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (isMouseOver || ! isEnabled())
        g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.6f));
    else
        g.setColour (findColour (ThemeColours::defaultText));

    g.strokePath (iconPath, PathStrokeType (1.5f));
}

ExpandedTableComponent::ExpandedTableComponent (TableListBox* table, Component* parent)
    : PopupComponent (parent)
{
    expandedTable.reset (table);
    addAndMakeVisible (expandedTable.get());
    setSize (expandedTable->getWidth(), expandedTable->getHeight());
}

void ExpandedTableComponent::updatePopup()
{
    if (expandedTable->getNumRows() == 0)
    {
        findParentComponentOfClass<CallOutBox>()->exitModalState (0);
        return;
    }

    if (auto* tableModel = dynamic_cast<StreamTableModel*> (expandedTable->getTableListBoxModel()))
    {
        tableModel->table = expandedTable.get();
    }

    expandedTable->setSize (expandedTable->getWidth(), expandedTable->getNumRows() * 20 + 24);
    setSize (expandedTable->getWidth(), expandedTable->getHeight());
    expandedTable->repaint();
}
