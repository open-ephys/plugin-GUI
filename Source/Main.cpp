/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#include <fstream>
#include <stdio.h>

/**
 * This function is called when the console window is closed.
 * Handles the CTRL+C, CTRL+BREAK, and console close button  events.
*/
#ifdef _WIN32
BOOL WINAPI ConsoleHandler (DWORD CEvent)
{
    switch (CEvent)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
            JUCEApplication::getInstance()->systemRequestedQuit();
            return TRUE;
        case CTRL_CLOSE_EVENT:
            JUCEApplication::getInstance()->quit();
            return TRUE;
        default:
            return FALSE;
    }
}
#endif

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

    void initialise (const String& commandLine)
    {
        std::cout << commandLine << std::endl;

        StringArray parameters;
        parameters.addTokens (commandLine, " ", "\"");
        parameters.removeEmptyStrings();

#ifdef _WIN32

        SetConsoleTitleA ("Open Ephys GUI Launcher");

        std::cout << "Initializing Open Ephys GUI... DO NOT CLOSE THIS WINDOW" << std::endl;

        SetConsoleCtrlHandler (ConsoleHandler, TRUE);

        if (HWND hwnd = GetConsoleWindow())
        {
            if (HMENU hMenu = GetSystemMenu (hwnd, FALSE))
            {
                EnableMenuItem (hMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            }
        }

#endif

        SystemStats::setApplicationCrashHandler (handleCrash);

        // Parse parameters
        if (! parameters.isEmpty())
        {
            bool isConsoleApp = false;
            File fileToLoad;

            for (auto param : parameters)
            {
                if (param.equalsIgnoreCase ("--headless"))
                {
                    isConsoleApp = true;
                }
                else if (fileToLoad.getFullPathName().isEmpty())
                {
                    File localPath (File::getCurrentWorkingDirectory().getChildFile (param));

                    if (localPath.existsAsFile())
                    {
                        fileToLoad = localPath;
                        continue;
                    }

                    File globalPath (param);

                    if (globalPath.existsAsFile())
                        fileToLoad = globalPath;
                }
            }

            mainWindow = std::make_unique<MainWindow> (fileToLoad, isConsoleApp);
        }
        else
        {
            mainWindow = std::make_unique<MainWindow>();
        }
    }

    void shutdown() {}

    static void handleCrash (void* input)
    {
        MainWindow::handleCrash (input);
    }

    void systemRequestedQuit()
    {
        bool shouldQuit = true;

        if (CoreServices::getAcquisitionStatus())
        {
            String message;

            if (CoreServices::getRecordingStatus())
            {
                AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                             "Cannot quit while recording is active.",
                                             "Please stop recording before closing the GUI.",
                                             "OK");
                shouldQuit = false;
            }
            else
            {
                shouldQuit = AlertWindow::showOkCancelBox (AlertWindow::WarningIcon,
                                                           "Are you sure you want to quit?",
                                                           "The GUI is still acquiring data.",
                                                           "Yes",
                                                           "No");
            }
        }

        if (shouldQuit)
        {
            mainWindow->shutDownGUI();
            quit();
        }
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

    void anotherInstanceStarted (const String& commandLine)
    {
    }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that starts the app.
START_JUCE_APPLICATION (OpenEphysApplication)
