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

StreamInfoView::StreamInfoView(const DataStream* stream_, GenericEditor* editor_, bool isEnabled_) :
    isEnabled(isEnabled_), 
    stream(stream_), 
    streamId(stream_->getStreamId()),
    editor(editor_), 
    streamIsStillNeeded(true),
    acquisitionIsActive(false)
{
    LOGD(editor->getNameAndId(), " adding stream ", getStreamId(), " with ", stream->getChannelCount(), " channels ");

    updateInfoString();

    if (editor->getProcessor()->isFilter())
    {
        enableButton = std::make_unique<StreamEnableButton>("x");
        enableButton->addListener(this);
        enableButton->setClickingTogglesState(true);
        enableButton->setToggleState(true, dontSendNotification);
        addAndMakeVisible(enableButton.get());
        enabledString = "Bypass";
    }
    else {
        enabledString = "";
    }
        
    streamName = stream->getName();
    sourceNodeId = stream->getNodeId();

    delayMonitor = nullptr;
    ttlMonitor = nullptr;

}

void StreamInfoView::setDelayMonitor(DelayMonitor* monitor)
{

    if (monitor != delayMonitor)
    {
        LOGD(streamName, " UPDATING DELAY MONITOR to ", monitor);
        delayMonitor = monitor;
        delayMonitor->setLookAndFeel(&editor->getLookAndFeel());
    }
    
}

void StreamInfoView::setTTLMonitor(TTLMonitor* monitor)
{
    if (monitor != ttlMonitor)
    {
        LOGD(streamName, " UPDATING TTL MONITOR to ", monitor);
        ttlMonitor = monitor;
        delayMonitor->setLookAndFeel(&editor->getLookAndFeel());
    }
    
}



uint16 StreamInfoView::getStreamId() const
{
    return stream->getStreamId();
}

String StreamInfoView::getStreamName() const
{
    return streamName;
}

int StreamInfoView::getStreamSourceNodeId() const
{
    return sourceNodeId;
}

void StreamInfoView::updateInfoString()
{
    
    String channelString = " channel";
    
    if (stream->getChannelCount() > 1)
        channelString += "s";
    
    infoString = stream->getSourceNodeName() + " (" + String(stream->getSourceNodeId()) + ")"
        + "\n"
        + String(stream->getChannelCount()) + channelString + " @ " +
        String(stream->getSampleRate()) + " Hz";

}

bool StreamInfoView::getEnabledState() const
{
    return isEnabled;
}

void StreamInfoView::setEnabled(bool state)
{
    isEnabled = state;

    if (enableButton != nullptr)
    {
        enableButton->setToggleState(state, dontSendNotification);
        enableButton->repaint();
    }

    if (delayMonitor != nullptr)
        delayMonitor->setEnabled(state);

    repaint();
}

void StreamInfoView::startAcquisition()
{
    if (delayMonitor != nullptr)
    {
        delayMonitor->startAcquisition();
        ttlMonitor->startAcquisition();
    }

    acquisitionIsActive = true;

    if (enableButton != nullptr)
        enableButton->setClickingTogglesState(false);
    
}

void StreamInfoView::stopAcquisition()
{
    if (delayMonitor != nullptr)
    {
        delayMonitor->stopAcquisition();
        ttlMonitor->stopAcquisition();
    }

    acquisitionIsActive = false;

    if (enableButton != nullptr)
        enableButton->setClickingTogglesState(true);
    
}

void StreamInfoView::beginUpdate()
{
    streamIsStillNeeded = false;
}

void StreamInfoView::update(const DataStream* newStream)
{
    streamIsStillNeeded = true;

    streamId = newStream->getStreamId();

    stream = newStream;

    updateInfoString();
}

void StreamInfoView::buttonClicked(Button* button)
{
    if (button == enableButton.get() && !acquisitionIsActive)
    {
        setEnabled(!isEnabled);
        enableButton->setToggleState(isEnabled, dontSendNotification);

        //LOGD("Stream ", getStreamId(), " enabled: ", isEnabled);

        repaint();
        editor->streamEnabledStateChanged(getStreamId(), isEnabled);
    }
}


void StreamInfoView::resized()
{
    
}

void StreamInfoView::paint(Graphics& g)
{

    if (isEnabled)
        g.setColour(Colours::black);
    else
        g.setColour(Colours::darkgrey);

    g.setFont(Font("Fira Sans", "SemiBold", 12));
    g.drawMultiLineText(infoString, 5, 18, getWidth() +100, Justification::left);
    g.drawText(enabledString, 22, 38, 120, 12, Justification::left);

}

