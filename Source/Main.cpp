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
#ifdef _WIN32
#include <winsock2.h>
#include <Windows.h>
#define _MAIN
#endif
#include "../JuceLibraryCode/JuceHeader.h"
#include "MainWindow.h"
#include "UI/LookAndFeel/CustomLookAndFeel.h"

#include <stdio.h>
#include <fstream>

/**

  Launches the application and creates the CustomLookAndFeelClass.

  The OpenEphysApplication class own the application's MainWindow (via
  a ScopedPointer).

  @see MainWindow

*/

class OpenEphysApplication : public JUCEApplication
{
public:

    OpenEphysApplication() {}

    ~OpenEphysApplication() {}

    void initialise(const String& commandLine)
    {

        std::cout << commandLine << std::endl;

        StringArray parameters;
        parameters.addTokens(commandLine, " ", "\"");
        parameters.removeEmptyStrings();

#ifdef _WIN32

        if (AllocConsole())
        {
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
            console_out = std::ofstream("CONOUT$");
            std::cout.rdbuf(console_out.rdbuf());
            std::cerr.rdbuf(console_out.rdbuf());
            SMALL_RECT windowSize = { 0, 0, 85 - 1, 35 - 1 };
            COORD bufferSize = { 85 , 9999 };
            HANDLE wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTitle("Open Ephys GUI ::: Console");
            SetConsoleWindowInfo(wHnd, true, &windowSize);
            SetConsoleScreenBufferSize(wHnd, bufferSize);
        }

#endif

        SystemStats::setApplicationCrashHandler(handleCrash);

        customLookAndFeel = std::make_unique<CustomLookAndFeel>();
        LookAndFeel::setDefaultLookAndFeel(customLookAndFeel.get());

        // signal chain to load
        if (!parameters.isEmpty())
        {
            File fileToLoad(File::getCurrentWorkingDirectory().getChildFile(parameters[0]));
            mainWindow = std::make_unique<MainWindow>(fileToLoad);
        }
        else
        {
            mainWindow = std::make_unique<MainWindow>();
        }
    }

    void shutdown() { }

    static void handleCrash(void* input)
    {
        MainWindow::handleCrash(input);
    }

    void systemRequestedQuit()
    {
        mainWindow->shutDownGUI();

        quit();
    }

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
    std::unique_ptr <MainWindow> mainWindow;
    std::unique_ptr <CustomLookAndFeel> customLookAndFeel;
    std::ofstream console_out;
};

//==============================================================================
// This macro generates the main() routine that starts the app.
START_JUCE_APPLICATION(OpenEphysApplication)
