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

}

void DataWindow::closeButtonPressed()
{
	setVisible(false);
	controlButton->setToggleState(false,false);
}