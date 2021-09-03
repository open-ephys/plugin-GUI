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

#include "StreamSelectorButton.h"

#include "../Settings/DataStream.h"

StreamSelectorButton::StreamSelectorButton(const DataStream* stream_) 
    : Button(stream_->getName())
    , isEnabled(true)
    , isSelected(true)
    , stream(stream_)
{

    setClickingTogglesState(true);
    setToggleState(true, false);

    infoString = stream->getName()
        + "\n"
        + "Channels: " + String(stream->getChannelCount())
        ;

    selectedGrad = ColourGradient(Colour(240, 179, 12), 0.0, 0.0,
        Colour(207, 160, 33), 0.0, 20.0f,
        false);
    selectedOverGrad = ColourGradient(Colour(209, 162, 33), 0.0, 5.0f,
        Colour(190, 150, 25), 0.0, 0.0f,
        false);
    neutralGrad = ColourGradient(Colour(220, 220, 220), 0.0, 0.0,
        Colour(170, 170, 170), 0.0, 20.0f,
        false);
    neutralOverGrad = ColourGradient(Colour(180, 180, 180), 0.0, 5.0f,
        Colour(150, 150, 150), 0.0, 0.0,
        false);
}

StreamSelectorButton::~StreamSelectorButton() {}

uint16 StreamSelectorButton::getStreamId() const
{
    return stream->getStreamId();
}

bool StreamSelectorButton::getSelectedState() const
{
    return isSelected;
}

void StreamSelectorButton::setEnabled(bool state)
{
    isEnabled = state;

    repaint();
}


void StreamSelectorButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{

    if (isEnabled)
        g.setColour(Colours::black);
    else
        g.setColour(Colours::grey);


    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 4.0);


    if (getToggleState())
    {
        if (isMouseOver && isEnabled)
            g.setGradientFill(selectedOverGrad);
        else
            g.setGradientFill(selectedGrad);
    }
    else
    {
        if (isMouseOver && isEnabled)
            g.setGradientFill(neutralOverGrad);
        else
            g.setGradientFill(neutralGrad);
    }

    g.fillRoundedRectangle(1, 1, getWidth()-2, getHeight()-2, 3.0);

    if (isEnabled)
        g.setColour(Colours::black);
    else
        g.setColour(Colours::grey);

    g.drawMultiLineText(infoString, 5, 15, getWidth() - 5, Justification::left);

}

StreamButtonHolder::StreamButtonHolder() 
{

}


StreamButtonHolder::~StreamButtonHolder()
{

}

void StreamButtonHolder::clear()
{
    buttons.clear();
}

void StreamButtonHolder::add(StreamSelectorButton* button)
{
    buttons.add(button);
    addAndMakeVisible(button);

    setBounds(getX(), getY(), getWidth(), buttonHeight * buttons.size() + buttonSpacing * buttons.size());
}

void StreamButtonHolder::remove(StreamSelectorButton* button)
{
    if (buttons.contains(button))
        buttons.remove(buttons.indexOf(button));

    removeChildComponent(button);

    setBounds(getX(), getY(), getWidth(), buttonHeight * buttons.size() + buttonSpacing * buttons.size());
}

void StreamButtonHolder::resized()
{
    std::cout << "StreamButtonHolder resized, num buttons: " << buttons.size() << std::endl;

    for (int i = 0; i < buttons.size(); i++)
    {
        std::cout << "Settings bounds for button " << i + 1 << ": " <<
            0 << " " << buttonHeight * i << " " << getWidth() << " " << buttonHeight << std::endl;
        buttons[i]->setBounds(0, buttonHeight * i + buttonSpacing * i, getWidth(), buttonHeight);
    }
}