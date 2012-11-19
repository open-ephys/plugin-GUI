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

#include "LfpDisplayCanvas.h"

#include <math.h>

LfpDisplayCanvas::LfpDisplayCanvas(LfpDisplayNode* n) : processor(n),
	 	xBuffer(105), yBuffer(2),
	    plotHeight(180), selectedChan(-1), screenBufferIndex(0),
	    timebase(1.0f), displayGain(0.0001f), displayBufferIndex(0),
	    headerHeight(40), plotOverlap(200), interplotDistance(70),
	    timeOffset(0.0f), footerHeight(0)
{

	nChans = processor->getNumInputs();
	sampleRate = processor->getSampleRate();
	std::cout << "Setting num inputs on LfpDisplayCanvas to " << nChans << std::endl;

	displayBuffer = processor->getDisplayBufferAddress();
	displayBufferSize = displayBuffer->getNumSamples();
	std::cout << "Setting displayBufferSize on LfpDisplayCanvas to " << displayBufferSize << std::endl;

	totalHeight = nChans*(interplotDistance) + plotHeight/2 + headerHeight;

	refreshMs = 100; // override 5 s refresh rate
	
}

LfpDisplayCanvas::~LfpDisplayCanvas()
{
}


void LfpDisplayCanvas::newOpenGLContextCreated()
{

	setUp2DCanvas();
	activateAntiAliasing();

	glClearColor (0.667, 0.698, 0.718, 1.0);
	resized();

}

void LfpDisplayCanvas::beginAnimation()
{
	std::cout << "Beginning animation." << std::endl;

	displayBufferSize = displayBuffer->getNumSamples();

	screenBufferIndex = 0;
	
	startCallbacks();
}

void LfpDisplayCanvas::endAnimation()
{
	std::cout << "Ending animation." << std::endl;
	stopCallbacks();
}

void LfpDisplayCanvas::update()
{
	nChans = processor->getNumInputs();
	sampleRate = processor->getSampleRate();

	std::cout << "Setting num inputs on LfpDisplayCanvas to " << nChans << std::endl;

	refreshScreenBuffer();

	repaint();

	totalHeight = nChans*(interplotDistance) + plotHeight/2 + headerHeight;//(plotHeight+yBuffer)*nChans + yBuffer + headerHeight;
}


void LfpDisplayCanvas::setParameter(int param, float val)
{
	if (param == 0) {
		timebase = val;
		refreshScreenBuffer();
	} else {
		displayGain = val * 0.0001f;
	}

	repaint();
	
}

void LfpDisplayCanvas::refreshState()
{
	// called when the component's tab becomes visible again
	displayBufferIndex = processor->getDisplayBufferIndex();
	screenBufferIndex = 0;

	//resized();

}

void LfpDisplayCanvas::refreshScreenBuffer()
{

	screenBufferIndex = 0;

	int w = getWidth(); 
	//std::cout << "Refreshing buffer size to " << w << "pixels." << std::endl;

	for (int i = 0; i < w; i++)
	{
		float x = float(i) / float(w);

		for (int n = 0; n < nChans; n++)
		{
			waves[n][i*2] = x;
			waves[n][i*2+1] = 0.5f; // line in center of display
		}
	}

}

void LfpDisplayCanvas::updateScreenBuffer()
{
	// copy new samples from the displayBuffer into the screenBuffer (waves)
	int maxSamples = getWidth();

	int index = processor->getDisplayBufferIndex();

	int nSamples = index - displayBufferIndex;

	if (nSamples < 0) // buffer has reset to 0
	{
		nSamples = (displayBufferSize - displayBufferIndex) + index;
	}

	float ratio = sampleRate * timebase / float(getWidth());

	// this number is crucial:
	int valuesNeeded = (int) float(nSamples) / ratio;

	float subSampleOffset = 0.0;
	int nextPos = (displayBufferIndex + 1) % displayBufferSize;

	if (valuesNeeded > 0 && valuesNeeded < 1000) {

	    for (int i = 0; i < valuesNeeded; i++)
	    {
	    	float gain = 1.0;
	    	float alpha = (float) subSampleOffset;
	    	float invAlpha = 1.0f - alpha;

	    	for (int channel = 0; channel < nChans; channel++) {

	        	waves[channel][screenBufferIndex*2+1] = 
	        		*(displayBuffer->getSampleData(channel, displayBufferIndex))*invAlpha*gain*displayGain;

	        	waves[channel][screenBufferIndex*2+1] += 
	        		*(displayBuffer->getSampleData(channel, nextPos))*alpha*gain*displayGain;

	        	waves[channel][screenBufferIndex*2+1] += 0.5f; // to center in viewport

	       	}

	       	//// now do the event channel
	       	waves[nChans][screenBufferIndex*2+1] = 
	       		*(displayBuffer->getSampleData(nChans, displayBufferIndex));


	       	subSampleOffset += ratio;

	       	while (subSampleOffset >= 1.0)
	       	{
	       		if (++displayBufferIndex >= displayBufferSize)
	       			displayBufferIndex = 0;
	       		
	       		nextPos = (displayBufferIndex + 1) % displayBufferSize;
	       		subSampleOffset -= 1.0;
	       	}

	       	screenBufferIndex++;
	       	screenBufferIndex %= maxSamples;

	    }

	} else {
		//std::cout << "Skip." << std::endl;
	}
}

