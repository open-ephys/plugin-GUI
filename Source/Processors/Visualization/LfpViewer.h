/*
  ==============================================================================

    LfpViewer.h
    Created: 16 Aug 2011 8:46:57pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __LFPVIEWER_H_7FEACF46__
#define __LFPVIEWER_H_7FEACF46__

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



class LfpViewer : public Renderer
{
public:
	LfpViewer(AudioSampleBuffer* streamBuffer, MidiBuffer* eventBuffer, UIComponent* ui);
	~LfpViewer();
	void renderOpenGL();	
	void newOpenGLContextCreated();
	void mouseDown(const MouseEvent &e);
};


#endif  // __LFPVIEWER_H_7FEACF46__
