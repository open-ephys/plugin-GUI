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


#include "SignalGeneratorEditor.h"
#include "../SignalGenerator.h"
#include <stdio.h>


SignalGeneratorEditor::SignalGeneratorEditor (GenericProcessor* parentNode) 
	: GenericEditor(parentNode), amplitudeSlider(0), frequencySlider(0)

{
	desiredWidth = 250;

	amplitudeSlider = new Slider (T("Amplitude Slider"));
	amplitudeSlider->setBounds(25,40,200,40);
	amplitudeSlider->setRange(0.005,0.05,0.005);
	amplitudeSlider->addListener(this);
	addAndMakeVisible(amplitudeSlider);

	frequencySlider = new Slider (T("High-Cut Slider"));
	frequencySlider->setBounds(25,70,200,40);
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