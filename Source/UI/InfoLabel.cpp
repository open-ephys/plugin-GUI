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

#include "InfoLabel.h"

InfoLabel::InfoLabel() : xBuffer(10), yBuffer(10)
{
	
}

InfoLabel::~InfoLabel()
{

}


void InfoLabel::newOpenGLContextCreated()
{

	setUp2DCanvas();
	activateAntiAliasing();

	glClearColor (0.667, 0.698, 0.718, 1.0);
	resized();


}

void InfoLabel::renderOpenGL()
{
	//makeCurrentContextActive();

	glClear(GL_COLOR_BUFFER_BIT); // clear buffers to preset values

	drawLabel();

	drawScrollBars();

	//makeCurrentContextInactive();
}


void InfoLabel::drawLabel()
{
	
	glViewport(xBuffer,
		 	   yBuffer-getScrollAmount(),
		 	   getWidth()-2*xBuffer,
		 	   jmax(getHeight(),getTotalHeight())-2*yBuffer);

	float mult = 1/float(getWidth());

	glColor4f(0.5,0.5,0.5,0.6);

	glBegin(GL_LINE_STRIP);
	glVertex2f(0.1,0);
	glVertex2f(0.1,1.0);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex2f(0,0.1);
	glVertex2f(1.0,0.1);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex2f(0.9,0);
	glVertex2f(0.9,1.0);
	glEnd();

	glBegin(GL_LINE_STRIP);
	glVertex2f(0,0.9);
	glVertex2f(1.0,0.9);
	glEnd();

	glColor4f(0.5,0.5,0.5,0.8);

	getFont(String("miso-regular"))->FaceSize(12.0f);
	
	for (float x = 0.1f; x < 1.0f; x += 0.8f)
	{
		for (float y = 0.1f; y < 1.0f; y += 0.8f)
		{
			glRasterPos2f(x+0.005f,y+0.025f);
			String s = String("(0.");
			s += int(x*10);
			s += String(", 0.");
			s += int(y*10);
			s += String(")");
			getFont(String("miso-regular"))->Render(s);
		}
	}
}

void InfoLabel::resized() 
{

	//std::cout << getWidth() << " " << getHeight() 
	//		  << std::endl;

	canvasWasResized();

}

int InfoLabel::getTotalHeight() 
{
	return 300;
}

void InfoLabel::mouseDown(const MouseEvent& e) 
{
	// Point<int> pos = e.getPosition();
	// int xcoord = pos.getX();

	// if (xcoord < getWidth()-getScrollBarWidth())
	// {
	// 	int chan = (e.getMouseDownY() + getScrollAmount())/(yBuffer+plotHeight);

	// 	//if (chan == selectedChan)
	// 	//	selectedChan = -1;
	// 	//else
	// 		selectedChan = chan;

	// 	repaint();
	// }
	mouseDownInCanvas(e);
}

void InfoLabel::mouseDrag(const MouseEvent& e) {mouseDragInCanvas(e);}
void InfoLabel::mouseMove(const MouseEvent& e) {mouseMoveInCanvas(e);}
void InfoLabel::mouseUp(const MouseEvent& e) 	{mouseUpInCanvas(e);}
void InfoLabel::mouseWheelMove(const MouseEvent& e, float a, float b) {mouseWheelMoveInCanvas(e,a,b);}

