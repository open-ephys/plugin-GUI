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
    addAndMakeVisible(enableButton.get());

    infoString = stream->getName()
        + "\n"
        + "Channels: " + String(stream->getChannelCount())
        ;
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
    enableButton->setBounds(40, 40, 100, 20);
}

void StreamInfoView::paint(Graphics& g)
{

    if (isEnabled)
        g.setColour(Colours::black);
    else
        g.setColour(Colours::grey);

    g.drawMultiLineText(infoString, 5, 15, getWidth() - 5, Justification::left);

}

StreamSelector::StreamSelector() :
    streamInfoViewWidth(80),
    streamInfoViewHeight(80)
{

    viewport = std::make_unique<Viewport>();
    addAndMakeVisible(viewport.get());

    leftScrollButton = std::make_unique<UtilityButton>("<", Font("Small Text", 10, Font::plain));
    leftScrollButton->addListener(this);
    leftScrollButton->setClickingTogglesState(false);
    addAndMakeVisible(leftScrollButton.get());

    rightScrollButton = std::make_unique<UtilityButton>(">", Font("Small Text", 10, Font::plain));
    rightScrollButton->addListener(this);
    rightScrollButton->setClickingTogglesState(false);
    addAndMakeVisible(rightScrollButton.get());

    streamSelectorButton = std::make_unique<UtilityButton>("Stream", Font("Small Text", 10, Font::plain));
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
    g.setColour(Colours::darkgrey);
    g.drawRect(0, 0, getWidth(), getHeight(), 1);
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

