/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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
#include "glWinInit.h"
#endif
#include "../JuceLibraryCode/JuceHeader.h"
#include "MainWindow.h"
#include "UI/CustomLookAndFeel.h"

#include <stdio.h>

#ifdef WIN32
//Initializer function to load OpenGL Extensions on Windows
void glWinInit()
{
glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) wglGetProcAddress("glCheckFramebufferStatusEXT");
glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) wglGetProcAddress("glGenerateMipmap");
glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) wglGetProcAddress("glFramebufferRenderbufferEXT");
glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) wglGetProcAddress("glFramebufferTexture2DEXT");
glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) wglGetProcAddress("glRenderbufferStorageEXT");
glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) wglGetProcAddress("glBindRenderbufferEXT");
glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) wglGetProcAddress("glGenRenderbuffersEXT");
glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) wglGetProcAddress("glBindFramebufferEXT");
glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) wglGetProcAddress("glGenFramebuffersEXT");
glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) wglGetProcAddress("glDeleteRenderbuffers");
glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) wglGetProcAddress("glDeleteFramebuffers");
}
#endif

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
		StringArray parameters;
		parameters.addTokens(commandLine," ","\"");
#ifdef WIN32
		//glWinInit();

		if (parameters.contains("--console",true)) {
			if (AllocConsole())
			{
				freopen("CONOUT$","w",stdout);
				SetConsoleTitle("Debug Console");
			}
		}

#endif

        mainWindow = new MainWindow();
       

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
