/*
  ==============================================================================

    OpenGLCanvas.cpp
    Created: 27 Jan 2012 4:23:10pm
    Author:  jsiegle

  ==============================================================================
*/

#include "OpenGLCanvas.h"
#include <stdio.h>
#include <math.h>

OpenGLCanvas::OpenGLCanvas() : //OpenGLComponent(OpenGLComponent::OpenGLType::openGLDefault, true),
	scrollPix(0), scrollTime(0), scrollDiff(0), originalScrollPix(0), 
	scrollBarWidth(15), PI(3.1415926), showScrollTrack(true),
	animationIsActive(false), refreshMs(60)
{

	loadFonts();

	timer = new Time();

}

OpenGLCanvas::~OpenGLCanvas()
{
	
}

void OpenGLCanvas::setUp2DCanvas()
{
	glMatrixMode (GL_PROJECTION);

	glLoadIdentity();
	glOrtho (0, 1, 1, 0, 0, 1);
	glMatrixMode (GL_MODELVIEW);
	
	glEnable(GL_TEXTURE_2D);
}

void OpenGLCanvas::activateAntiAliasing()
{
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void OpenGLCanvas::loadFonts()
{

	const unsigned char* buffer = reinterpret_cast<const unsigned char*>(BinaryData::misoregular_ttf);
	size_t bufferSize = BinaryData::misoregular_ttfSize;

	FTPixmapFont* font1 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font1);

	buffer = reinterpret_cast<const unsigned char*>(BinaryData::misobold_ttf);
	bufferSize = BinaryData::misobold_ttfSize;

	FTPixmapFont* font2 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font2);

	buffer = reinterpret_cast<const unsigned char*>(BinaryData::misolight_ttf);
	bufferSize = BinaryData::misolight_ttfSize;

	FTPixmapFont* font3 = new FTPixmapFont(buffer, bufferSize);
	
	fontList.add(font3);

}

