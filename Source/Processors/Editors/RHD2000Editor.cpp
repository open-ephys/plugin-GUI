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

#include "RHD2000Editor.h"

RHD2000Editor::RHD2000Editor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)
{
	desiredWidth = 400;

	for (int i = 0; i < 4; i++)
	{
		HeadstageOptionsInterface* hsOptions = new HeadstageOptionsInterface(i);
		headstageOptionsInterfaces.add(hsOptions);

		addAndMakeVisible(hsOptions);
	}

}

RHD2000Editor::~RHD2000Editor()
{
    
}

void RHD2000Editor::resized()
{

	int width = getWidth()/4 - 20;

	for (int i = 0; i < headstageOptionsInterfaces.size(); i++)
	{
		headstageOptionsInterfaces[i]->setBounds(10+i*width,30, width,getHeight()-50);
	}
}

// --------------------------------------------------------------------

HeadstageOptionsInterface::HeadstageOptionsInterface(int hsNum) :
	hsNumber(hsNum)
{

	switch (hsNumber)
	{
		case 0 :
			name = "A";
			break;
		case 1:
			name = "B";
			break;
		case 2:
			name = "C";
			break;
		case 3:
			name = "D";
			break;
		default:
			name = "X";
	}

}

HeadstageOptionsInterface::~HeadstageOptionsInterface()
{

}

void HeadstageOptionsInterface::paint(Graphics& g)
{
	g.setColour(Colours::lightgrey);

	g.fillRoundedRectangle(5,0,getWidth()-10,getHeight(),4.0f);

}