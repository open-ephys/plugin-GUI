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

#ifndef __SPIKEVIEWER_H_AF442416__
#define __SPIKEVIEWER_H_AF442416__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/Visualizer.h"

#ifdef _WIN32
#include <windows.h>
#endif

#if JUCE_WINDOWS
#include <gl/gl.h>
#include <gl/glu.h>
#elif JUCE_LINUX
#include <GL/gl.h>
#include <GL/glut.h>
#undef KeyPress
#elif JUCE_IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#elif JUCE_MAC
#include <GLUT/glut.h>
#endif

#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80e1
#endif

/**
  
  --THIS FILE IS OBSOLETE, BUT REMAINS FOR REFERENCE PURPOSES--

*/


class SpikeViewer : public Renderer
{
public:
	SpikeViewer(AudioSampleBuffer* streamBuffer, MidiBuffer* eventBuffer, UIComponent* ui);
	~SpikeViewer();
	void renderOpenGL();	
	void newOpenGLContextCreated();

	//void setViewportForProjection
private:
	int nTrodes;

	Array<float> peaks;

	float xBox, yBox, xPadding, yPadding;

	void drawBorder();

	void clearWaveforms();
	void clearProjections();

	void drawProjections();
	float drawWaveform(uint8* dataptr, int numSamples);

	void setViewportForWaveN(int n);
	void setViewportForProjectionN(int n);

	void resized();

};


#endif  // __SPIKEVIEWER_H_AF442416__