FTPixmapFont* OpenGLCanvas::getFont(String fontName)
{
	
	if (fontName.equalsIgnoreCase("miso-regular"))
		return fontList[0];
	else if (fontName.equalsIgnoreCase("miso-bold"))
		return fontList[1];
	else if (fontName.equalsIgnoreCase("miso-light"))
		return fontList[2];
	else
		return fontList[0]; // miso-regular is default font

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


void OpenGLCanvas::drawScrollBars()
{
	float scrollBarY = float(getHeight())/float(getTotalHeight());
	float timeSinceScroll = timer->getMillisecondCounter()-scrollTime;
	
	if (scrollBarY < 1.0f && timeSinceScroll < 1300)
	{
		float alpha;

		if (timeSinceScroll < 1000)
			alpha = 1.0f;
		else
			alpha = 1.0f*(1-float(timeSinceScroll-1000)/300.0f);

		float Yoffset = float(scrollPix)/float(getTotalHeight());

		if (showScrollTrack)
			drawScrollBar(0.995f, 2.0f/getHeight(), alpha*0.2f);

		scrollBarBottom = scrollBarY + Yoffset - 2.0f/getHeight();
		scrollBarTop = Yoffset + 2.0f/getHeight();
		
		drawScrollBar(scrollBarBottom, scrollBarTop, alpha*0.5f);

	} else {
		if (!animationIsActive) {
			stopTimer(); 
		}
		showScrollTrack = false;
	}
}

void OpenGLCanvas::drawScrollBar(float y1, float y2, float alpha)
{
	glViewport(0,0,getWidth(),getHeight());

	float x1 = (getWidth()-8.0f)/getWidth();
	float x2 = (getWidth()-2.0f)/getWidth();
	//float px2 = 2.0f/getWidth();

	glColor4f(0.0f, 0.0f, 0.0f, alpha);

	glBegin(GL_POLYGON);

	glVertex2f(x1,y1);
	//glVertex2f(x1+px2,y1+px2);
	///glVertex2f(x1+px2*2,y1+px2);
	glVertex2f(x2,y1);
	glVertex2f(x2,y2);
	//glVertex2f(x2-px2,y2-px2);
	//glVertex2f(x2-px2*2,y2-px2);
	glVertex2f(x1,y2);

	glEnd();

}

void OpenGLCanvas::showScrollBars()
{
	scrollTime = timer->getMillisecondCounter();
	startTimer(refreshMs);
}

void OpenGLCanvas::drawRoundedRect(float x,
 						   		  float y,
 						   		  float w,
 						   		  float h,
 						   		  float r,
 						   		  int n)
{
	//glLineWidth(3.0);
	GLint* params = new GLint[4];

	glGetIntegerv(GL_VIEWPORT, params);

	//std::cout << params[0] << " " << params[1] << " " << params[2] << " "
	//	      << params[3] << std::endl;

	float ratio = float(params[3])/float(params[2]);

	//std::cout << ratio << std::endl;

	glBegin(GL_LINE_LOOP);
	
	for (int side = 0; side < 4; side++)
	{
		float x0[2];
		float y0[2];
		float origin[2];
		float angle[2];
		
		switch (side) {
			case 0:
				x0[0] = 0; x0[1] = 0;
				y0[0] = r; y0[1] = h-r;
				origin[0] = r*ratio; origin[1] = h-r;
				angle[0] = PI/2; angle[1] = PI;
				break;

			case 1:
				x0[0] = r*ratio; x0[1] = w-r*ratio;
				y0[0] = h; y0[1] = h;
				origin[0] = w-r*ratio; origin[1] = h-r;
				angle[0] = 0; angle[1] = PI/2;
				break;

			case 2:
				x0[0] = w; x0[1] = w;
				y0[0] = h-r; y0[1] = r;
				origin[0] = w-r*ratio; origin[1] = r;
				angle[0] = 3*PI/2; angle[1] = 2*PI;
				break;

			case 3:
				x0[0] = w-r*ratio; x0[1] = r*ratio;
				y0[0] = 0; y0[1] = 0;
				origin[0] = r*ratio; origin[1] = r;
				angle[0] = PI; angle[1] = 3*PI/2;
				break;

			default:
				break;
		}

		//glLineWidth(2.0);
		glVertex2f(x0[0]+x,y0[1]+y);
		glVertex2f(x0[1]+x,y0[1]+y);

		//glLineWidth(1.0);
		for (float a = angle[1]; a > angle[0]; a -= (PI/2)/n)
		{
			glVertex2f(cos(a)*r*ratio+origin[0]+x,
			           sin(a)*r+origin[1]+y);
		}

	}

	glEnd();

}

void OpenGLCanvas::mouseMoveInCanvas(const MouseEvent& e)
{
	if (getTotalHeight() > getHeight()) {
	Point<int> pos = e.getPosition();
	int xcoord = pos.getX();
	if (xcoord > getWidth() - scrollBarWidth)
	{
		showScrollTrack = true; showScrollBars();
	}
	}
}

void OpenGLCanvas::mouseDownInCanvas(const MouseEvent& e)
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
}

void OpenGLCanvas::mouseDragInCanvas(const MouseEvent& e)
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
	
			scrollTime = timer->getMillisecondCounter();
			showScrollTrack = true;
			repaint();
		} 
	}
	}
}

void OpenGLCanvas::mouseUpInCanvas(const MouseEvent& e)
{
	scrollDiff = 0;
}

void OpenGLCanvas::mouseWheelMoveInCanvas(const MouseEvent&e,
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

}

void OpenGLCanvas::canvasWasResized()
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (scrollPix + getHeight() > getTotalHeight() && getTotalHeight() > getHeight())
		scrollPix = getTotalHeight() - getHeight();
	else
		scrollPix = 0;

	showScrollBars();

}

void OpenGLCanvas::timerCallback()
{
	repaint();
}



void OpenGLCanvas::mouseDown(const MouseEvent& e) 
{
	mouseDownInCanvas(e);
}

void OpenGLCanvas::mouseDrag(const MouseEvent& e) {mouseDragInCanvas(e);}
void OpenGLCanvas::mouseMove(const MouseEvent& e) {mouseMoveInCanvas(e);}
void OpenGLCanvas::mouseUp(const MouseEvent& e) 	{mouseUpInCanvas(e);}
void OpenGLCanvas::mouseWheelMove(const MouseEvent& e, float a, float b) {mouseWheelMoveInCanvas(e,a,b);}

void OpenGLCanvas::resized()
{
	canvasWasResized();
}