StreamSelector::StreamSelector(GenericEditor* ed_) :
    editor(ed_),
    streamInfoViewWidth(130),
    streamInfoViewHeight(80),
    scrollOffset(0),
    viewedStreamIndex(0)
{

    viewport = std::make_unique<Viewport>();
    addAndMakeVisible(viewport.get());

    viewedComponent = std::make_unique<Component>();
    viewport->setViewedComponent(viewedComponent.get(), false);
    viewport->setScrollBarsShown(false, false, false, false);

    leftScrollButton = std::make_unique<StreamScrollButton>("<");
    leftScrollButton->addListener(this);
    leftScrollButton->setClickingTogglesState(false);
    addAndMakeVisible(leftScrollButton.get());

    rightScrollButton = std::make_unique<StreamScrollButton>(">");
    rightScrollButton->addListener(this);
    rightScrollButton->setClickingTogglesState(false);
    addAndMakeVisible(rightScrollButton.get());

    streamSelectorButton = std::make_unique<StreamNameButton>("Stream");
    streamSelectorButton->addListener(this);
    streamSelectorButton->setClickingTogglesState(false);
    addAndMakeVisible(streamSelectorButton.get());

    scrollOffset.reset(4);
}

StreamInfoView* StreamSelector::getStreamInfoView(const DataStream* streamToCheck)
{

    for (auto stream : streams)
    {
        if (stream->streamId == streamToCheck->getStreamId())
            return stream;
    }

    return nullptr;
}

bool StreamSelector::checkStream(const DataStream* streamToCheck)
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

TTLMonitor* StreamSelector::getTTLMonitor(const DataStream* stream)
{
    StreamInfoView* siv = getStreamInfoView(stream);

    if (siv != nullptr)
        return siv->getTTLMonitor();
    else
        return nullptr;
}

DelayMonitor* StreamSelector::getDelayMonitor(const DataStream* stream)
{
    StreamInfoView* siv = getStreamInfoView(stream);

    if (siv != nullptr)
        return siv->getDelayMonitor();
    else
        return nullptr;
}

void StreamSelector::startAcquisition()
{
    for (auto stream : streams)
    {
        stream->startAcquisition();
    }
}

void StreamSelector::stopAcquisition()
{
    for (auto stream : streams)
    {
        stream->stopAcquisition();
    }
}

void StreamSelector::setStreamEnabledState(uint16 streamId, bool isEnabled)
{
    //LOGD("Setting state for stream ", streamId, ":  ", isEnabled);
    streamStates[streamId] = isEnabled;
}

void StreamSelector::resized()
{
    viewport->setBounds(5, 20, getWidth() - 10, getHeight() - 30);

    viewedComponent->setBounds(0, 0, streams.size() * streamInfoViewWidth, getHeight() - 30);
    
    leftScrollButton->setBounds(2, 2, 18, 18);
    rightScrollButton->setBounds(getWidth() - 20, 2, 18, 18);
    streamSelectorButton->setBounds(20, 2, getWidth() - 40, 18);

    //LOGD("StreamSelector resized, num streams: ", streams.size());
    //LOGD(" Scroll offset: ", scrollOffset.getCurrentValue());

    for (int i = 0; i < streams.size(); i++)
    {
        streams[i]->setBounds(i * streamInfoViewWidth, 0, streamInfoViewWidth, streamInfoViewHeight);
    }

    if (streams.size() > 0)
        streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getKey());

    viewport->setViewPosition(viewedStreamIndex * streamInfoViewWidth, 0);
}

void StreamSelector::buttonClicked(Button* button)
{
    int currentlyViewedStream = viewedStreamIndex;

    if (button == leftScrollButton.get())
    {
        LOGDD("Scroll left");

        if (viewedStreamIndex != 0)
        {
            viewedStreamIndex -= 1;
            scrollOffset.setTargetValue(viewedStreamIndex * streamInfoViewWidth);
            startTimer(50);

            LOGDD("  Target value: ", viewedStreamIndex * streamInfoViewWidth);

            streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getKey());
            streamSelectorButton->repaint();
        }

    }
    else if (button == rightScrollButton.get())
    {
        LOGDD("Scroll right");

        LOGDD("Total streams: ", getNumStreams());

        if (viewedStreamIndex != streams.size() -1)
        {
            viewedStreamIndex += 1;
            scrollOffset.setTargetValue(viewedStreamIndex * streamInfoViewWidth);
            startTimer(50);

            streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getKey());
            streamSelectorButton->repaint();
        }
    }
    else if (button == streamSelectorButton.get())
    {

        if (streams.size() < 2)
            return;

        PopupMenu menu;

        for (int i = 1; i <= streams.size(); i++)
        {
            menu.addItem(i, // index
                         streams[i-1]->getStream()->getName(), // message
                         true); // isSelectable
        }
     
        const int result = menu.show(); // returns 0 if nothing is selected

        if (result > 0)
        {
            viewedStreamIndex = result - 1;
            scrollOffset.setTargetValue(viewedStreamIndex * streamInfoViewWidth);
            startTimer(50);

            streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getKey());
            streamSelectorButton->repaint();
        }
    }

    if (currentlyViewedStream != viewedStreamIndex)
        editor->updateSelectedStream(streams[viewedStreamIndex]->streamId);
}

