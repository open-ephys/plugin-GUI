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

#include "StreamSelector.h"

#include "DelayMonitor.h"
#include "TTLMonitor.h"
#include "GenericEditor.h"
#include "../GenericProcessor/GenericProcessor.h"

#include "../Settings/DataStream.h"


StreamTableModel::StreamTableModel(StreamSelectorTable* owner_) 
    : owner(owner_)
{

}


void StreamTableModel::cellClicked(int rowNumber, int columnId, const MouseEvent& event)
{

    if (owner->viewedStreamIndex != rowNumber)
    {
        owner->viewedStreamIndex = rowNumber;
        owner->editor->updateSelectedStream(streams[rowNumber]->getStreamId());
    }

}

Component* StreamTableModel::refreshComponentForCell(int rowNumber,
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

        delayMonitors[streams[rowNumber]->getKey()] = delayMonitor;

        return  delayMonitor;
    }
    else if (columnId == StreamTableModel::Columns::TTL_LINE_STATES)
    {

        auto* ttlMonitor = dynamic_cast<TTLMonitor*> (existingComponentToUpdate);

        if (ttlMonitor == nullptr)
        {
            ttlMonitor = new TTLMonitor(8, 8);
        }

        ttlMonitors[streams[rowNumber]->getKey()] = ttlMonitor;

        return  ttlMonitor;
    }
    
    jassert(existingComponentToUpdate == nullptr);

    return nullptr;
}

int StreamTableModel::getNumRows()
{
    return streams.size(); // dataStreams.size();
}

void StreamTableModel::update(Array<const DataStream*> dataStreams_, int viewedStreamIndex_)
{

    streams = dataStreams_;

    delayMonitors.clear();
    ttlMonitors.clear();
    for (auto stream : streams)
    {
        delayMonitors[stream->getKey()] = nullptr;
        ttlMonitors[stream->getKey()] = nullptr;
    }

    viewedStreamIndex = viewedStreamIndex_;

    table->updateContent();

}


void StreamTableModel::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    
    if (rowNumber % 2 == 0)
        g.fillAll(owner->findColour(ThemeColors::componentBackground));
    else
        g.fillAll(owner->findColour(ThemeColors::componentBackground).darker(0.25f));

    if (rowIsSelected)
    {
        g.setColour(Colours::yellow);
        g.drawRect(0, 0, width, height, 2);
        g.fillEllipse(5, 7, 6, 6);
    }

}

void StreamTableModel::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    if (rowNumber >= streams.size())
        return;

    if (columnId == StreamTableModel::Columns::PROCESSOR_ID)
    {
        g.setFont(12);
        g.setColour(owner->editor->findColour(ThemeColors::defaultText));
        g.drawText(String(streams[rowNumber]->getSourceNodeId()), 2, 0, width - 4, height, Justification::centredLeft);
    }
    else if (columnId == StreamTableModel::Columns::NAME)
    {
        g.setFont(12);
        g.setColour(owner->editor->findColour(ThemeColors::defaultText));
        g.drawText(String(streams[rowNumber]->getName()), 2, 0, width - 5, height, Justification::centredLeft);
    }
    else if (columnId == StreamTableModel::Columns::NUM_CHANNELS)
    {
        g.setFont(12);
        g.setColour(owner->editor->findColour(ThemeColors::defaultText));
        g.drawText(String(streams[rowNumber]->getChannelCount()), 2, 0, width - 4, height, Justification::centredLeft);
    }
    else if (columnId == StreamTableModel::Columns::SAMPLE_RATE)
    {
        g.setFont(12);
        g.setColour(owner->editor->findColour(ThemeColors::defaultText));
        g.drawText(String(streams[rowNumber]->getSampleRate()), 2, 0, width - 4, height, Justification::centredLeft);
    }
}

String StreamTableModel::getCellTooltip(int rowNumber, int columnId)
{
    if (rowNumber >= streams.size())
        return String();

    if (columnId == StreamTableModel::Columns::NAME)
    {
        return streams[rowNumber]->getName();
    }

    return String();
}



StreamSelectorTable::StreamSelectorTable(GenericEditor* ed_) :
    editor(ed_),
    streamInfoViewWidth(130),
    streamInfoViewHeight(80),
    viewedStreamIndex(0)
{

    tableModel = std::make_unique<StreamTableModel>(this);
    streamTable.reset(createTableView());
    tableModel->table = streamTable.get();

    addAndMakeVisible(streamTable.get());
    streamTable->setBounds(2, 20, 232, 70);
    streamTable->getViewport()->setScrollBarsShown(true, false, true, false);
    streamTable->getViewport()->setScrollBarThickness(10);

    expanderButton = std::make_unique<ExpanderButton>();
    addAndMakeVisible(expanderButton.get());
    expanderButton->setBounds(222, 4, 15, 15);
    expanderButton->addListener(this);

}

