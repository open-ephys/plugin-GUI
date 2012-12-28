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

#ifndef __VISUALIZER_H_C5943EC1__
#define __VISUALIZER_H_C5943EC1__

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../../JuceLibraryCode/JuceHeader.h"
#include "OpenGLCanvas.h"

/**

  Abstract base class for displaying data.

  @see LfpDisplayCanvas, SpikeDisplayCanvas

*/

class Visualizer : public OpenGLCanvas

{
public: 
	Visualizer() {}
	~Visualizer() {}

	virtual void newOpenGLContextCreated() = 0;
	virtual void renderOpenGL() = 0;

	virtual void refreshState() = 0;

	virtual void update() = 0;
	virtual int getTotalHeight() = 0;

	virtual void beginAnimation() = 0;
	virtual void endAnimation() = 0;

	virtual void setParameter(int, float) = 0;
    virtual void setParameter(int, int, int, float) = 0;

};


#endif  // __VISUALIZER_H_C5943EC1__
