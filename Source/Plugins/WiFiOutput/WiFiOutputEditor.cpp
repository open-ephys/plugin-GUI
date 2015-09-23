/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#include "WiFiOutputEditor.h"
#include <stdio.h>


WiFiOutputEditor::WiFiOutputEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{

    accumulator = 0;

    desiredWidth = 150;

    Image im;
    im = ImageCache::getFromMemory(BinaryData::wifi_png,
                                   BinaryData::wifi_pngSize);

    icon = new ImageIcon(im);
    addAndMakeVisible(icon);
    icon->setBounds(35,35,80,80);

    icon->setOpacity(0.3f);

}

WiFiOutputEditor::~WiFiOutputEditor()
{
    deleteAllChildren();
}

void WiFiOutputEditor::receivedEvent()
{

    icon->setOpacity(0.8f);
    startTimer(50);

}

void WiFiOutputEditor::timerCallback()
{

    repaint();

    accumulator++;

    if (isFading)
    {

        if (accumulator > 15.0)
        {
            stopTimer();
            isFading = false;
        }

    }
    else
    {

        if (accumulator < 10.0)
        {
            icon->setOpacity(0.8f-(0.05*float(accumulator)));
            accumulator++;
        }
        else
        {
            icon->setOpacity(0.3f);
            stopTimer();
            accumulator = 0;
        }
    }
}