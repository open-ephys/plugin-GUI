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

#include "VisualizerEditor.h"

SelectorButton::SelectorButton(const String& name_)
	: Button(name_)
{
	setClickingTogglesState (true);
    setTooltip ("Toggle a state.");
}

SelectorButton::~SelectorButton()
{
}

void SelectorButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour(Colours::white);
    else 
        g.setColour(Colours::darkgrey);

    if (isMouseOver)
        g.setColour(Colours::yellow);


   	if (getName().equalsIgnoreCase("window"))
   	{
   		// window icon
   		g.drawRect(0,0,getWidth(),getHeight(),1.0);
   		g.fillRect(0,0,getWidth(),3.0);
   	} else {
   		// tab icon
   		g.drawRect(0,0,10,getHeight());
   	}
  
}




VisualizerEditor::VisualizerEditor (GenericProcessor* parentNode) 
	: GenericEditor(parentNode),
	  tabIndex(-1), dataWindow(0),
	  isPlaying(false),
	  canvas(0)

{

	desiredWidth = 250;

	windowSelector = new SelectorButton("window");
	windowSelector->addListener(this);
	windowSelector->setBounds(190,5,15,10);

	windowSelector->setToggleState(false,false);
	addAndMakeVisible(windowSelector);

	tabSelector = new SelectorButton("tab");
	tabSelector->addListener(this);
	tabSelector->setBounds(210,5,15,10);
	
	addAndMakeVisible(tabSelector);
	tabSelector->setToggleState(false,false);

}

VisualizerEditor::~VisualizerEditor()
{

	if (tabIndex > -1)
	{
		getDataViewport()->removeTab(tabIndex);
	}

	deleteAllChildren();

}

void VisualizerEditor::enable()
{
	std::cout << "   Enabling VisualizerEditor" << std::endl;
	if (canvas != 0)
		canvas->beginAnimation();
	
	isPlaying = true;
}

void VisualizerEditor::disable()
{
	if (canvas != 0)
		canvas->endAnimation();

	isPlaying = false;
}

void VisualizerEditor::updateVisualizer()
{

	if (canvas != 0)
		canvas->update();

}

void VisualizerEditor::buttonEvent(Button* button)
{

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

	buttonCallback(button);

}

