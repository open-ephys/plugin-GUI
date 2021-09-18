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

#include "../Settings/DataStream.h"

StreamInfoView::StreamInfoView(const DataStream* stream_, GenericEditor* editor_) :
    isEnabled(true), stream(stream_), editor(editor_)
{

    enableButton = std::make_unique<UtilityButton>("x", Font("Small Text", 10, Font::plain));
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

StreamSelector::StreamSelector() :
    streamInfoViewWidth(120),
    streamInfoViewHeight(80)
{

    viewport = std::make_unique<Viewport>();
    addAndMakeVisible(viewport.get());

    leftScrollButton = std::make_unique<UtilityButton>("<", Font("Small Text", 10, Font::plain));
    leftScrollButton->addListener(this);
    leftScrollButton->setCorners(true, false, true, false);
    leftScrollButton->setClickingTogglesState(false);
    addAndMakeVisible(leftScrollButton.get());

    rightScrollButton = std::make_unique<UtilityButton>(">", Font("Small Text", 10, Font::plain));
    rightScrollButton->addListener(this);
    rightScrollButton->setClickingTogglesState(false);
    rightScrollButton->setCorners(false, true, false, true);
    addAndMakeVisible(rightScrollButton.get());

    streamSelectorButton = std::make_unique<UtilityButton>("Stream",
        Font("FiraSans", 10, Font::plain));
    streamSelectorButton->setCorners(false, false, false, false);
    streamSelectorButton->addListener(this);
    streamSelectorButton->setClickingTogglesState(false);
    addAndMakeVisible(streamSelectorButton.get());

}


StreamSelector::~StreamSelector()
{

}

void StreamSelector::resized()
{
    viewport->setBounds(5, 20, getWidth() - 10, getHeight() - 20);
    
    leftScrollButton->setBounds(2, 2, 18, 18);
    rightScrollButton->setBounds(getWidth() - 20, 2, 18, 18);
    streamSelectorButton->setBounds(20, 2, getWidth() - 40, 18);

    std::cout << "StreamSelector resized, num streams: " << streams.size() << std::endl;

    for (int i = 0; i < streams.size(); i++)
    {
        streams[i]->setBounds(i * streamInfoViewWidth, 0, streamInfoViewWidth, streamInfoViewHeight);
    }
}

void StreamSelector::buttonClicked(Button* button)
{
    if (button == leftScrollButton.get())
    {
        std::cout << "Scroll left" << std::endl;
    }
    else if (button == rightScrollButton.get())
    {
        std::cout << "Scroll right" << std::endl;
    }
    else if (button == streamSelectorButton.get())
    {
        std::cout << "Select stream" << std::endl;
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

void StreamSelector::clear()
{
    streams.clear();
}

void StreamSelector::add(StreamInfoView* stream)
{
    streams.add(stream);
    viewport->addAndMakeVisible(stream);

    resized();
}

void StreamSelector::remove(StreamInfoView* stream)
{
    if (streams.contains(stream))
        streams.remove(streams.indexOf(stream));

    removeChildComponent(stream);

    resized();
}

