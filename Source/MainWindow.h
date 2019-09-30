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

#ifndef __MAINWINDOW_H_BA75E17__
#define __MAINWINDOW_H_BA75E17__

#include "../JuceLibraryCode/JuceHeader.h"
#include "UI/UIComponent.h"
#include "Audio/AudioComponent.h"
#include "Processors/ProcessorGraph/ProcessorGraph.h"

/**
  The main window for the GUI application.

  This object creates and destroys the AudioComponent, the ProcessorGraph,
  and the UIComponent (which exists as the ContentComponent of this window).

  @see AudioComponent, ProcessorGraph, UIComponent

*/


class MainWindow   : public DocumentWindow
{
public:

    /** Initializes the MainWindow, creates the AudioComponent, ProcessorGraph,
        and UIComponent, and sets the window boundaries. */
    MainWindow(const File& fileToLoad = File());

    /** Destroys the AudioComponent, ProcessorGraph, and UIComponent, and saves the window boundaries. */
    ~MainWindow();

    /** Called when the user hits the close button of the MainWindow. This destroys
        the MainWindow and closes the application. */
    void closeButtonPressed();

    /** A JUCE class that allows the MainWindow to respond to keyboard and menubar
        commands. */
    ApplicationCommandManager commandManager;

    /** Determines whether the last used configuration reloads upon startup. */
    bool shouldReloadOnStartup;

	void shutDownGUI();

private:

    /** Saves the MainWindow's boundaries into the file "windowState.xml", located in the directory
        from which the GUI is run. */
    void saveWindowBounds();

    /** Loads the MainWindow's boundaries into the file "windowState.xml", located in the directory
        from which the GUI is run. */
    void loadWindowBounds();

    /** A pointer to the application's AudioComponent (owned by the MainWindow). */
    ScopedPointer<AudioComponent> audioComponent;

    /** A pointer to the application's ProcessorGraph (owned by the MainWindow). */
    ScopedPointer<ProcessorGraph> processorGraph;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)

};


#endif  // __MAINWINDOW_H_BA75E17__
