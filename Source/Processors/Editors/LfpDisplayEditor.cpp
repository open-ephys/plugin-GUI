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
	  //streamBuffer(0), eventBuffer(0), canvas(0),
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

		for (int n = 0; n < getNumChildComponents(); n++)
	{
		Button* c = (Button*) getChildComponent(n);

		if (c->isVisible())
			c->addListener(this);

		if (c->getRadioGroupId() != 999)
			c->setVisible(true);
	}

	// timebaseSlider = new Slider (T("Time Base Slider"));
	// timebaseSlider->setBounds(60,20,200,40);
	// timebaseSlider->setRange(500,10000,500);
	// timebaseSlider->addListener(this);
	// addAndMakeVisible(timebaseSlider);

	// displayGainSlider = new Slider (T("Display Gain Slider"));
	// displayGainSlider->setBounds(60,65,200,40);
	// displayGainSlider->setRange(1,8,1);
	// displayGainSlider->addListener(this);
	// addAndMakeVisible(displayGainSlider);

	// windowSelector = new SelectorButton();
	// windowSelector->addListener(this);
	// windowSelector->setBounds(25,25,20,20);
	// windowSelector->setToggleState(false,false);
	// addAndMakeVisible(windowSelector);

	tabSelector = new SelectorButton();
	tabSelector->addListener(this);
	tabSelector->setBounds(215,30,25,25);
	
	addAndMakeVisible(tabSelector);
	tabSelector->setToggleState(false,false);

	//canvas = new LfpDisplayCanvas((LfpDisplayNode*) getProcessor());


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

void LfpDisplayEditor::updateNumInputs(int n)
{
	std::cout << "Setting num inputs on LfpDisplayEditor to " << n << std::endl;
	if (canvas != 0)
		canvas->updateNumInputs(n);
}

void LfpDisplayEditor::updateSampleRate(float r) 
{
	if (canvas != 0)
		canvas->updateSampleRate(r);
}

void LfpDisplayEditor::setBuffers(AudioSampleBuffer* asb, MidiBuffer* mb)
{
	std::cout << "LfpDisplayEditor buffers are set!" << std::endl;
	//streamBuffer = asb;
	//eventBuffer = mb;

	//std::cout << streamBuffer << std::endl;
	//std::cout << eventBuffer << std::endl;
}

void LfpDisplayEditor::buttonClicked(Button* button)
{

	if (!checkDrawerButton(button) && !checkChannelSelectors(button)) {

	int gId = button->getRadioGroupId();

	if (gId > 0) {
		if (canvas != 0)
		{
			canvas->setParameter(gId-1, button->getName().getFloatValue());
		}

	} else {

	if (canvas == 0) {
		
		LfpDisplayNode* processor = (LfpDisplayNode*) getProcessor();
		canvas = new LfpDisplayCanvas(processor);

		if (isPlaying)
			canvas->beginAnimation();
	}

	// if (button == windowSelector)
	// {
	// 	if (dataWindow == 0) {

	// 		dataWindow = new DataWindow(windowSelector);

	// 		//if (canvas == 0)
	// 		//	canvas = new LfpDisplayCanvas((LfpDisplayNode*) getProcessor());

	// 		//dataWindow->setContentComponent(new LfpDisplayCanvas(streamBuffer,eventBuffer,getConfiguration(), this));

	// 		if (tabSelector->getToggleState())
	// 		{
	// 			tabSelector->setToggleState(false, false);
	// 			dataViewport->removeTab(tabIndex);
	// 			tabIndex = -1;
	// 		}

	// 		//LfpDisplayNode* p = (LfpDisplayNode*) getProcessor();

	// 		dataWindow->setContentNonOwned(canvas, false);
	// 		//p->isVisible = true;

	// 		//getProcessor()->parentComponentChanged();
	// 		dataWindow->setVisible(true);
			
	// 	} else {

	// 		if (tabSelector->getToggleState())
	// 		{
	// 			tabSelector->setToggleState(false, false);
	// 			dataViewport->removeTab(tabIndex);
	// 			tabIndex = -1;
	// 		}

	// 		dataWindow->setVisible(windowSelector->getToggleState());

	// 		//LfpDisplayNode* p = (LfpDisplayNode*) getProcessor();
	// 		//p->isVisible = windowSelector->getToggleState();
	// 		//getProcessor()->parentComponentChanged();
	// 	}

	// } else
		 if (button == tabSelector)
		{
			if (tabSelector->getToggleState() && tabIndex < 0)
			{

				//std::cout << "Editor data viewport: " << dataViewport << std::endl;
				
				// if (windowSelector->getToggleState())
				// {
				// 	windowSelector->setToggleState(false, false);
				// 	dataWindow->setVisible(false);
				// }

				//tabIndex = dataViewport->addTabToDataViewport("LFP",new LfpDisplayCanvas(streamBuffer,eventBuffer,getConfiguration(), this));
				//Component* p = (Component*) getProcessor();

				//LfpDisplayNode* p = (LfpDisplayNode*) getProcessor();
				tabIndex = getDataViewport()->addTabToDataViewport("LFP",canvas);
				//if (isPlaying)
				///{
				//	canvas->beginAnimation();
				//}

				//p->isVisible = true;

			} else if (!tabSelector->getToggleState() && tabIndex > -1)
			{
				getDataViewport()->removeTab(tabIndex);
				tabIndex = -1;
				//LfpDisplayNode* p = (LfpDisplayNode*) getProcessor();
				//p->isVisible = false;
			}
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