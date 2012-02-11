/*
  ==============================================================================

    DataWindow.cpp
    Created: 8 Feb 2012 1:10:13pm
    Author:  jsiegle

  ==============================================================================
*/

#include "DataWindow.h"


DataWindow::DataWindow(Button* cButton)
	: DocumentWindow ("Stream Window", 
					  Colours::black, 
					  DocumentWindow::allButtons),
	  controlButton(cButton)

{
	centreWithSize(500,500);
	setUsingNativeTitleBar(true);
	setResizable(true,false);
	//setTitleBarHeight(40);
}

DataWindow::~DataWindow()
{
	//setContentComponent (0);
}

void DataWindow::closeButtonPressed()
{
	setVisible(false);
	controlButton->setToggleState(false,false);
}