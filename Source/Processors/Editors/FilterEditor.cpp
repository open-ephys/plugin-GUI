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
	: GenericEditor(parentNode, vp), lowSlider(0), highSlider(0)

{
	desiredWidth = 250;

	lowSlider = new Slider (T("Low-Cut Slider"));
	lowSlider->setBounds(25,20,200,40);
	lowSlider->setRange(10,600,10);
	lowSlider->addListener(this);
	addAndMakeVisible(lowSlider);

	highSlider = new Slider (T("High-Cut Slider"));
	highSlider->setBounds(25,65,200,40);
	highSlider->setRange(1000,10000,500);
	highSlider->addListener(this);
	addAndMakeVisible(highSlider);

}

FilterEditor::~FilterEditor()
{
	deleteAllChildren();
}

void FilterEditor::sliderValueChanged (Slider* slider)
{

	if (slider == lowSlider)
		getAudioProcessor()->setParameter(0,slider->getValue());
	else 
		getAudioProcessor()->setParameter(1,slider->getValue());

}