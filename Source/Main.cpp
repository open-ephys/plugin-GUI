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

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainWindow.h"
#include "UI/CustomLookAndFeel.h"

#include <stdio.h>

//------------------------------------------------------------------

/**
  
  Launches the application and creates the CustomLookAndFeelClass.

  The OpenEphysApplication class own the application's MainWindow (via
  a ScopedPointer).

  @see MainWindow

*/


class OpenEphysApplication  : public JUCEApplication
{
public:
    //==============================================================================
    OpenEphysApplication() {}

    ~OpenEphysApplication() {}

    //==============================================================================
    void initialise (const String& commandLine)
    {
        mainWindow = new MainWindow();
        mainWindow->setUsingNativeTitleBar (true);

        customLookAndFeel = new CustomLookAndFeel();
        LookAndFeel::setDefaultLookAndFeel(customLookAndFeel);
  
    }

    void shutdown() { }

    //==============================================================================
    void systemRequestedQuit()
    {
        std::cout << "Quit requested" << std::endl;
        quit();
    }

    //==============================================================================
    const String getApplicationName() { return "Open Ephys GUI";}
    const String getApplicationVersion() {return ProjectInfo::versionString;}
    bool moreThanOneInstanceAllowed() {return true;}
    void anotherInstanceStarted (const String& commandLine)
    {}

private:
    ScopedPointer <MainWindow> mainWindow;
    ScopedPointer <CustomLookAndFeel> customLookAndFeel;
};

//==============================================================================
// This macro generates the main() routine that starts the app.
START_JUCE_APPLICATION(OpenEphysApplication)
