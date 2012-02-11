/*
  ==============================================================================

    OpenGLCanvas.h
    Created: 27 Jan 2012 4:23:10pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __OPENGLCANVAS_H_98F0C13D__
#define __OPENGLCANVAS_H_98F0C13D__

#include "../../../JuceLibraryCode/JuceHeader.h"

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

#include <FTGL/ftgl.h>

class OpenGLCanvas : public OpenGLComponent, Timer

{
public:
	OpenGLCanvas();
	~OpenGLCanvas();

	void setUp2DCanvas();
	void activateAntiAliasing();

	void mouseDownInCanvas(const MouseEvent& e);
	void mouseDragInCanvas(const MouseEvent& e);
	void mouseMoveInCanvas(const MouseEvent& e);
	void mouseUpInCanvas(const MouseEvent& e);
	void mouseWheelMoveInCanvas(const MouseEvent&, float, float);

	virtual void resized();
	virtual void mouseDown(const MouseEvent& e);
	virtual void mouseDrag(const MouseEvent& e);
	virtual void mouseMove(const MouseEvent& e);
	virtual void mouseUp(const MouseEvent& e);
	virtual void mouseWheelMove(const MouseEvent&, float, float);

	void startCallbacks();
	void stopCallbacks();

	void canvasWasResized();

	int getScrollAmount() {return scrollPix;};
	int getScrollBarWidth() {return scrollBarWidth;}
	void drawScrollBars();

	void drawRoundedRect(float x, float y, float w, float h, float r, int n);
	
	FTGLPixmapFont* getFont(String fontName);

protected:

	virtual int getTotalHeight() = 0;

private:

	bool animationIsActive;

	int refreshMs;

	void loadFonts();

	void drawScrollBar(float y1, float y2, float alpha);
	
	void showScrollBars();

	int scrollBarWidth, scrollPix, scrollDiff, originalScrollPix;
	int scrollTime;
	bool showScrollTrack;

	Time* timer;
	void timerCallback();

	float scrollBarTop, scrollBarBottom;

	OwnedArray<FTGLPixmapFont> fontList;

	const float PI;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLCanvas);	

};


#endif  // __OPENGLCANVAS_H_98F0C13D__
