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

#include "EditorViewportButtons.h"

EditorScrollButton::EditorScrollButton(int d)
    : DrawableButton("ESB", DrawableButton::ImageFitted)
{

    direction = d;

    Path p;

    if (direction == RIGHT)
    {
        p.addTriangle(0.0f, 0.0f, 0.0f, 20.0f, 18.0f, 10.0f);
    }
    else
    {
        p.addTriangle(0.0f, 10.0f, 18.0f, 20.0f, 18.0f, 0.0f);
    }

    inactive.setPath(p);
    inactive.setFill(Colours::black);
    inactive.setStrokeFill(Colours::grey);
    inactive.setStrokeThickness(1.0f);

    activeNormal.setPath(p);
    activeNormal.setFill(Colours::grey);
    activeNormal.setStrokeFill(Colours::grey);
    activeNormal.setStrokeThickness(1.0f);

    activeOver.setPath(p);
    activeOver.setFill(Colours::grey);
    activeOver.setStrokeFill(Colours::grey);
    activeOver.setStrokeThickness(3.0f);

    activeDown.setPath(p);
    activeDown.setFill(Colours::white);
    activeDown.setStrokeFill(Colours::white);
    activeDown.setStrokeThickness(3.0f);

    setImages(&inactive, &inactive, &inactive);
    //  setBackgroundColours(Colours::black, Colours::black);
    setClickingTogglesState(false);

}

EditorScrollButton::~EditorScrollButton()
{
}


void EditorScrollButton::setActive(bool state)
{

    isActive = state;

    if (state == true)
    {
        setImages(&activeNormal, &activeOver, &activeDown);
    }
    else
    {
        setImages(&inactive, &inactive, &inactive);
    }

}

SignalChainScrollButton::SignalChainScrollButton(int d)
    : DrawableButton("SCSB", DrawableButton::ImageFitted)
{

    direction = d;

    DrawablePath normal;

    Path p;

    if (direction == DOWN)
    {
        p.addTriangle(0.0f, 0.0f, 9.0f, 20.0f, 18.0f, 0.0f);
    }
    else
    {
        p.addTriangle(0.0f, 20.0f, 9.0f, 0.0f, 18.0f, 20.0f);
    }

    inactive.setPath(p);
    inactive.setFill(Colours::black);
    inactive.setStrokeFill(Colours::grey);
    inactive.setStrokeThickness(1.0f);

    activeNormal.setPath(p);
    activeNormal.setFill(Colours::grey);
    activeNormal.setStrokeFill(Colours::grey);
    activeNormal.setStrokeThickness(1.0f);

    activeOver.setPath(p);
    activeOver.setFill(Colours::grey);
    activeOver.setStrokeFill(Colours::grey);
    activeOver.setStrokeThickness(3.0f);

    activeDown.setPath(p);
    activeDown.setFill(Colours::white);
    activeDown.setStrokeFill(Colours::white);
    activeDown.setStrokeThickness(3.0f);

    setImages(&inactive, &inactive, &inactive);
    //setBackgroundColours(Colours::black, Colours::black);
    setClickingTogglesState(false);

}

SignalChainScrollButton::~SignalChainScrollButton()
{
}

void SignalChainScrollButton::setActive(bool state)
{

    isActive = state;

    if (state == true)
    {
        setImages(&activeNormal, &activeOver, &activeDown);
    }
    else
    {
        setImages(&inactive, &inactive, &inactive);
    }

}
