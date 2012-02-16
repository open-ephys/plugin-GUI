/*
  ==============================================================================

    FilterEditor.cpp
    Created: 12 Jun 2011 5:04:08pm
    Author:  jsiegle

  ==============================================================================
*/

#include "FilterEditor.h"
#include "../FilterNode.h"
#include <stdio.h>


FilterEditor::FilterEditor (GenericProcessor* parentNode, FilterViewport* vp) 
	: GenericEditor(parentNode, vp)

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

	for (int n = 0; n < getNumChildComponents(); n++)
	{
		Button* c = (Button*) getChildComponent(n);

		if (c->isVisible())
			c->addListener(this);

		c->setVisible(true);
	}

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

void FilterEditor::buttonClicked (Button* button)
{
	//std::cout << button->getRadioGroupId() << " " << button->getName() << std::endl;
	String value = button->getName();
	float val;

	if (value.getLastCharacter() == juce_wchar('k')) {
		val = value.dropLastCharacters(1).getFloatValue() * 1000.0f;
	}
	else {
		val = value.getFloatValue();
	}

	//if (button->getRadioGroupId() == 1)
 	///	//getAudioProcessor()->setParameter(0,val);
 	//else 
 		//getAudioProcessor()->setParameter(1,val);

 	//std::cout << button->getRadioGroupId() << " " << val << std::endl;

}