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

#include "../Settings/DataStream.h"

StreamInfoView::StreamInfoView(const DataStream* stream_, GenericEditor* editor_, bool isEnabled_) :
    isEnabled(isEnabled_), stream(stream_), editor(editor_), streamIsStillNeeded(true)
{
    LOGD("Adding stream ", getStreamId(), " with ", stream->getChannelCount(), " channels ");

    updateInfoString();

    enableButton = std::make_unique<StreamEnableButton>("x");
    enableButton->addListener(this);
    enableButton->setClickingTogglesState(true);
    enableButton->setToggleState(true, false);
    addAndMakeVisible(enableButton.get());

    delayMonitor = std::make_unique<DelayMonitor>();
    addAndMakeVisible(delayMonitor.get());

    ttlMonitor = std::make_unique<TTLMonitor>();
    addAndMakeVisible(ttlMonitor.get());
}

StreamInfoView::~StreamInfoView() {}

uint16 StreamInfoView::getStreamId() const
{
    return stream->getStreamId();
}

void StreamInfoView::updateInfoString()
{
    infoString = "ID: " + String(getStreamId()) + " : " + stream->getSourceNodeName()
        + "\n"
        + String(stream->getChannelCount()) + " channels @ " +
        String(stream->getSampleRate()) + " Hz";

}

bool StreamInfoView::getEnabledState() const
{
    return isEnabled;
}

void StreamInfoView::setEnabled(bool state)
{
    isEnabled = state;

    if (isEnabled)
        enableButton->setButtonText("x");
    else
        enableButton->setButtonText(" ");

    enableButton->repaint();
}

void StreamInfoView::startAcquisition()
{
    delayMonitor->startAcquisition();
    ttlMonitor->startAcquisition();
}

void StreamInfoView::stopAcquisition()
{
    delayMonitor->stopAcquisition();
    ttlMonitor->stopAcquisition();
}

void StreamInfoView::beginUpdate()
{
    streamIsStillNeeded = false;
}

void StreamInfoView::update(const DataStream* newStream)
{
    streamIsStillNeeded = true;

    stream = newStream;

    updateInfoString();
}

void StreamInfoView::buttonClicked(Button* button)
{
    if (button == enableButton.get())
    {
        setEnabled(!isEnabled);
        enableButton->setToggleState(isEnabled, false);

        std::cout << "Button clicked --- Stream " << getStreamId() << " enabled: " << isEnabled << std::endl;

        editor->streamEnabledStateChanged(getStreamId(), isEnabled);
    }
}

void StreamInfoView::resized()
{
    enableButton->setBounds(3, 38, 15, 15);
    delayMonitor->setBounds(38, 58, 60, 15);
}

void StreamInfoView::paint(Graphics& g)
{

    if (isEnabled)
        g.setColour(Colours::black);
    else
        g.setColour(Colours::grey);

    g.setFont(12);
    g.drawMultiLineText(infoString, 5, 18, getWidth() - 5, Justification::left);
    g.drawText("Enabled", 22, 40, 60, 12, Justification::left);

    g.drawText("Delay:", 5, 60, 60, 12, Justification::left);

}

StreamSelector::StreamSelector(GenericEditor* ed_) :
    editor(ed_),
    streamInfoViewWidth(120),
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


StreamSelector::~StreamSelector()
{

}

StreamInfoView* StreamSelector::getStreamInfoView(const DataStream* streamToCheck)
{
    for (auto stream : streams)
    {
        if (stream->getStreamId() == streamToCheck->getStreamId())
            return stream;
    }

    return nullptr;
}

bool StreamSelector::checkStream(const DataStream* streamToCheck)
{
    //StreamInfoView* siv = getStreamInfoView(streamToCheck);

    std::map<uint16, bool>::iterator it = streamStates.begin();

    LOGD("STREAM STATES");
    while (it != streamStates.end())
    {
        // Accessing KEY from element pointed by it.
        uint16 streamId = it->first;
        // Accessing VALUE from element pointed by it.
        bool state = it->second;
        LOGD(streamId, " :: ", state);
        // Increment the Iterator to point to next entry
        it++;
    }

    if (streamStates.count(streamToCheck->getStreamId()) > 0)
    {
        LOGD(" Stream Selector returning ", streamStates[streamToCheck->getStreamId()]);
        return streamStates[streamToCheck->getStreamId()];
    }
        
    else
    {
        LOGD(" Stream not found, returning 1.");
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
    LOGD("Setting state for stream ", streamId, ":  ", isEnabled);
    streamStates[streamId] = isEnabled;
}

void StreamSelector::resized()
{
    viewport->setBounds(5, 20, getWidth() - 10, getHeight() - 20);

    streamInfoViewWidth = getWidth() - 10;

    viewedComponent->setBounds(0, 0, streams.size() * streamInfoViewWidth, getHeight() - 20);
    
    leftScrollButton->setBounds(2, 2, 18, 18);
    rightScrollButton->setBounds(getWidth() - 20, 2, 18, 18);
    streamSelectorButton->setBounds(20, 2, getWidth() - 40, 18);

    LOGD("StreamSelector resized, num streams: ", streams.size());
    LOGD(" Scroll offset: ", scrollOffset.getCurrentValue());

    for (int i = 0; i < streams.size(); i++)
    {
        streams[i]->setBounds(i * streamInfoViewWidth, 0, streamInfoViewWidth, streamInfoViewHeight);
    }

    if (streams.size() > 0)
        streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getName());

    viewport->setViewPosition(scrollOffset.getCurrentValue(), 0);
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

            std::cout << "  Target value: " << viewedStreamIndex * streamInfoViewWidth << std::endl;

            streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getName());
            streamSelectorButton->repaint();
        }
    }
    else if (button == streamSelectorButton.get())
    {
        std::cout << "Select stream" << std::endl;

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
        editor->updateSelectedStream(streams[viewedStreamIndex]->getStream()->getStreamId());
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

void StreamSelector::clear()
{
    streams.clear();
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

    LOGD("BEGIN UPDATE --- NUM STREAMS: ", streams.size());

    for (auto stream : streams)
    {
        stream->beginUpdate();
    }
}

void StreamSelector::finishedUpdate()
{

    LOGD("END UPDATE --- NUM STREAMS: ", streams.size());

    Array<StreamInfoView*> streamsToRemove;

    for (auto stream : streams)
    {
        LOGD("Checking viewer for stream ");

        if (!stream->streamIsStillNeeded)
        {
            streamsToRemove.add(stream);
        }
        else {
            LOGD(" STILL NEEDED.");
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

    resized();
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
    if (getToggleState())
        g.setColour(Colours::yellow);
    else
        g.setColour(Colours::darkgrey);

    g.drawText(getName(), 0, 0, getWidth(), getHeight(), Justification::centred, true);
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

