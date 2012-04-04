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

#ifndef __OPENGLCANVAS_H_98F0C13D__
#define __OPENGLCANVAS_H_98F0C13D__

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../../OpenGL.h"

class OpenGLCanvas : public OpenGLComponent, Timer

{
public:
	OpenGLCanvas();
	~OpenGLCanvas();

	void setUp2DCanvas();
	void activateAntiAliasing();

	virtual void refreshState() {};



	void resized();
	virtual void canvasWasResized() { }

	void mouseDown(const MouseEvent& e);
	void mouseDrag(const MouseEvent& e);
	void mouseMove(const MouseEvent& e);
	void mouseUp(const MouseEvent& e);
	void mouseWheelMove(const MouseEvent&, float, float);

	virtual void mouseDownInCanvas(const MouseEvent& e) {}
	virtual void mouseDragInCanvas(const MouseEvent& e) {}
	virtual void mouseMoveInCanvas(const MouseEvent& e) {}
	virtual void mouseUpInCanvas(const MouseEvent& e) {}
	virtual void mouseWheelMoveInCanvas(const MouseEvent&,
									    float,
									    float) {}

	void startCallbacks();
	void stopCallbacks();

	int getScrollAmount() {return scrollPix;};
	int getScrollBarWidth() {return scrollBarWidth;}
	void drawScrollBars();

	void drawRoundedRect(float x, float y, float w, float h, float r, int n);
	
	FTGLPixmapFont* getFont(String fontName);

protected:

	virtual int getTotalHeight() = 0;
	int scrollPix;
	void showScrollBars();

private:

	bool animationIsActive;

	int refreshMs;

	void loadFonts();

	void drawScrollBar(float y1, float y2, float alpha);
	

	int scrollBarWidth, scrollDiff, originalScrollPix;
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
