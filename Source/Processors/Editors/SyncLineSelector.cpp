/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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
#include "../../UI/LookAndFeel/CustomLookAndFeel.h"
#include <string>
#include <vector>

SyncChannelButton::SyncChannelButton (int _id, SyncLineSelector* _parent)
    : Button (String (_id)), id (_id), parent (_parent)
{
    btnColour = parent->lineColours[(id - 1) % parent->lineColours.size()];
}

SyncChannelButton::~SyncChannelButton() {}

void SyncChannelButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    g.setColour (findColour (ThemeColours::outline));
    g.fillRoundedRectangle (0.0f, 0.0f, getWidth(), getHeight(), 0.001 * getWidth());

    if (isMouseOver)
    {
        if (getToggleState())
            g.setColour (btnColour.brighter());
        else
            g.setColour (findColour (ThemeColours::widgetBackground).contrasting (0.3f));
    }
    else
    {
        if (getToggleState())
            g.setColour (btnColour);
        else
            g.setColour (findColour (ThemeColours::widgetBackground));
    }
    g.fillRoundedRectangle (1, 1, getWidth() - 2, getHeight() - 2, 0.001 * getWidth());

    //Draw text string in middle of button
    if (getToggleState())
        g.setColour (btnColour.contrasting());
    else
        g.setColour (findColour (ThemeColours::defaultText));

    g.setFont (FontOptions ("Inter", "Regular", 10.0f));
    g.drawText (String (id), 0, 0, getWidth(), getHeight(), Justification::centred);
}

SetPrimaryButton::SetPrimaryButton (const String& name) : Button (name)
{
}

SetPrimaryButton::~SetPrimaryButton() {}

void SetPrimaryButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (isMouseOver)
    {
        if (getToggleState())
            g.setColour (findColour (ThemeColours::highlightedFill).withAlpha (0.5f));
        else
            g.setColour (findColour (ThemeColours::widgetBackground).contrasting (0.3f));
    }
    else
    {
        if (getToggleState())
            g.setColour (findColour (ThemeColours::highlightedFill));
        else
            g.setColour (findColour (ThemeColours::widgetBackground));
    }
    g.fillRoundedRectangle (1.0f, 1.0f, (float) getWidth() - 2.0f, (float) getHeight() - 2.0f, 2.0f);

    g.setColour (findColour (ThemeColours::outline));
    g.drawRoundedRectangle (0.0f, 0.0f, (float) getWidth(), (float) getHeight(), 2.0f, 1.0f);

    g.setColour (findColour (ThemeColours::defaultText));
    g.setFont (FontOptions ("Inter", "Regular", 12.0f));
    g.drawText (String (getName()), 0, 0, getWidth(), getHeight(), Justification::centred);
}

//SyncLineSelector::SyncLineSelector(int nChans, int selectedIdx, bool isPrimary_)
SyncLineSelector::SyncLineSelector (Component* parent, SyncLineSelector::Listener* listener_, int numChans, int selectedLine_, bool isPrimary_, bool canSelectNone_)
    : PopupComponent (parent),
      listener (listener_),
      isPrimary (isPrimary_),
      nChannels (numChans),
      detectedChange (false),
      selectedLine (selectedLine_),
      canSelectNone (canSelectNone_)
{
    lineColours.add (Colour (224, 185, 36));
    lineColours.add (Colour (243, 119, 33));
    lineColours.add (Colour (237, 37, 36));
    lineColours.add (Colour (217, 46, 171));
    lineColours.add (Colour (101, 31, 255));
    lineColours.add (Colour (48, 117, 255));
    lineColours.add (Colour (116, 227, 156));
    lineColours.add (Colour (82, 173, 0));

    width = 368; //can use any multiples of 16 here for dynamic resizing

    int nColumns = 16;
    nRows = nChannels / nColumns + (int) (! (nChannels % nColumns == 0));
    buttonSize = width / 16;
    height = buttonSize * nRows;

    for (int i = 0; i < nRows; i++)
    {
        for (int j = 0; j < nColumns; j++)
        {
            if (nColumns * i + j < nChannels)
            {
                int buttonIdx = nColumns * i + j;
                buttons.add (new SyncChannelButton (nColumns * i + j + 1, this));
                buttons.getLast()->setBounds (width / nColumns * j, height / nRows * i, buttonSize, buttonSize);
                buttons.getLast()->setToggleState ((buttonIdx == selectedLine ? true : false), NotificationType::dontSendNotification);
                buttons.getLast()->addListener (this);
                addChildAndSetID (buttons.getLast(), String (buttonIdx));
            }
        }
    }

    if (! isPrimary)
    {
        setPrimaryStreamButton = new SetPrimaryButton ("Set as main clock");
        setPrimaryStreamButton->setBounds (0, height, 0.5 * width, width / nColumns);
        setPrimaryStreamButton->addListener (this);
        addChildAndSetID (setPrimaryStreamButton, "SETPRIMARY");
    }
    else
    {
        height = buttonSize * (nRows - 1);
    }

    if (nChannels <= 8)
        width /= 2;

    setSize (width, height + buttonSize);
    setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
}

SyncLineSelector::~SyncLineSelector() {}

void SyncLineSelector::mouseMove (const MouseEvent& event) {}

void SyncLineSelector::mouseDown (const MouseEvent& event) {}

void SyncLineSelector::mouseUp (const MouseEvent& event) {}

void SyncLineSelector::buttonClicked (Button* button)
{
    if (button->getComponentID() == "SETPRIMARY")
    {
        setSize (width, buttonSize * nRows);
        height = buttonSize * (nRows);
        isPrimary = true;
        setPrimaryStreamButton->setVisible (false);
        listener->primaryStreamChanged();
    }
    else
    {
        bool sameButton = false;

        for (int i = 0; i < buttons.size(); i++)
        {
            if (buttons[i]->getToggleState() && buttons[i] == button)
                sameButton = true;

            buttons[i]->setToggleState (false, dontSendNotification);
        }

        if (canSelectNone && sameButton)
        {
            selectedLine = -1;
        }
        else
        {
            button->setToggleState (true, dontSendNotification);
            selectedLine = std::stoi (button->getComponentID().toStdString());
        }

        listener->selectedLineChanged (selectedLine);
        repaint();
    }

    detectedChange = true;
}

void SyncLineSelector::updatePopup()
{
    selectedLine = listener->getSelectedLine();
    for (int i = 0; i < buttons.size(); i++)
    {
        if (buttons[i]->getId() - 1 == selectedLine)
            buttons[i]->setToggleState (true, NotificationType::dontSendNotification);
        else
            buttons[i]->setToggleState (false, NotificationType::dontSendNotification);
    }
}
