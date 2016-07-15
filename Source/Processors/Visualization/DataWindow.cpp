/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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


DataWindow::DataWindow(Button* cButton, String name)
    : DocumentWindow(name,
                     Colours::black,
                     DocumentWindow::allButtons)
    , controlButton(cButton)
    , vizEditor(nullptr)

{
    centreWithSize(800,500);
    setUsingNativeTitleBar(true);
    setResizable(true,false);
}

DataWindow::DataWindow(Button* button, VisualizerEditor* editor, String name)
    : DataWindow(button, name)
{
    vizEditor = editor;
}

DataWindow::~DataWindow()
{

}

void DataWindow::closeButtonPressed()
{
    setContentNonOwned(0,false);
    setVisible(false);
    controlButton->setToggleState(false,dontSendNotification);
    if (vizEditor != nullptr){
        dynamic_cast<VisualizerEditor*>(vizEditor)->windowClosed();
    }
}
