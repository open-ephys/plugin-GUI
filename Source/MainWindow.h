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

#ifndef __MAINWINDOW_H_BA75E17__
#define __MAINWINDOW_H_BA75E17__

#include "../JuceLibraryCode/JuceHeader.h"
#include "UI/UIComponent.h"
#include "Audio/AudioComponent.h"
#include "Processors/ProcessorGraph.h"

//-----------------------------------------------------------------------

/**
  The main window for the GUI application.

  This object creates and destroys the AudioComponent, the ProcessorGraph,
  and the UIComponent (which exists as the ContentComponent of this window).

  @see AudioComponent, ProcessorGraph, UIComponent

*/


class MainWindow   : public DocumentWindow
{
public:
    //=======================================================================
    MainWindow();
    ~MainWindow();

    void closeButtonPressed();

    ApplicationCommandManager commandManager;

private:
    //========================================================================
  
    void saveWindowBounds();
    void loadWindowBounds();

   AudioComponent* audioComponent;
   ProcessorGraph* processorGraph;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)

};


#endif  // __MAINWINDOW_H_BA75E17__
