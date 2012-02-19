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