int StreamSelector::getViewedIndex()
{
    
    return viewedStreamIndex;
}

void StreamSelector::setViewedIndex(int i)
{

    if (i >= 0 && i < streams.size())
    {
        viewedStreamIndex = i;
        streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getKey());
        streamSelectorButton->repaint();

        viewport->setViewPosition(viewedStreamIndex * streamInfoViewWidth, 0);
        
        //editor->updateSelectedStream(streams[viewedStreamIndex]->streamId);
    }
        
}


void StreamSelector::paint(Graphics& g)
{
    
    g.setGradientFill(ColourGradient(
        Colours::darkgrey, 0.0f, 0.0f,
        Colours::darkgrey.withAlpha(0.25f), 0.0f, 30.0f, false));
    g.fillRoundedRectangle(3, 24, getWidth() - 5, getHeight() - 26, 5.0f);

    g.setColour(findColour(ThemeColors::componentParentBackground));
    g.drawRoundedRectangle(3, 24, getWidth() - 5, getHeight() - 26, 5.0f, 0.5f);
}


const DataStream* StreamSelector::getCurrentStream()
{
    if (streams.size() > 0)
        return streams[0]->getStream();
    else
        return nullptr;
}




void StreamSelector::add(const DataStream* stream)
{

    if (getStreamInfoView(stream) == nullptr)
    {
        streams.add(new StreamInfoView(stream, editor, checkStream(stream)));
        viewedComponent->addAndMakeVisible(streams.getLast());
    }
    else
    {
        getStreamInfoView(stream)->update(stream);
    }
    
    setStreamEnabledState(stream->getStreamId(), checkStream(stream));

}

void StreamSelector::beginUpdate()
{
    for (auto stream : streams)
    {
        stream->beginUpdate();
    }
}

uint16 StreamSelector::finishedUpdate()
{

    Array<StreamInfoView*> streamsToRemove;

    for (auto stream : streams)
    {
        if (!stream->streamIsStillNeeded)
        {
            streamsToRemove.add(stream);
        }
    }

    for (auto stream : streamsToRemove)
    {
        remove(stream);
    }
    
    if (viewedStreamIndex >= streams.size() || viewedStreamIndex < 0)
    {
        viewedStreamIndex = streams.size() - 1;
    }
    
    if (viewedStreamIndex > -1)
    {
        viewport->setViewPosition(viewedStreamIndex * streamInfoViewWidth, 0);
        
        streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getKey());
        streamSelectorButton->repaint();
    }

    
    resized();
    
    if (viewedStreamIndex > -1)
        return streams[viewedStreamIndex]->getStream()->getStreamId();
    else
        return 0;
}

void StreamSelector::remove(StreamInfoView* stream)
{
    if (streams.contains(stream))
        streams.remove(streams.indexOf(stream));

    viewedComponent->removeChildComponent(stream);
}

void StreamSelector::timerCallback()
{

    if (scrollOffset.getCurrentValue() == viewedStreamIndex * streamInfoViewWidth)
        stopTimer();

    float newOffset = scrollOffset.getNextValue();

    viewport->setViewPosition(newOffset, 0);
}



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
        auto* delayMonitor = static_cast<DelayMonitor*> (existingComponentToUpdate);

        if (delayMonitor == nullptr)
        {
            delayMonitor = new DelayMonitor();
            LOGD("CREATING NEW DELAY MONITOR ", delayMonitor);
        }

        streams[rowNumber]->setDelayMonitor(delayMonitor);

        return  delayMonitor; // streams[rowNumber]->getDelayMonitor();
    }
    else if (columnId == StreamTableModel::Columns::TTL_LINE_STATES)
    {

        auto* ttlMonitor = static_cast<TTLMonitor*> (existingComponentToUpdate);

        if (ttlMonitor == nullptr)
        {
            ttlMonitor = new TTLMonitor(8, 8);
            LOGD("CREATING NEW TTL MONITOR ", ttlMonitor);
        }

        streams[rowNumber]->setTTLMonitor(ttlMonitor);

        return  ttlMonitor; // streams[rowNumber]->getDelayMonitor();
    }
    
    jassert(existingComponentToUpdate == nullptr);

    return nullptr;
}

