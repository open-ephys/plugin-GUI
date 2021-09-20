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

StreamInfoView::StreamInfoView(const DataStream* stream_, GenericEditor* editor_) :
    isEnabled(true), stream(stream_), editor(editor_)
{

    enableButton = std::make_unique<StreamEnableButton>("x");
    enableButton->addListener(this);
    enableButton->setClickingTogglesState(true);
    enableButton->setToggleState(true, false);
    addAndMakeVisible(enableButton.get());

    infoString = "Source: " + stream->getSourceNodeName()
        + "\n"
        + String(stream->getChannelCount()) + " channels @ " +
        String(stream->getSampleRate()) + " Hz";

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

bool StreamInfoView::getEnabledState() const
{
    return isEnabled;
}

void StreamInfoView::setEnabled(bool state)
{
    isEnabled = state;

    repaint();
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

void StreamInfoView::buttonClicked(Button* button)
{
    if (button == enableButton.get())
    {
        setEnabled(button->getToggleState());
        
        if (button->getToggleState())
            button->setButtonText("x");
        else
            button->setButtonText(" ");
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

void StreamSelector::resized()
{
    viewport->setBounds(5, 20, getWidth() - 10, getHeight() - 20);

    streamInfoViewWidth = getWidth() - 10;

    viewedComponent->setBounds(0, 0, streams.size() * streamInfoViewWidth, getHeight() - 20);
    
    leftScrollButton->setBounds(2, 2, 18, 18);
    rightScrollButton->setBounds(getWidth() - 20, 2, 18, 18);
    streamSelectorButton->setBounds(20, 2, getWidth() - 40, 18);

    std::cout << "StreamSelector resized, num streams: " << streams.size() << std::endl;

    for (int i = 0; i < streams.size(); i++)
    {
        streams[i]->setBounds(i * streamInfoViewWidth, 0, streamInfoViewWidth, streamInfoViewHeight);
    }

    viewport->setViewPosition(scrollOffset.getCurrentValue(), 0);
}

void StreamSelector::buttonClicked(Button* button)
{
    int currentlyViewedStream = viewedStreamIndex;

    if (button == leftScrollButton.get())
    {
        std::cout << "Scroll left" << std::endl;

        if (viewedStreamIndex != 0)
        {
            viewedStreamIndex -= 1;
            scrollOffset.setTargetValue(viewedStreamIndex * streamInfoViewWidth);
            startTimer(50);

            std::cout << "  Target value: " << viewedStreamIndex * streamInfoViewWidth << std::endl;

            streamSelectorButton->setName(streams[viewedStreamIndex]->getStream()->getName());
            streamSelectorButton->repaint();
        }

    }
    else if (button == rightScrollButton.get())
    {
        std::cout << "Scroll right" << std::endl;

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

void StreamSelector::add(StreamInfoView* stream)
{
    streams.add(stream);
    viewedComponent->addAndMakeVisible(stream);

    if (streams.size() == 1)
        streamSelectorButton->setName(streams[0]->getStream()->getName());

    resized();
}

void StreamSelector::remove(StreamInfoView* stream)
{
    if (streams.contains(stream))
        streams.remove(streams.indexOf(stream));

    viewedComponent->removeChildComponent(stream);

    resized();
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

