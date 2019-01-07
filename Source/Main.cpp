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
#ifdef WIN32
#include <Windows.h>
#define _MAIN
#endif
#include "../JuceLibraryCode/JuceHeader.h"
#include "MainWindow.h"
#include "UI/LookAndFeel/CustomLookAndFeel.h"

#include <stdio.h>
#include <fstream>

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
    void initialise(const String& commandLine)
    {

        std::cout << commandLine << std::endl;

        StringArray parameters;
        parameters.addTokens(commandLine," ","\"");
        parameters.removeEmptyStrings();

#ifdef WIN32
        //glWinInit();

        int consoleArg = parameters.indexOf("--console", true);
        if (consoleArg != -1)
        {
            parameters.remove(consoleArg);
            if (AllocConsole())
            {
                freopen("CONOUT$","w",stdout);
				freopen("CONOUT$","w",stderr);
                console_out = std::ofstream("CONOUT$");
                std::cout.rdbuf(console_out.rdbuf());
				std::cerr.rdbuf(console_out.rdbuf());
                SetConsoleTitle("Debug Console");
				std::cout << "Debug console..." << std::endl;
            }
        }

#endif

        customLookAndFeel = new CustomLookAndFeel();
        LookAndFeel::setDefaultLookAndFeel(customLookAndFeel);


        // signal chain to load
        if (!parameters.isEmpty())
        {
            File fileToLoad(File::getCurrentWorkingDirectory().getChildFile(parameters[0]));
            mainWindow = new MainWindow(fileToLoad);
        }
        else
        {
            mainWindow = new MainWindow();
        }
    }

    void shutdown() { }

    //==============================================================================
    void systemRequestedQuit()
    {
        //std::cout << "Quit requested" << std::endl;
        quit();
    }

    //==============================================================================
    const String getApplicationName()
    {
        return "Open Ephys GUI";
    }
    const String getApplicationVersion()
    {
        return ProjectInfo::versionString;
    }
    bool moreThanOneInstanceAllowed()
    {
        return true;
    }
    void anotherInstanceStarted(const String& commandLine)
    {}

private:
    ScopedPointer <MainWindow> mainWindow;
    ScopedPointer <CustomLookAndFeel> customLookAndFeel;
    std::ofstream console_out;
};

//==============================================================================
// This macro generates the main() routine that starts the app.
START_JUCE_APPLICATION(OpenEphysApplication)
