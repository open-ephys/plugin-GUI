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

#ifndef __INFOLABEL_H_14DA9A62__
#define __INFOLABEL_H_14DA9A62__

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/Visualization/OpenGLCanvas.h"

/**
  
  Displays general instructions about how to use the application.

  Inhabits a tab in the DataViewport.

  @see UIComponent, DataViewport

*/

class InfoLabel : public OpenGLCanvas

{
public: 
	InfoLabel();
	~InfoLabel();

    /** Initializes an OpenGL context for drawing.*/
	void newOpenGLContextCreated();

    /** Draws the InfoLabel.*/
	void renderOpenGL();

private:

	int xBuffer, yBuffer;

    /** Draws the InfoLabel.*/
	void drawLabel();

    /** Returns the requested height of the InfoLabel (used to create
    scroll bars if the height exceeds the actual height of the component).*/
	int getTotalHeight();

    /** Called when the boundaries of the InfoLabel are changed.*/
	void canvasWasResized();

    /** An FTGL layout class used to constrain the text.*/
	FTSimpleLayout layout;	

    /** The text displayed to the user.*/
	String infoString;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoLabel);


};




#endif  // __INFOLABEL_H_14DA9A62__
