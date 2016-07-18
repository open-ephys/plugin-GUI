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

{
    centreWithSize(800,500);
    setUsingNativeTitleBar(true);
    setResizable(true,false);
}

DataWindow::~DataWindow()
{

}

void DataWindow::closeButtonPressed()
{
    setContentNonOwned(0,false);
    setVisible(false);
    controlButton->setToggleState(false,dontSendNotification);
    // with the BailOutChecker, it is safe to "delete" a DataWindow instance
    // from this callback/listener. This would (typically) not be done, because instances
    // of DataWindow are (typically) "owned" by Editors, and will be deleted
    // when the Editor dies.
    //
    Component::BailOutChecker checker (this);
    if (! checker.shouldBailOut()){
        closeWindowListeners.callChecked (checker, &DataWindow::Listener::windowClosed);
    }
}

void DataWindow::addListener (DataWindow::Listener* const newListener)
{
    closeWindowListeners.add (newListener);
}

void DataWindow::removeListener (DataWindow::Listener* const listener)
{
    closeWindowListeners.remove (listener);
}
