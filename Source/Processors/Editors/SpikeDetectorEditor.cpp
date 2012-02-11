/*
  ==============================================================================

    SpikeDetectorEditor.cpp
    Created: 14 Aug 2011 6:27:19pm
    Author:  jsiegle

  ==============================================================================
*/


#include "SpikeDetectorEditor.h"
#include "../SpikeDetector.h"
#include <stdio.h>


SpikeDetectorEditor::SpikeDetectorEditor (GenericProcessor* parentNode, FilterViewport* vp) 
	: GenericEditor(parentNode, vp), threshSlider(0)

{
	desiredWidth = 200;

	threshSlider = new Slider (T("Threshold Slider"));
	threshSlider->setBounds(25,20,150,40);
	threshSlider->setRange(1000,20000,1000);
	threshSlider->addListener(this);
	addAndMakeVisible(threshSlider);

}

SpikeDetectorEditor::~SpikeDetectorEditor()
{

	deleteAllChildren();	

}

void SpikeDetectorEditor::sliderValueChanged (Slider* slider)
{

	if (slider == threshSlider)
		getAudioProcessor()->setParameter(0,slider->getValue());

}