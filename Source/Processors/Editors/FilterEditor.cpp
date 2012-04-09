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

#include "FilterEditor.h"
#include "../FilterNode.h"
#include <stdio.h>


FilterEditor::FilterEditor (GenericProcessor* parentNode) 
	: GenericEditor(parentNode)

{
	desiredWidth = 250;

	StringArray lowCutValues;
	lowCutValues.add("1");
	lowCutValues.add("10");
	lowCutValues.add("100");
	lowCutValues.add("500");

	StringArray highCutValues;
	highCutValues.add("1K");
	highCutValues.add("3K");
	highCutValues.add("6K");
	highCutValues.add("9K");

	createRadioButtons(35, 50, 160, lowCutValues, "Low Cutoff");
	createRadioButtons(35, 90, 160, highCutValues, "High Cutoff");

	// for (int n = 0; n < getNumChildComponents(); n++)
	// {
	// 	Button* c = (Button*) getChildComponent(n);

	// 	if (c->isVisible())
	// 		c->addListener(this);

	// 	if (c->getRadioGroupId() != 999)
	// 		c->setVisible(true);
	// }

}

FilterEditor::~FilterEditor()
{
	deleteAllChildren();
}

// void FilterEditor::sliderValueChanged (Slider* slider)
// {

// 	if (slider == lowSlider)
// 		getAudioProcessor()->setParameter(0,slider->getValue());
// 	else 
// 		getAudioProcessor()->setParameter(1,slider->getValue());

// }

void FilterEditor::buttonEvent (Button* button)
{
	//std::cout << button->getRadioGroupId() << " " << button->getName() << std::endl;
	
	//if (!checkDrawerButton(button) && !checkChannelSelectors(button)) {

		String value = button->getName();
		float val;

		val = value.getFloatValue();

		Array<int> chans = getActiveChannels();
		
		GenericProcessor* p = (GenericProcessor*) getAudioProcessor();

		for (int n = 0; n < chans.size(); n++) {
			
			p->setCurrentChannel(chans[n]);

			if (button->getRadioGroupId() == 1)
	 			getAudioProcessor()->setParameter(0,val);
	 		else 
	 			getAudioProcessor()->setParameter(1,val*1000.0f);

		}	

 	//std::cout << button->getRadioGroupId() << " " << val << std::endl;
 //	}

}