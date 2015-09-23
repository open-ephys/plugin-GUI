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

#include "EventNodeEditor.h"
#include "../Editors/ChannelSelector.h"
#include <stdio.h>


EventNodeEditor::EventNodeEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
    desiredWidth = 200;



    //createRadioButtons(35, 65, 160, hzValues, "Event frequency");

    // for (int n = 0; n < getNumChildComponents(); n++)
    // {
    // 	Button* c = (Button*) getChildComponent(n);

    // 	if (c->isVisible())
    // 		c->addListener(this);

    // 	c->setVisible(true);
    // }

    channelSelector->eventsOnly = true;

}

EventNodeEditor::~EventNodeEditor()
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

void EventNodeEditor::buttonEvent(Button* button)
{
    std::cout << button->getRadioGroupId() << " " << button->getName() << std::endl;

    String value = button->getName();
    //float val;

    getAudioProcessor()->setParameter(0,value.getFloatValue());

    // if (value.getLastCharacter() == juce_wchar('k')) {
    // 	val = value.dropLastCharacters(1).getFloatValue() * 1000.0f;
    // }
    // else {
    // 	val = value.getFloatValue();
    // }

    //if (button->getRadioGroupId() == 1)
    ///	//getAudioProcessor()->setParameter(0,val);
    //else
    //getAudioProcessor()->setParameter(1,val);

    //std::cout << button->getRadioGroupId() << " " << val << std::endl;

}