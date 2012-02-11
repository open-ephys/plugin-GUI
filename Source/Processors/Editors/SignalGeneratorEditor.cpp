/*
  ==============================================================================

    SignalGeneratorEditor.cpp
    Created: 10 Feb 2012 1:28:45pm
    Author:  jsiegle

  ==============================================================================
*/


#include "SignalGeneratorEditor.h"
#include "../SignalGenerator.h"
#include <stdio.h>


SignalGeneratorEditor::SignalGeneratorEditor (GenericProcessor* parentNode, FilterViewport* vp) 
	: GenericEditor(parentNode, vp), amplitudeSlider(0), frequencySlider(0)

{
	desiredWidth = 250;

	amplitudeSlider = new Slider (T("Amplitude Slider"));
	amplitudeSlider->setBounds(25,20,200,40);
	amplitudeSlider->setRange(0.005,0.05,0.005);
	amplitudeSlider->addListener(this);
	addAndMakeVisible(amplitudeSlider);

	frequencySlider = new Slider (T("High-Cut Slider"));
	frequencySlider->setBounds(25,65,200,40);
	frequencySlider->setRange(1,100,1);
	frequencySlider->addListener(this);
	addAndMakeVisible(frequencySlider);

}

SignalGeneratorEditor::~SignalGeneratorEditor()
{
	deleteAllChildren();
}

void SignalGeneratorEditor::sliderValueChanged (Slider* slider)
{

	if (slider == amplitudeSlider)
		getAudioProcessor()->setParameter(0,slider->getValue());
	else 
		getAudioProcessor()->setParameter(1,slider->getValue());

}