TableListBox* StreamSelectorTable::createTableView(bool expanded)
{
    TableListBox* table = new TableListBox("Stream Table", tableModel.get());
    
    table->setHeader(std::make_unique<TableHeaderComponent>());

    table->getHeader().addColumn(" ", StreamTableModel::Columns::SELECTED, 12, 12, 12, TableHeaderComponent::notResizableOrSortable);
    table->getHeader().addColumn("NAME", StreamTableModel::Columns::NAME, 94, 94, 94, TableHeaderComponent::notResizableOrSortable);
    table->getHeader().addColumn("DELAY", StreamTableModel::Columns::DELAY, 50, 50, 50, TableHeaderComponent::notResizableOrSortable);
    table->getHeader().addColumn("TTL", StreamTableModel::Columns::TTL_LINE_STATES, 60, 60, 60, TableHeaderComponent::notResizableOrSortable);

    if (expanded)
    {
        table->getHeader().addColumn("ID", StreamTableModel::Columns::PROCESSOR_ID, 30, 30, 30, TableHeaderComponent::notResizableOrSortable);
        table->getHeader().addColumn("# CH", StreamTableModel::Columns::NUM_CHANNELS, 30, 30, 30, TableHeaderComponent::notResizableOrSortable);
        table->getHeader().addColumn("Hz", StreamTableModel::Columns::SAMPLE_RATE, 40, 40, 40, TableHeaderComponent::notResizableOrSortable);
        table->getHeader().addColumn("", StreamTableModel::Columns::ENABLED, 15, 15, 15, TableHeaderComponent::notResizableOrSortable);
    }

    if (expanded)
        table->setHeaderHeight(24);
    else
        table->setHeaderHeight(0);

    table->setRowHeight(20);
    table->setMultipleSelectionEnabled(false);

    return table;
}


StreamSelectorTable::~StreamSelectorTable()
{
}

void StreamSelectorTable::buttonClicked(Button* button)
{
    if (button == expanderButton.get())
    {
        LOGD("EXPANDER BUTTON CLICKED ");

        auto* table = createTableView(true);
        table->setBounds(0, 0, 331, streams.size() * 20 + 24);
        table->selectRow(viewedStreamIndex);
        tableModel->table = table;

        CallOutBox& myBox
            = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(table),
                button->getScreenBounds(),
                nullptr);

        myBox.setDismissalMouseClicksAreAlwaysConsumed(true);
        myBox.addComponentListener(this);

        editor->updateDelayAndTTLMonitors();

    }
}


void StreamSelectorTable::componentBeingDeleted(Component& component)
{
    LOGD("POPUP TABLE CLOSED");

    tableModel->table = streamTable.get();
    streamTable->selectRow(viewedStreamIndex);

    Array<const DataStream*> newStreams;

    for (auto stream : streams)
    {
        newStreams.add(stream);
    }

    tableModel->update(newStreams, viewedStreamIndex);

    editor->updateDelayAndTTLMonitors();

}

int StreamSelectorTable::getDesiredWidth() 
{ 
    return 240;
}


bool StreamSelectorTable::checkStream(const DataStream* streamToCheck)
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

    if (streamStates.count(streamToCheck->getStreamId()) > 0)
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

TTLMonitor* StreamSelectorTable::getTTLMonitor(const DataStream* stream)
{
    return tableModel->getTTLMonitor(stream->getKey());
}

DelayMonitor* StreamSelectorTable::getDelayMonitor(const DataStream* stream)
{
    return tableModel->getDelayMonitor(stream->getKey());
}

void StreamSelectorTable::startAcquisition()
{
    startTimer(50);
}

void StreamSelectorTable::stopAcquisition()
{
    stopTimer();
}

void StreamSelectorTable::timerCallback()
{

    for (auto stream : streams)
    {
        TTLMonitor* ttlMonitor = tableModel->getTTLMonitor(stream->getKey());

        if (ttlMonitor != nullptr)
            ttlMonitor->repaint(); // postCommandMessage(0);
    }

    counter++;

    if (counter == 10)
    {
        counter = 0;

        for (auto stream : streams)
        {
            DelayMonitor* delayMonitor = tableModel->getDelayMonitor(stream->getKey());

            if (delayMonitor != nullptr)
                delayMonitor->repaint(); // postCommandMessage(0);

        }
    }
}

