/*
  ==============================================================================

    WiFiOutputEditor.cpp
    Created: 16 Feb 2012 11:13:03am
    Author:  jsiegle

  ==============================================================================
*/


#include "WiFiOutputEditor.h"
#include <stdio.h>


WiFiOutputEditor::WiFiOutputEditor (GenericProcessor* parentNode, FilterViewport* vp) 
	: GenericEditor(parentNode, vp)

{

	accumulator = 0;

	desiredWidth = 150;

	Image im;
	im = ImageCache::getFromMemory (BinaryData::wifi_png, 
	 								BinaryData::wifi_pngSize);

	icon = new ImageIcon(im);
	addAndMakeVisible(icon);
	icon->setBounds(35,35,80,80);

	icon->setOpacity(0.3f);

}

WiFiOutputEditor::~WiFiOutputEditor()
{
	deleteAllChildren();
}

void WiFiOutputEditor::receivedEvent()
{
	
	icon->setOpacity(0.8f);
	startTimer(50);

}

void WiFiOutputEditor::timerCallback()
{
	if (accumulator < 10)
	{
		icon->setOpacity(0.8f-(0.05*float(accumulator)));
		accumulator++;
	} else {
		icon->setOpacity(0.3f);
		stopTimer();
		accumulator = 0;
	}
	
}