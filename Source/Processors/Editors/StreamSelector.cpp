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

    delayMonitor = std::make_unique<DelayMonitor>();
    addAndMakeVisible(delayMonitor.get());

    ttlMonitor = std::make_unique<TTLMonitor>();
    addAndMakeVisible(ttlMonitor.get());
        
    

}

uint16 StreamInfoView::getStreamId() const
{
    return stream->getStreamId();
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
    if (enableButton != nullptr)
        enableButton->setBounds(6, 38, 12, 12);
    
    if (delayMonitor != nullptr)
    {
        delayMonitor->setBounds(88, 35, 60, 12);
        ttlMonitor->setBounds(10, 59, 120, 12);
    }
    
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
    viewport->setBounds(5, 20, getWidth() - 10, getHeight() - 20);

    viewedComponent->setBounds(0, 0, streams.size() * streamInfoViewWidth, getHeight() - 20);
    
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
        streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getName());

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

            streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getName());
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

            streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getName());
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

            streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getName());
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
        streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getName());
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

    g.setColour(Colour(25, 25, 25));
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
        
        streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getName());
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

    g.drawText(getName(), 0, 0, getWidth(), getHeight(), Justification::centred, true);
}