int StreamTableModel::getNumRows()
{
    return streams.size(); // dataStreams.size();
}

void StreamTableModel::update(Array<StreamInfoView*> dataStreams_, int viewedStreamIndex_)
{

    LOGD("StreamTableModel::update()");
    streams = dataStreams_;

    LOGD("Num data streams: ", streams.size());
    viewedStreamIndex = viewedStreamIndex_;

    LOGD("Table::updateContent()");
    table->updateContent();

}


void StreamTableModel::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    
    if (rowNumber % 2 == 0)
        g.fillAll(owner->editor->findColour(ThemeColors::componentBackground));
    else
        g.fillAll(owner->editor->findColour(ThemeColors::componentBackground).darker(0.25f));

    if (rowIsSelected)
    {
        g.setColour(Colours::yellow);
        g.drawRect(0, 0, width, height, 1);
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
        g.drawText(String(streams[rowNumber]->getStreamSourceNodeId()), 4, 0, width, height, Justification::centredLeft);
    }
    else if (columnId == StreamTableModel::Columns::NAME)
    {
        g.setFont(12);
        g.setColour(owner->editor->findColour(ThemeColors::defaultText));
        g.drawText(String(streams[rowNumber]->getStreamName()), 4, 0, width, height, Justification::centredLeft);
    }
}


CustomTableLookAndFeel::CustomTableLookAndFeel()
{
    // Set the colors for the scrollbars
    setColour(ScrollBar::thumbColourId, Colours::grey); // Color of the scrollbar thumb
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
    streamTable->setBounds(2, 18, 231, 80);

    customTableLookAndFeel = std::make_unique<CustomTableLookAndFeel>();
    setLookAndFeel(customTableLookAndFeel.get());

    expanderButton = std::make_unique<ExpanderButton>();
    //addAndMakeVisible(expanderButton.get());
    expanderButton->setBounds(222, 4, 15, 15);
    expanderButton->addListener(this);

    shadowGradient = std::make_unique<ShadowGradient>();
    addAndMakeVisible(shadowGradient.get());
    shadowGradient->setBounds(2, 18, 231, 80);
    shadowGradient->setInterceptsMouseClicks(false, false);

}

TableListBox* StreamSelectorTable::createTableView(bool expanded)
{
    TableListBox* table = new TableListBox("Stream Table", tableModel.get());
    
    table->setHeader(std::make_unique<TableHeaderComponent>());

    table->getHeader().addColumn(" ", StreamTableModel::Columns::SELECTED, 12, 12, 12, TableHeaderComponent::notResizableOrSortable);
    table->getHeader().addColumn("Name", StreamTableModel::Columns::NAME, 94, 94, 94, TableHeaderComponent::notResizableOrSortable);
    table->getHeader().addColumn("DELAY", StreamTableModel::Columns::DELAY, 50, 50, 50, TableHeaderComponent::notResizableOrSortable);
    table->getHeader().addColumn("TTL", StreamTableModel::Columns::TTL_LINE_STATES, 60, 60, 60, TableHeaderComponent::notResizableOrSortable);

    if (expanded)
    {
        table->getHeader().addColumn("ID", StreamTableModel::Columns::PROCESSOR_ID, 30, 30, 30, TableHeaderComponent::notResizableOrSortable);
        table->getHeader().addColumn("# CH", StreamTableModel::Columns::NUM_CHANNELS, 20, 20, 20, TableHeaderComponent::notResizableOrSortable);
        table->getHeader().addColumn("Hz", StreamTableModel::Columns::SAMPLE_RATE, 40, 40, 40, TableHeaderComponent::notResizableOrSortable);
        table->getHeader().addColumn("ENABLE", StreamTableModel::Columns::ENABLED, 15, 15, 15, TableHeaderComponent::notResizableOrSortable);
    }

    table->getHeader().setColour(TableHeaderComponent::ColourIds::backgroundColourId, Colour(240, 240, 240));
    table->getHeader().setColour(TableHeaderComponent::ColourIds::highlightColourId, Colour(240, 240, 240));
    table->getHeader().setColour(TableHeaderComponent::ColourIds::textColourId, Colour(20, 20, 20));

    if (expanded)
        table->setHeaderHeight(20);
    else
        table->setHeaderHeight(0);

    table->setRowHeight(20);
    table->setMultipleSelectionEnabled(false);

    return table;
}