void StreamSelectorTable::setStreamEnabledState(uint16 streamId, bool isEnabled)
{
    //LOGD("Setting state for stream ", streamId, ":  ", isEnabled);
    streamStates[streamId] = isEnabled;
}

void StreamSelectorTable::resized()
{
	//viewport->setBounds(5, 5, 300, getHeight() - 10);
}


int StreamSelectorTable::getViewedIndex()
{

    return viewedStreamIndex;
}

void StreamSelectorTable::setViewedIndex(int i)
{

    if (i >= 0 && i < streams.size())
    {
        viewedStreamIndex = i;
        streamTable->selectRow(viewedStreamIndex);
    }

}


void StreamSelectorTable::paint(Graphics& g)
{
    g.setColour(findColour(ThemeColors::widgetBackground));
    g.fillRoundedRectangle(1.0f, 1.0f, (float)getWidth() - 6.0f, (float)getHeight() - 2.0f, 5.0f);
    g.setColour(findColour(ThemeColors::defaultText));
    g.setFont(Font("Inter", "Medium", 13));
    g.drawText("   Available data streams: ", Rectangle<float>(150.0f, 20.0f), Justification::left);

    g.setColour(findColour(ThemeColors::outline).withAlpha(0.8f));
    g.drawRoundedRectangle(1.0f, 1.0f, (float)getWidth() - 6.0f, (float)getHeight() - 2.0f, 5.0f, 1.0f);
    g.fillRect(1.0f, 19.0f, (float)getWidth() - 6.0f, 1.0f);
   
}


const DataStream* StreamSelectorTable::getCurrentStream()
{
    if (streams.size() > 0)
        return streams[viewedStreamIndex];
    else
        return nullptr;

}


void StreamSelectorTable::add(const DataStream* stream)
{

    streams.add(stream);

}

void StreamSelectorTable::beginUpdate()
{
    streams.clear();


}

uint16 StreamSelectorTable::finishedUpdate()
{

    Array<const DataStream*> newStreams;

    for (auto stream : streams)
    {
        newStreams.add(stream);
    }

    tableModel->update(newStreams, viewedStreamIndex);

    

    if (streams.size() == 0)
    {
        expanderButton->setEnabled(false);
        return 0;
    }
    else {
        expanderButton->setEnabled(true);

        if (viewedStreamIndex < streams.size())
        {
            streamTable->selectRow(viewedStreamIndex);
            return streams[viewedStreamIndex]->getStreamId();
        }
        else
        {
            viewedStreamIndex = streams.size() - 1;
            streamTable->selectRow(viewedStreamIndex);
            return streams[viewedStreamIndex]->getStreamId();
        }

    }

}

void StreamSelectorTable::remove(const DataStream* stream)
{
    if (streams.contains(stream))
        streams.remove(streams.indexOf(stream));
}


StreamEnableButton::StreamEnableButton(const String& name) : 
    Button(name), isEnabled(true)
{
}

void StreamEnableButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
 
    if (!getToggleState())
    {
        g.setColour(Colours::darkgrey);
        g.drawRect(0, 0, getWidth(), getHeight(), 1.0);
        g.drawLine(0, 0, getWidth(), getHeight(), 1.0);
        g.drawLine(0, getHeight(), getWidth(), 0, 1.0);

        return;
    }

    g.setColour(Colours::black);
    g.drawRect(0, 0, getWidth(), getHeight(), 1.0);
}

ExpanderButton::ExpanderButton() :
    Button("Expander"), 
    isEnabled(true)
{

    const float height = 9.0f;
    const float width = 9.0f;

    // Draw the lower left arrow
    iconPath.startNewSubPath(0.0f, height / 2);
    iconPath.lineTo(0.0f, height);
    iconPath.lineTo(width / 2, height);

    // Draw the upper right arrow
	iconPath.startNewSubPath(width / 2, 0.0f);
    iconPath.lineTo(width, 0.0f);
    iconPath.lineTo(width, height / 2);

    // Draw the diagonal line
    iconPath.startNewSubPath(0.0f, height);
    iconPath.lineTo(width, 0.0f);
    
}

void ExpanderButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (isMouseOver)
        g.setColour(findColour(ThemeColors::defaultText).withAlpha(0.6f));
    else
        g.setColour(findColour(ThemeColors::defaultText));

    g.strokePath(iconPath, PathStrokeType(1.5f));
}
