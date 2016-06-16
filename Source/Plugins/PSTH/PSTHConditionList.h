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

#ifndef PSTHCONDITIONLIST_H_INCLUDED
#define PSTHCONDITIONLIST_H_INCLUDED

#include <EditorHeaders.h>
#include <VisualizerEditorHeaders.h>
#include "PSTHCommon.h"
#include "PSTHProcessor.h"
class PSTHCanvas;
class PSTHProcessor;

class ConditionList : public Component,
	public Button::Listener

{
public:

	ConditionList(PSTHProcessor* n, Viewport* p, PSTHCanvas* c);
	~ConditionList();

	/** Draws the ConditionList. */
	void paint(Graphics& g);
	void buttonClicked(Button* btn);
	void updateConditionButtons();

private:
	PSTHProcessor* processor;
	Viewport* viewport;
	PSTHCanvas* canvas;

	ScopedPointer<ColorButton> titleButton;
	ScopedPointer<ColorButton> allButton, noneButton;
	OwnedArray<ColorButton> conditionButtons;
	/** The main method for drawing the ProcessorList.*/
	//   void drawItems(Graphics& g);

	/** Called when a mouse click begins within the boundaries of the ProcessorList.*/
	//void mouseDown(const MouseEvent& e);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConditionList);

};

#endif