StreamSelectorTable::~StreamSelectorTable()
{
    setLookAndFeel(nullptr);
}

void StreamSelectorTable::buttonClicked(Button* button)
{
    if (button == expanderButton.get())
    {
        LOGD("EXPANDER BUTTON CLICKED ");

        auto* table = createTableView(true);
        table->setBounds(0, 0, 356, streams.size() * 20 + 20);
        table->selectRow(viewedStreamIndex);
        tableModel->table = table;
        table->setLookAndFeel(customTableLookAndFeel.get());

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

    Array<StreamInfoView*> newStreams;

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

StreamInfoView* StreamSelectorTable::getStreamInfoView(const DataStream* streamToCheck)
{

    for (auto stream : streams)
    {
        if (stream->streamId == streamToCheck->getStreamId())
            return stream;
    }

    return nullptr;
}

TTLMonitor* StreamSelectorTable::getTTLMonitor(const DataStream* stream)
{
    StreamInfoView* siv = getStreamInfoView(stream);

    if (siv != nullptr)
        return siv->getTTLMonitor();
    else
        return nullptr;

    return nullptr;
}

DelayMonitor* StreamSelectorTable::getDelayMonitor(const DataStream* stream)
{
    StreamInfoView* siv = getStreamInfoView(stream);

    if (siv != nullptr)
        return siv->getDelayMonitor();
    else
        return nullptr;

    return nullptr;
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
        TTLMonitor* ttlMonitor = stream->getTTLMonitor();

        if (ttlMonitor != nullptr)
            stream->getTTLMonitor()->repaint(); // postCommandMessage(0);
    }

    counter++;

    if (counter == 10)
    {
        counter = 0;

        for (auto stream : streams)
        {
            DelayMonitor* delayMonitor = stream->getDelayMonitor();

            if (delayMonitor != nullptr)
                delayMonitor->repaint(); // postCommandMessage(0);

            //LOGD(delayMonitor);
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
    LOGD("RESIZING TABLE");
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

    
    g.setColour(editor->findColour(ThemeColors::widgetBackground));
    g.fillRect(0, 20, getWidth()-5, getHeight() - 21);
    g.fillRoundedRectangle(0, 0, getWidth() - 5, getHeight() - 40, 5.0f);
    g.setColour(editor->findColour(ThemeColors::defaultText));
    g.setFont(12);
    g.drawText(" SELECT DATA STREAM: ", Rectangle<float>(150.0f, 20.0f), Justification::left);
   
}


const DataStream* StreamSelectorTable::getCurrentStream()
{
    if (streams.size() > 0)
        return streams[viewedStreamIndex]->getStream();
    else
        return nullptr;

}


void StreamSelectorTable::add(const DataStream* stream)
{

    streams.add(new StreamInfoView(stream, editor, true));

}

void StreamSelectorTable::beginUpdate()
{
    streams.clear();


}

uint16 StreamSelectorTable::finishedUpdate()
{

    Array<StreamInfoView*> newStreams;

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
            return streams[viewedStreamIndex]->getStream()->getStreamId();
        }
        else
        {
            viewedStreamIndex = streams.size() - 1;
            streamTable->selectRow(viewedStreamIndex);
            return streams[viewedStreamIndex]->getStream()->getStreamId();
        }

    }

}

void StreamSelectorTable::remove(StreamInfoView* stream)
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
        g.setColour(Colours::white);
    else
        g.setColour(Colours::lightgrey);

    g.strokePath(iconPath, PathStrokeType(1.5f));
}


void ShadowGradient::paint(Graphics& g)
{
    g.setGradientFill(ColourGradient(
      Colours::black.withAlpha(0.5f), 0.0f, 0.0f,
       Colours::black.withAlpha(0.0f), 0.0f, 10.0f, false));
   g.fillRect(0, 0, getWidth(), getHeight());
}


StreamScrollButton::StreamScrollButton(const String& name) :
    Button(name), isEnabled(true)
{
}

void StreamScrollButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (isEnabled)
        g.setColour(Colours::black);
    else
        g.setColour(Colours::darkgrey);

    g.drawText(getName(), 0, 0, getWidth(), getHeight(), Justification::centred, true);
}

StreamNameButton::StreamNameButton(const String& name) :
    Button(name), isEnabled(true)
{
}

void StreamNameButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (isEnabled)
        g.setColour(Colours::black);
    else
        g.setColour(Colours::darkgrey);

    g.setFont(Font("Fira Sans", "SemiBold", 14.0f));
    g.drawText(getName(), 0, 0, getWidth(), getHeight(), Justification::centred, true);
}

