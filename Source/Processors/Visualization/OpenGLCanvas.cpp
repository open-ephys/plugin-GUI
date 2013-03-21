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

#include "OpenGLCanvas.h"
#include <stdio.h>
#include <math.h>

OpenGLCanvas::OpenGLCanvas() : //OpenGLComponent(OpenGLComponent::OpenGLType::openGLDefault, true),
	scrollPix(0), animationIsActive(false), refreshMs(5000),
	scrollBarWidth(15), scrollDiff(0), originalScrollPix(0),
	scrollTime(0), showScrollTrack(true), PI(3.1415926)
{

}

OpenGLCanvas::~OpenGLCanvas()
{
	
}

void OpenGLCanvas::startCallbacks()
{
	startTimer(refreshMs);
	animationIsActive = true;
}

void OpenGLCanvas::stopCallbacks()
{
	stopTimer();
	animationIsActive = false;
}

void OpenGLCanvas::paint(Graphics& g)
{

	paintCanvas(g);

	drawScrollBars(g);

}

void OpenGLCanvas::drawScrollBars(Graphics& g)
{

	//std::cout << "Drawing scroll bars" << std::endl;
	
	float scrollBarY = float(getHeight())/float(getTotalHeight());
	float timeSinceScroll = timer.getMillisecondCounter()-scrollTime;
	
	if (scrollBarY < 1.0f && timeSinceScroll < 1300)
	{
		float alpha;

		if (timeSinceScroll < 1000)
			alpha = 1.0f;
		else
			alpha = 1.0f*(1-float(timeSinceScroll-1000)/300.0f);

		float Yoffset = float(scrollPix)/float(getTotalHeight());

		if (showScrollTrack)
			drawScrollBar(g, 0.995f, 2.0f, alpha*0.2f);

		scrollBarBottom = scrollBarY + Yoffset - 2.0f;
		scrollBarTop = Yoffset + 2.0f;
		
		drawScrollBar(g, scrollBarBottom, scrollBarTop, alpha*0.5f);

	} else {
		if (!animationIsActive) {
			stopTimer(); 
		}
		showScrollTrack = false;
	}

}

void OpenGLCanvas::drawScrollBar(Graphics& g, float y1, float y2, float alpha)
{

	glViewport(0, getFooterHeight(),
		       getWidth(),
		       getHeight()-getHeaderHeight()-getFooterHeight());

	float x1 = getWidth()-8.0f;

	g.setColour(Colours::black.withAlpha(alpha));

	g.fillRect(x1, y1, 1.0f, y2-y1);

}

void OpenGLCanvas::showScrollBars()
{
	scrollTime = timer.getMillisecondCounter();
	startTimer(refreshMs);
}


void OpenGLCanvas::mouseMove(const MouseEvent& e)
{
	if (getTotalHeight() > getHeight()) {

		Point<int> pos = e.getPosition();
		int xcoord = pos.getX();
		if (xcoord > getWidth() - scrollBarWidth)
		{
			showScrollTrack = true; showScrollBars();
		}
	}

	mouseMoveInCanvas(e);
}

void OpenGLCanvas::mouseDown(const MouseEvent& e)
{

	if (getTotalHeight() > getHeight()) {

	Point<int> pos = e.getPosition();
	int xcoord = pos.getX();

	if (xcoord > getWidth()-scrollBarWidth) {

		int ycoord = pos.getY();

		float targetPoint = float(ycoord)/float(getHeight());

		if (targetPoint < scrollBarTop && targetPoint < scrollBarTop)
		{

			scrollPix = int(float(ycoord)/float(getHeight())*float(getTotalHeight()));

		} else if (targetPoint > scrollBarBottom && targetPoint > scrollBarBottom) {
			
			scrollPix = int(float(ycoord)/float(getHeight())*float(getTotalHeight())) -
						(scrollBarBottom-scrollBarTop)*float(getTotalHeight());

		}
		
		showScrollTrack = true;
		showScrollBars();
	}
	}

	mouseDownInCanvas(e);
}

void OpenGLCanvas::mouseDrag(const MouseEvent& e)
{

	if (getTotalHeight() > getHeight()) {
	if (e.getMouseDownX() > getWidth()-scrollBarWidth)
	{

		if (float(e.getMouseDownY()/float(getHeight())) > scrollBarTop &&
		    float(e.getMouseDownY()/float(getHeight())) < scrollBarBottom)
		{

			if (scrollDiff == 0)
			{
				originalScrollPix = scrollPix;
				scrollDiff = 1;
			}

		}

		if (scrollDiff == 1)
		{
			scrollPix = originalScrollPix + 
				float(e.getDistanceFromDragStartY())/float(getHeight())
				* float(getTotalHeight());

			if (scrollPix < 0)
				scrollPix = 0;
			
			if (scrollPix + getHeight() > getTotalHeight())
				scrollPix = getTotalHeight() - getHeight();
	
			scrollTime = timer.getMillisecondCounter();
			showScrollTrack = true;
			repaint();
		} 
	}
	}

	mouseDragInCanvas(e);
}

void OpenGLCanvas::mouseUp(const MouseEvent& e)
{
	scrollDiff = 0;

	mouseUpInCanvas(e);
}

int OpenGLCanvas::mouseWheelMove(const MouseEvent&e,
                                      float wheelIncrementX, float wheelIncrementY)

{
	if (getTotalHeight() > getHeight()) {

		if (wheelIncrementY > 0)
		{
			if (scrollPix + getHeight() < getTotalHeight())
			{
				scrollPix += int(100.0f*wheelIncrementY);
				if (scrollPix + getHeight() > getTotalHeight())
					scrollPix = getTotalHeight() - getHeight();
			}
		} else if (wheelIncrementY < 0)
		{
			if (scrollPix > 0)
			{
				scrollPix += int(100.0f*wheelIncrementY);
				if (scrollPix < 0)
					scrollPix = 0;
			}
		}

		repaint();

		showScrollBars();

	}

	mouseWheelMoveInCanvas(e, wheelIncrementX, wheelIncrementY);

}

void OpenGLCanvas::timerCallback()
{
	repaint();
}


void OpenGLCanvas::resized()
{

	if (scrollPix + getHeight() > getTotalHeight() && getTotalHeight() > getHeight())
		scrollPix = getTotalHeight() - getHeight();
	else
		scrollPix = 0;

	showScrollBars();

	canvasWasResized();
}
