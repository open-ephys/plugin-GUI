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
#include "../../UI/EditorViewport.h"

#include "../DataThreads/RHD2000Thread.h"

RHD2000Editor::RHD2000Editor(GenericProcessor* parentNode,
                             RHD2000Thread* board_,
                             bool useDefaultParameterEditors=true
                            )
    : GenericEditor(parentNode, useDefaultParameterEditors), board(board_)
{
    desiredWidth = 400;

    int width = desiredWidth/4 - 10;

    for (int i = 0; i < 4; i++)
    {
        HeadstageOptionsInterface* hsOptions = new HeadstageOptionsInterface(board, this, i);
        headstageOptionsInterfaces.add(hsOptions);

        addAndMakeVisible(hsOptions);

        hsOptions->setBounds(8+i*width,30, width, 85);
    }

}

RHD2000Editor::~RHD2000Editor()
{

}


// --------------------------------------------------------------------

HeadstageOptionsInterface::HeadstageOptionsInterface(RHD2000Thread* board_,
                                                     RHD2000Editor* editor_,
                                                     int hsNum) :
    board(board_), editor(editor_), hsNumber(hsNum), isEnabled(false)
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

    isEnabled = board->isHeadstageEnabled(hsNumber);

    enabledButton = new UtilityButton("enabled", Font("Small Text", 13, Font::plain));
    enabledButton->addListener(this);
    enabledButton->setRadius(3.0f);
    enabledButton->setBounds(10,30,70,25);
    addAndMakeVisible(enabledButton);



}

HeadstageOptionsInterface::~HeadstageOptionsInterface()
{

}

void HeadstageOptionsInterface::buttonClicked(Button* button)
{

    if (!(editor->acquisitionIsActive))
    {

        //std::cout << "Acquisition is not active" << std::endl;
        if (isEnabled)
        {
            isEnabled = false;
        }
        else
        {
            isEnabled = true;
        }

        board->enableHeadstage(hsNumber, isEnabled);

        repaint();

        editor->getEditorViewport()->makeEditorVisible(editor, false, true);
    }

}

// void HeadstageOptionsInterface::mouseUp(const MouseEvent& event)
// {
// 	///>>>> ???? WHY ISN"T THIS WORKING?

// 	if (event.eventComponent == this)
// 	{


// 	}

// }

void HeadstageOptionsInterface::paint(Graphics& g)
{
    g.setColour(Colours::lightgrey);

    g.fillRoundedRectangle(5,0,getWidth()-10,getHeight(),4.0f);

    if (isEnabled)
        g.setColour(Colours::black);
    else
        g.setColour(Colours::grey);

    g.setFont(Font("Small Text",20,Font::plain));

    g.drawText(name, 8, 5, 200, 15, Justification::left, false);

}