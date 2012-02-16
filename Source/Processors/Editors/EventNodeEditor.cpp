/*
  ==============================================================================

    EventNodeEditor.cpp
    Created: 16 Feb 2012 11:12:43am
    Author:  jsiegle

  ==============================================================================
*/

#include "EventNodeEditor.h"
#include <stdio.h>


EventNodeEditor::EventNodeEditor (GenericProcessor* parentNode, FilterViewport* vp) 
	: GenericEditor(parentNode, vp)

{
	desiredWidth = 250;

	StringArray hzValues;
	hzValues.add("0.25");
	hzValues.add("0.5");
	hzValues.add("1");
	hzValues.add("2");

	createRadioButtons(35, 65, 160, hzValues, "Event frequency");

	for (int n = 0; n < getNumChildComponents(); n++)
	{
		Button* c = (Button*) getChildComponent(n);

		if (c->isVisible())
			c->addListener(this);

		c->setVisible(true);
	}

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

void EventNodeEditor::buttonClicked (Button* button)
{
	//std::cout << button->getRadioGroupId() << " " << button->getName() << std::endl;
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