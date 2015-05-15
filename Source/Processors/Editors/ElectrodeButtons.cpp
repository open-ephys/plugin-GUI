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

#include "ElectrodeButtons.h"

ElectrodeButton::ElectrodeButton(int chan_) : Button("Electrode"), chan(chan_)
{
	setClickingTogglesState(true);
	//setRadioGroupId(299);
	setToggleState(true, dontSendNotification);
	setButtonText(String(chan_));
}
ElectrodeButton::~ElectrodeButton() {}

int ElectrodeButton::getChannelNum()
{
	return chan;
}

void ElectrodeButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour(Colours::orange);
    else
        g.setColour(Colours::darkgrey);

    if (isMouseOver)
        g.setColour(Colours::white);

    if (!isEnabled())
        g.setColour(Colours::black);

    g.fillRect(0,0,getWidth(),getHeight());

    // g.setFont(buttonFont);
    g.setColour(Colours::black);

    g.drawRect(0,0,getWidth(),getHeight(),1.0);

    if (!isEnabled())
    {
        g.setColour(Colours::grey);
    }

    if (chan < 100)
        g.setFont(10.f);
    else
        g.setFont(8.f);

    if (chan >= 0)
        g.drawText(getButtonText(),0,0,getWidth(),getHeight(),Justification::centred,true);
}

void ElectrodeButton::setChannelNum(int i)
{
    setChannelNum(i,true);
}

void ElectrodeButton::setChannelNum(int i, bool changeButtonText)
{
    chan = i;

    if (changeButtonText)
    {
        setButtonText(String(chan));
    }
}

ElectrodeEditorButton::ElectrodeEditorButton(const String& name_, Font font_) : Button("Electrode Editor"),
name(name_), font(font_)
{
	if (name.equalsIgnoreCase("edit") || name.equalsIgnoreCase("monitor"))
		setClickingTogglesState(true);
}
ElectrodeEditorButton::~ElectrodeEditorButton() {}

void ElectrodeEditorButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour(Colours::darkgrey);
    else
        g.setColour(Colours::lightgrey);

    g.setFont(font);

    g.drawText(name,0,0,getWidth(),getHeight(),Justification::left,true);
}