void LfpDisplayCanvas::canvasWasResized()
{
	//std::cout << "Resized!" << std::endl;	
	refreshScreenBuffer();
}

void LfpDisplayCanvas::renderOpenGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear buffers to preset values

	
	//glClear(GL_COLOR_BUFFER_BIT); // clear buffers to preset values

    if (animationIsActive)
        updateScreenBuffer();

	for (int i = 0; i < nChans; i++)
	{
		bool isSelected = false;

		if (selectedChan == i)
			isSelected = true;

		if (checkBounds(i)) {
			//setViewport(i);
			//drawBorder(isSelected);
			drawWaveform(i,isSelected);
			drawChannelInfo(i,isSelected);
			
		}	
	}

	drawEvents();

	drawScrollBars();

	drawProgressBar();

	drawTimeline();
    
   // glFlush();
    //swapBuffers();
	
}

void LfpDisplayCanvas::drawEvents()
{

	//std::cout << waves[nChans][1] << std::endl;

	glViewport(xBuffer, 0, getWidth()-xBuffer, getHeight());

	glLineWidth(2.0f);
	

	// loop through events
	for (int n = 1; n < getWidth()*2; n += 2)
	{

		if (waves[nChans][n] > 0)
		{

			float x = (float(n-1)/2)/float(getWidth());

			int ttlState = int(waves[nChans][n]);
			//std::cout << x << std::endl;

			if ((ttlState & 0x100) >> 8) // channel 8
			{
				glColor4f(0.9, 0.9, 0.9, 0.4);
                
				glBegin(GL_LINE_STRIP);
                glVertex2f(x, 0);
                glVertex2f(x, 1);
				glEnd();
			}
            
            if ((ttlState & 0x80) >> 7) // channel 7
			{
				glColor4f(0.5, 0.3, 0.2, 0.1);
                
				glBegin(GL_LINE_STRIP);
                glVertex2f(x, 0);
                glVertex2f(x, 1);
				glEnd();
			}

            if ((ttlState & 0x40) >> 6) // channel 6
			{
				glColor4f(1.0, 0.3, 0.0, 0.1);
                
				glBegin(GL_LINE_STRIP);
                glVertex2f(x, 0);
                glVertex2f(x, 1);
				glEnd();
			}
            
			if ((ttlState & 0x20) >> 5) // channel 5
			{
				glColor4f(1.0, 0.0, 0.0, 0.1);

				glBegin(GL_LINE_STRIP);
					glVertex2f(x, 0);
					glVertex2f(x, 1);
				glEnd();
			}

			if ((ttlState & 0x10) >> 4) // channel 4
			{
				glColor4f(0.0, 1.0, 0.0, 0.1);

				glBegin(GL_LINE_STRIP);
					glVertex2f(x, 0);
					glVertex2f(x, 1);
				glEnd();

			}

			if ((ttlState & 0x8) >> 3) // channel 3
			{
				glColor4f(0.0, 0.0, 1.0, 0.1);

				glBegin(GL_LINE_STRIP);
					glVertex2f(x, 0);
					glVertex2f(x, 1);
				glEnd();

			}

			if ((ttlState & 0x4) >> 2) // channel 2
			{
				glColor4f(0.0, 1.0, 1.0, 0.1);

				glBegin(GL_LINE_STRIP);
					glVertex2f(x, 0);
					glVertex2f(x, 1);
				glEnd();

			}

			if ((ttlState & 0x2) >> 1) // channel 1
			{
				glColor4f(1.0, 1.0, 0.0, 0.1);

				glBegin(GL_LINE_STRIP);
					glVertex2f(x, 0);
					glVertex2f(x, 1);
				glEnd();

			}

			if ((ttlState & 0x1)) // channel 0
			{
				glColor4f(1.0, 1.0, 1.0, 0.1);

				glBegin(GL_LINE_STRIP);
					glVertex2f(x, 0);
					glVertex2f(x, 1);
				glEnd();

			}

		}

	}
}

