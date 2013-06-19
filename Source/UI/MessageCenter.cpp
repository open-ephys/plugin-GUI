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

#include "MessageCenter.h"

//---------------------------------------------------------------------

MessageCenter::MessageCenter() :
    messageBackground(Colours::grey.withAlpha(0.5f))
{

    messageDisplayArea = new Label("Message Display Area","No new messages.");

    addAndMakeVisible(messageDisplayArea);

}

MessageCenter::~MessageCenter()
{

}

void MessageCenter::paint(Graphics& g)
{

    g.setColour(Colour(58,58,58));

    g.fillRect(0, 0, getWidth(), getHeight());

    g.setColour(messageBackground);

    g.fillRect(5, 5, getWidth()-10, getHeight()-10);

}

void MessageCenter::resized()
{
    if (messageDisplayArea != 0)
        messageDisplayArea->setBounds(5,0,getWidth(),getHeight());

}

void MessageCenter::actionListenerCallback(const String& message)
{

    messageDisplayArea->setText(message, dontSendNotification);

    messageBackground = Colours::orange;

    repaint();

}