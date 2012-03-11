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
#include "../Visualization/LfpDisplayCanvas.h"

SelectorButton::SelectorButton()
	: DrawableButton (T("Selector"), DrawableButton::ImageFitted)
{
		DrawablePath normal, over, down;

	    Path p;
        p.addRectangle (0.0,0.0,20.0,20.0);
        normal.setPath (p);
        normal.setFill (Colours::lightgrey);
        normal.setStrokeThickness (0.0f);

        over.setPath (p);
        over.setFill (Colours::black);
        over.setStrokeFill (Colours::black);
        over.setStrokeThickness (5.0f);

        setImages (&normal, &over, &over);
        setBackgroundColours(Colours::darkgrey, Colours::white);
        setClickingTogglesState (true);
        setTooltip ("Toggle a state.");

}

SelectorButton::~SelectorButton()
{
}


LfpDisplayEditor::LfpDisplayEditor (GenericProcessor* parentNode) 
	: GenericEditor(parentNode),
	  tabIndex(-1), dataWindow(0),
	  isPlaying(false),
	  canvas(0)

{

	desiredWidth = 250;

	StringArray timeBaseValues;
	timeBaseValues.add("1");
	timeBaseValues.add("2");
	timeBaseValues.add("5");
	timeBaseValues.add("10");

	createRadioButtons(35, 50, 160, timeBaseValues, "Display width (s)");

	StringArray displayGainValues;
	displayGainValues.add("1");
	displayGainValues.add("2");
	displayGainValues.add("4");
	displayGainValues.add("8");

	createRadioButtons(35, 90, 160, displayGainValues, "Display Gain");

	windowSelector = new SelectorButton();
	windowSelector->addListener(this);
	windowSelector->setBounds(190,10,10,10);

	windowSelector->setToggleState(false,false);
	addAndMakeVisible(windowSelector);

	tabSelector = new SelectorButton();
	tabSelector->addListener(this);
	tabSelector->setBounds(205,10,10,10);
	
	addAndMakeVisible(tabSelector);
	tabSelector->setToggleState(false,false);

}

LfpDisplayEditor::~LfpDisplayEditor()
{

	if (tabIndex > -1)
	{
		getDataViewport()->removeTab(tabIndex);
	}

	deleteAllChildren();

}

void LfpDisplayEditor::enable()
{
	std::cout << "   Enabling LfpDisplayEditor" << std::endl;
	if (canvas != 0)
		canvas->beginAnimation();
	
	isPlaying = true;
}

void LfpDisplayEditor::disable()
{
	if (canvas != 0)
		canvas->endAnimation();

	isPlaying = false;
}

void LfpDisplayEditor::updateVisualizer()
{

	if (canvas != 0)
		canvas->update();

}


void LfpDisplayEditor::setBuffers(AudioSampleBuffer* asb, MidiBuffer* mb)
{
	std::cout << "LfpDisplayEditor buffers are set!" << std::endl;
	//streamBuffer = asb;
	//eventBuffer = mb;

	//std::cout << streamBuffer << std::endl;
	//std::cout << eventBuffer << std::endl;
}

LfpDisplayCanvas* LfpDisplayEditor::createNewCanvas()
{


	LfpDisplayNode* processor = (LfpDisplayNode*) getProcessor();
	return new LfpDisplayCanvas(processor);

}

void LfpDisplayEditor::buttonEvent(Button* button)
{

	//if (!checkDrawerButton(button) && !checkChannelSelectors(button)) {

	int gId = button->getRadioGroupId();

	if (gId > 0) {
		if (canvas != 0)
		{
			canvas->setParameter(gId-1, button->getName().getFloatValue());
		}

	} else {

		if (canvas == 0) {
			
			canvas = createNewCanvas();

			if (isPlaying)
				canvas->beginAnimation();
		}

		if (button == windowSelector)
		{

			if (tabSelector->getToggleState() && windowSelector->getToggleState())
		 	{
		 		tabSelector->setToggleState(false, false);
		 		getDataViewport()->destroyTab(tabIndex);
		 		tabIndex = -1;
		 	}

		 	if (dataWindow == 0) {

				dataWindow = new DataWindow(windowSelector);
		 		dataWindow->setContentNonOwned(canvas, false);
		 		dataWindow->setVisible(true);
		 		canvas->refreshState();
				
		 	} else {

		 		dataWindow->setVisible(windowSelector->getToggleState());
		 		
		 		if (windowSelector->getToggleState())
		 		{
		 			dataWindow->setContentNonOwned(canvas, false);
		 			canvas->refreshState();
		 		} else {
		 			dataWindow->setContentNonOwned(0, false);
		 		}
		 		
		 	}

		} 
		else if (button == tabSelector) 
		{
			if (tabSelector->getToggleState() && tabIndex < 0)
			{

				 if (windowSelector->getToggleState())
				 {
				 	dataWindow->setContentNonOwned(0, false);
				 	windowSelector->setToggleState(false, false);
				 	dataWindow->setVisible(false);
				 }

				tabIndex = getDataViewport()->addTabToDataViewport("LFP",canvas);


			} else if (!tabSelector->getToggleState() && tabIndex > -1)
			{
				getDataViewport()->destroyTab(tabIndex);
				tabIndex = -1;

			}
		}
	}

}

void LfpDisplayEditor::sliderValueChanged (Slider* slider)
{

	// if (canvas != 0)
	// {
	// 	if (slider == timebaseSlider)
	// 		canvas->setParameter(0,slider->getValue());
	// 	else
	// 		canvas->setParameter(1,slider->getValue());
	// }

	
	//else 
	//	getAudioProcessor()->setParameter(1,slider->getValue());

}