void LfpDisplayCanvas::drawWaveform(int chan, bool isSelected)
{
	setViewport(chan);

	int w = getWidth();
	
	// draw zero line
	glColor4f(1.0, 1.0, 1.0, 0.2);
	glBegin(GL_LINE_STRIP);
	glVertex2f(0, 0.5);
	glVertex2f(1, 0.5);
	glEnd();


	// setWaveformColor(chan, isSelected);
	if (isSelected)
		glColor4f(1.0, 1.0, 1.0, 1.0);
	else
		glColor4f(1.0, 1.0, 1.0, 0.4);

	glEnableClientState(GL_VERTEX_ARRAY);
	
	glVertexPointer( 2,         // number of coordinates per vertex (2, 3, or 4)
	     			 GL_FLOAT,  // data type
		   			 0, 	    // byte offset between consecutive vertices
		   			 waves[chan]); // pointer to the first coordinate of the first vertex

    glDrawArrays(GL_LINE_STRIP, // mode
    			 0,				// starting index
    			 w);  // number of indices to be rendered
	
	glDisableClientState(GL_VERTEX_ARRAY);


}

void LfpDisplayCanvas::drawProgressBar()
{

	glViewport(xBuffer,0,getWidth()-xBuffer,getHeight());
	int w = getWidth();

	// color of progress bar
	glColor4f(1.0, 1.0, 0.1, 1.0);

	glBegin(GL_LINE_STRIP);
	glVertex2f(float(screenBufferIndex)/w,0);
	glVertex2f(float(screenBufferIndex)/w,1);
	glEnd();
}

void LfpDisplayCanvas::drawTimeline()
{
	
	glViewport(0,getHeight()-headerHeight,getWidth(),headerHeight);
	glColor4f(0.2f, 0.2f, 0.2f, 1.0f);
	glRectf(0,0,1,1);

	glColor4f(1.0f, 1.0f, 1.0f, 0.25f);

	String s = "TIME (s)";

	glRasterPos2f(5.0f/float(getWidth()), 0.7);

	getFont(cpmono_plain)->FaceSize(14);
	getFont(cpmono_plain)->Render(s);

	glViewport(xBuffer,getHeight()-headerHeight,getWidth()-xBuffer,headerHeight);

	float step;

	if (timebase < 1)
	{
		step = 0.1;
	} else if (timebase >= 1 && timebase < 2)
	{
		step = 0.2;
	} else if (timebase >= 2 && timebase < 5)
	{
		step = 0.5;
	} else {
		step = 1.0;
	}

	float currentPos = 0;
	glLineWidth(2.0);

	while (currentPos < timebase)
	{

		float xcoord = currentPos / timebase;

		glBegin(GL_LINE_STRIP);
		glVertex2f(xcoord,0);
		glVertex2f(xcoord,1);
		glEnd();

		String s = String(currentPos, 1);

		glRasterPos2f(xcoord + 5.0f/float(getWidth()), 0.4);

		getFont(cpmono_plain)->Render(s);

		currentPos += step;
	}

	glViewport(xBuffer, getHeight()-headerHeight, getWidth()-xBuffer, headerHeight/2);
	glColor4f(0.2f, 0.2f, 0.4f, 1.0f);
	glRectf(0,0,1,1);

	currentPos = 0;


	glColor4f(1.0f, 1.0f, 1.0f, 0.25f);

	while (currentPos < timebase)
	{

		float xcoord = currentPos/timebase + timeOffset / float(getWidth());

		glBegin(GL_LINE_STRIP);
		glVertex2f(xcoord,0);
		glVertex2f(xcoord,1);
		glEnd();

		String s = String(currentPos, 1);

		glRasterPos2f(xcoord+5.0f/float(getWidth()), 0.85);

		getFont(cpmono_plain)->Render(s);

		currentPos += step;
	}

}


