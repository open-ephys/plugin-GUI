/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

#include "LfpDisplayEditor.h"


LfpDisplayEditor::LfpDisplayEditor (GenericProcessor* parentNode) 
	: VisualizerEditor(parentNode)

{

	tabText = "LFP";

	desiredWidth = 250;

	StringArray timeBaseValues;
	timeBaseValues.add("1");
	timeBaseValues.add("2");
	timeBaseValues.add("5");
	timeBaseValues.add("10");

	//createRadioButtons(35, 50, 160, timeBaseValues, "Display width (s)");

	StringArray displayGainValues;
	displayGainValues.add("1");
	displayGainValues.add("2");
	displayGainValues.add("4");
	displayGainValues.add("8");

	//createRadioButtons(35, 90, 160, displayGainValues, "Display Gain");


}

LfpDisplayEditor::~LfpDisplayEditor()
{
}


Visualizer* LfpDisplayEditor::createNewCanvas()
{

	LfpDisplayNode* processor = (LfpDisplayNode*) getProcessor();
	return new LfpDisplayCanvas(processor);

}

void LfpDisplayEditor::buttonCallback(Button* button)
{

	int gId = button->getRadioGroupId();

	if (gId > 0) {
		if (canvas != 0)
		{
			canvas->setParameter(gId-1, button->getName().getFloatValue());
		}

	} 

}
