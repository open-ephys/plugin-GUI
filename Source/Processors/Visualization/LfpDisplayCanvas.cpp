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

LfpDisplayCanvas::LfpDisplayCanvas(LfpDisplayNode* n) : processor(n),
	 	xBuffer(105), yBuffer(2),
	    plotHeight(180), selectedChan(-1), screenBufferIndex(0),
	    timebase(1.0f), displayGain(0.0001f), displayBufferIndex(0),
	    headerHeight(20), plotOverlap(200), interplotDistance(70)
{

	nChans = processor->getNumInputs();
	sampleRate = processor->getSampleRate();
	std::cout << "Setting num inputs on LfpDisplayCanvas to " << nChans << std::endl;

	displayBuffer = processor->getDisplayBufferAddress();
	displayBufferSize = displayBuffer->getNumSamples();
	std::cout << "Setting displayBufferSize on LfpDisplayCanvas to " << displayBufferSize << std::endl;

	totalHeight = nChans*(interplotDistance) + plotHeight/2 + headerHeight;
	
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
	
	glClear(GL_COLOR_BUFFER_BIT); // clear buffers to preset values

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

	drawScrollBars();

	drawProgressBar();

	drawTicks();
	
}

void LfpDisplayCanvas::drawWaveform(int chan, bool isSelected)
{
	setViewport(chan);

	int w = getWidth();

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

void LfpDisplayCanvas::drawTicks()
{
	
	glViewport(0,getHeight()-headerHeight,getWidth(),headerHeight);
	glColor4f(0.2f, 0.2f, 0.2f, 1.0f);
	glRectf(0,0,1,1);

	glColor4f(1.0f, 1.0f, 1.0f, 0.25f);

	String s = "TIME (s)";

	glRasterPos2f(5.0f/float(getWidth()), 0.7);

	getFont(String("cpmono-plain"))->FaceSize(14);
	getFont(String("cpmono-plain"))->Render(s);

	glViewport(xBuffer,getHeight()-headerHeight,getWidth()-xBuffer,headerHeight);

	for (int i = 0; i < 10; i++)
	{
		if (i == 5)
			glLineWidth(3.0);
		else if (i == 1 || i == 3 || i == 7 || i == 9)
			glLineWidth(2.0);
		else
			glLineWidth(1.0);

		glBegin(GL_LINE_STRIP);
		glVertex2f(0.1*i,0);
		glVertex2f(0.1*i,1);
		glEnd();

		String s = String((timebase / 10)*i, 2);

		glRasterPos2f(0.1*i+5.0f/float(getWidth()), 0.7);

		getFont(String("cpmono-plain"))->Render(s);
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

	getFont(String("cpmono-bold"))->FaceSize(35);
	getFont(String("cpmono-bold"))->Render(s);
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