bool LfpDisplayCanvas::checkBounds(int chan)
{
	bool isVisible;

	int lowerBound = (chan+1)*(interplotDistance)+plotHeight/2;//(chan+1)*(plotHeight+yBuffer);
	int upperBound = chan*(interplotDistance)-plotHeight/2;

	if (getScrollAmount() < lowerBound && getScrollAmount() + getHeight() > upperBound)
		isVisible = true;
	else
		isVisible = false;
	
	return isVisible;

}

void LfpDisplayCanvas::setViewport(int chan)
{
	int y = (chan+1)*(interplotDistance); //interplotDistance - plotHeight/2);

	glViewport(xBuffer,
			   getHeight()-y+getScrollAmount()- headerHeight - plotHeight/2,
	           getWidth()-xBuffer,
	           plotHeight);
}

void LfpDisplayCanvas::setInfoViewport(int chan)
{
	int y = (chan+1)*(interplotDistance); //interplotDistance - plotHeight/2);

	glViewport(yBuffer,
			   getHeight()-y+getScrollAmount()- headerHeight - interplotDistance/2 - yBuffer,
	           xBuffer-yBuffer,
	           interplotDistance - yBuffer*2);
}

void LfpDisplayCanvas::drawBorder(bool isSelected)
{
	float alpha = 0.5f;

	if (isSelected)
		alpha = 1.0f;

	glColor4f(0.0f, 0.0f, 0.0f, alpha);
	glBegin(GL_LINE_STRIP);
 	glVertex2f(0.0f, 0.0f);
 	glVertex2f(1.0f, 0.0f);
 	glVertex2f(1.0f, 1.0f);
 	glVertex2f(0.0f, 1.0f);
 	glVertex2f(0.0f, 0.0f);
 	glEnd();

}

void LfpDisplayCanvas::drawChannelInfo(int chan, bool isSelected)
{

	setInfoViewport(chan);
	drawBorder(isSelected);

	float alpha = 0.5f;

	if (isSelected)
		alpha = 1.0f;

	glColor4f(0.0f,0.0f,0.0f,alpha);
	glRasterPos2f(5.0f/getWidth(),0.6);
	String s = "";//String("Channel ");
	s += (chan+1);

	getFont(cpmono_bold)->FaceSize(35);
	getFont(cpmono_bold)->Render(s);
}

int LfpDisplayCanvas::getTotalHeight() 
{
	return totalHeight;
}


void LfpDisplayCanvas::mouseDownInCanvas(const MouseEvent& e) 
{

	Point<int> pos = e.getPosition();
	int xcoord = pos.getX();
	int ycoord = pos.getY();

	if (xcoord < getWidth()-getScrollBarWidth() && ycoord > headerHeight)
	{
		int ycoord = e.getMouseDownY() - headerHeight - interplotDistance/2;// - interplotDistance/2;// - interplotDistance;
		int chan = (ycoord + getScrollAmount())/(yBuffer+interplotDistance);

			selectedChan = chan;

		repaint();
	}

}

void LfpDisplayCanvas::mouseDragInCanvas(const MouseEvent& e) 
{

	int ypos = e.getMouseDownY();

	if (ypos <= headerHeight/2) {

		float scaleFactor = (float) e.getDistanceFromDragStartY();

		if (scaleFactor < 60.0 && scaleFactor > -200.0f)
		{
			timebase = pow(10.0f, -scaleFactor/200.0f);
		}

		repaint();

	} else if (ypos > headerHeight/2 && ypos < headerHeight) {

		float scaleFactor = (float) e.getDistanceFromDragStartX();

		timeOffset = scaleFactor;

		repaint();

	}


}

void LfpDisplayCanvas::mouseMoveInCanvas(const MouseEvent &e)
{

	int ypos = e.getMouseDownY();

	if (ypos <= headerHeight/2)
	{
		cursorType = MouseCursor::UpDownResizeCursor;
	} else if (ypos > headerHeight/2 && ypos < headerHeight) {
		cursorType = MouseCursor::LeftRightResizeCursor;
	} else {
		cursorType = MouseCursor::NormalCursor;
	}

}

const MouseCursor LfpDisplayCanvas::getMouseCursor()
{

	const MouseCursor c = MouseCursor(cursorType);

	return c;

}