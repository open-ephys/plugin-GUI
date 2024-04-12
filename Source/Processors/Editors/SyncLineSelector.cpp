/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2022 Open Ephys

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

#include "SyncLineSelector.h"
#include <string>
#include <vector>

SyncChannelButton::SyncChannelButton(int _id, SyncLineSelector* _parent) 
    : Button(String(_id)), id(_id), parent(_parent) 
{
    btnColor = parent->lineColors[(id - 1) % parent->lineColors.size()];
}


SyncChannelButton::~SyncChannelButton() {}

void SyncChannelButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{

	g.setColour(Colour(0,0,0));
    g.fillRoundedRectangle(0.0f, 0.0f, getWidth(), getHeight(), 0.001*getWidth());

    if (isMouseOver)
	{
		if (getToggleState())
			g.setColour(btnColor.brighter());
		else
			g.setColour(Colour(210, 210, 210));
	}
	else 
	{
		if (getToggleState())
			g.setColour(btnColor);
		else
			g.setColour(Colour(110, 110, 110));
	}
	g.fillRoundedRectangle(1,1,getWidth()-2,getHeight()-2,0.001*getWidth());

    //Draw text string in middle of button
    if (getToggleState())
        g.setColour(btnColor.contrasting());
    else
        g.setColour(Colour(255,255,255));

	g.setFont(10);
	g.drawText (String(id), 0,0, getWidth(), getHeight(), Justification::centred); 

}

SetPrimaryButton::SetPrimaryButton(const String& name) : Button(name) 
{

}

SetPrimaryButton::~SetPrimaryButton() {}

void SetPrimaryButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
    g.setColour(Colour(0,0,0));
    g.fillRoundedRectangle (0.0f, 0.0f, getWidth(), getHeight(), 0.001*getWidth());

    if (isMouseOver)
    {
        g.setColour(Colour(220, 220, 220));
    }
    else
    {
        if (getToggleState())
            g.setColour(Colour(200, 200, 200));
        else
            g.setColour(Colour(110, 110, 110));
    }
    g.fillRoundedRectangle(0.0f, 0.0f, getWidth(), getHeight(), 0.01*getWidth());
    
	g.setColour(Colour(255,255,255));
	g.setFont(12);
	g.drawText (String(getName()), 0, 0, getWidth(), getHeight(), Justification::centred);
}

//SyncLineSelector::SyncLineSelector(int nChans, int selectedIdx, bool isPrimary_)
SyncLineSelector::SyncLineSelector(SyncLineSelector::Listener* listener_, int numChans, int selectedLine_, bool isPrimary_, bool canSelectNone_)
    : listener(listener_),
    isPrimary(isPrimary_),
    nChannels(numChans),
    detectedChange(false),
    selectedLine(selectedLine_),
    canSelectNone(canSelectNone_)
{
    lineColors.add(Colour(224, 185, 36));
    lineColors.add(Colour(243, 119, 33));
    lineColors.add(Colour(237, 37, 36));
    lineColors.add(Colour(217, 46, 171));
    lineColors.add(Colour(101, 31, 255));
    lineColors.add(Colour(48, 117, 255));
    lineColors.add(Colour(116, 227, 156));
    lineColors.add(Colour(82, 173, 0));
    
    width = 368; //can use any multiples of 16 here for dynamic resizing

    int nColumns = 16;
    nRows = nChannels / nColumns + (int)(!(nChannels % nColumns == 0));
    buttonSize = width / 16;
    height = buttonSize * nRows;

	for (int i = 0; i < nRows; i++)
	{
		for (int j = 0; j < nColumns; j++)
		{
            if (nColumns*i+j < nChannels)
            {
                int buttonIdx = nColumns*i+j;
                buttons.add(new SyncChannelButton(nColumns*i+j+1, this));
                buttons.getLast()->setBounds(width/nColumns*j, height/nRows*i, buttonSize, buttonSize);
                buttons.getLast()->setToggleState((buttonIdx == selectedLine ? true : false), NotificationType::dontSendNotification);
                buttons.getLast()->addListener(this);
                addChildAndSetID(buttons.getLast(), String(buttonIdx));
            }
			
		}
	}
    
    if (!isPrimary)
    {
        setPrimaryStreamButton = new SetPrimaryButton("Set as main clock");
        setPrimaryStreamButton->setBounds(0, height, 0.5*width, width / nColumns);
        setPrimaryStreamButton->addListener(this);
        addChildAndSetID(setPrimaryStreamButton,"SETPRIMARY");
    }
    else
    {
        height = buttonSize * (nRows - 1);
    }
    
    
    if (nChannels <= 8)
        width /= 2;

	setSize (width, height + buttonSize);
	setColour(ColourSelector::backgroundColourId, Colours::transparentBlack);

}

SyncLineSelector::~SyncLineSelector() {}

void SyncLineSelector::mouseMove(const MouseEvent &event) {}

void SyncLineSelector::mouseDown(const MouseEvent &event) {}

void SyncLineSelector::mouseUp(const MouseEvent &event) {}

void SyncLineSelector::buttonClicked(Button* button)
{

    if (button->getComponentID() == "SETPRIMARY")
    {
        setSize (width, buttonSize * nRows);
        height = buttonSize * (nRows);
        isPrimary = true;
        setPrimaryStreamButton->setVisible(false);
        listener->primaryStreamChanged();
    }
    else
    {
        bool sameButton = false;

        for (int i = 0; i < buttons.size(); i++)
        {
            if (buttons[i]->getToggleState() && buttons[i] == button)
                sameButton = true;

            buttons[i]->setToggleState(false, dontSendNotification);
        }

        if (canSelectNone && sameButton)
        {
            selectedLine = -1;
        }
        else
        {
            button->setToggleState(true, dontSendNotification);
            selectedLine = std::stoi(button->getComponentID().toStdString());
        }
        
        listener->selectedLineChanged(selectedLine);
        repaint();
    }
    
    detectedChange = true;
    
}
