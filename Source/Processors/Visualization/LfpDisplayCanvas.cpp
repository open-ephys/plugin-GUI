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
	 	xBuffer(0), yBuffer(0),
	    plotHeight(40), selectedChan(-1), screenBufferIndex(0),
	    timebase(1.0f), displayGain(5.0f), displayBufferIndex(0)
{

	//GenericProcessor* gp = (GenericProcessor*) editor->getProcessor();


	nChans = processor->getNumInputs();
	sampleRate = processor->getSampleRate();
	std::cout << "Setting num inputs on LfpDisplayCanvas to " << nChans << std::endl;

	displayBuffer = processor->getDisplayBufferAddress();
	displayBufferSize = displayBuffer->getNumSamples();
		std::cout << "Setting displayBufferSize on LfpDisplayCanvas to " << displayBufferSize << std::endl;


	totalHeight = (plotHeight+yBuffer)*nChans + yBuffer;

	screenBuffer = new AudioSampleBuffer(nChans, 10000);
	
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


	//startTimer(50);

}

void LfpDisplayCanvas::beginAnimation()
{
	std::cout << "Beginning animation." << std::endl;

	displayBufferSize = displayBuffer->getNumSamples();

	screenBuffer->clear();

	//displayBufferIndex = 0;
	screenBufferIndex = 0;
	
	startCallbacks();
}

void LfpDisplayCanvas::endAnimation()
{
	std::cout << "Ending animation." << std::endl;
	stopCallbacks();
}

void LfpDisplayCanvas::updateNumInputs(int n)
{
	std::cout << "Setting num inputs on LfpDisplayCanvas to " << n << std::endl;
	nChans = n;
	if (n < 200)
		screenBuffer->setSize(nChans, 10000);
	//sampleRate = processor->getSampleRate();
}

void LfpDisplayCanvas::updateSampleRate(float r)
{
	sampleRate = r;
	displayBufferSize = displayBuffer->getNumSamples();
	std::cout << "Display canvas updating sample rate to " << r << std::endl;
}

void LfpDisplayCanvas::setParameter(int param, float val)
{
	if (param == 0)
		timebase = val;
	else
		displayGain = val;
	
}

void LfpDisplayCanvas::refreshState()
{
	// called when the component's tab becomes visible again
	displayBufferIndex = processor->getDisplayBufferIndex();
	screenBufferIndex = 0;

}

void LfpDisplayCanvas::updateScreenBuffer()
{
	// copy new samples from the displayBuffer into the screenBuffer
	int maxSamples = getWidth();

	int index = processor->getDisplayBufferIndex();

	//std::cout << index << screenBufferIndex << std::endl;

	int nSamples = index - displayBufferIndex;

	if (nSamples < 0)
	{
		nSamples = (displayBufferSize - displayBufferIndex) + index;
	}

	float ratio = sampleRate * timebase / float(getWidth());

	// this number is crucial:
	int valuesNeeded = (int) float(nSamples) / ratio;

	//lock->enterRead();
	float subSampleOffset = 0.0;
	int nextPos = (displayBufferIndex + 1) % displayBufferSize;
	
	//int screenBufferPos; 

	if (valuesNeeded > 0 && valuesNeeded < 1000) {

		int maxVal = screenBufferIndex + valuesNeeded;
		int overflow = maxVal - maxSamples;

		screenBuffer->clear(screenBufferIndex, valuesNeeded);

		if (overflow > 0)
			screenBuffer->clear(0, overflow);

	    for (int i = 0; i < valuesNeeded; i++)
	    {
	    	float gain = 1.0;
	    	float alpha = (float) subSampleOffset;
	    	float invAlpha = 1.0f - alpha;

	        for (int channel = 0; channel < displayBuffer->getNumChannels(); channel++) {

	        	screenBuffer->addFrom(channel,
	        						  screenBufferIndex,
	        						  *displayBuffer,
	        						  channel,
	        						  displayBufferIndex,
	        						  1,
	        						  invAlpha*gain*displayGain);
	        	
	        	screenBuffer->addFrom(channel,
	        						  screenBufferIndex,
	        						  *displayBuffer,
	        						  channel,
	        						  nextPos,
	        						  1,
	        						  alpha*gain*displayGain);
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


void LfpDisplayCanvas::renderOpenGL()
{
	
	glClear(GL_COLOR_BUFFER_BIT); // clear buffers to preset values

	//drawTicks();

	updateScreenBuffer();

	for (int i = 0; i < nChans; i++)
	{
		bool isSelected = false;

		if (selectedChan == i)
			isSelected = true;

		if (checkBounds(i)) {
			setViewport(i);
			//drawBorder(isSelected);
			drawChannelInfo(i,isSelected);
			drawWaveform(i,isSelected);
		}	
	}
	drawScrollBars();

	//std::cout << "Render." << std::endl;
}

void LfpDisplayCanvas::drawWaveform(int chan, bool isSelected)
{
	// draw the screen buffer for a given channel

	float w = float(getWidth());

	glBegin(GL_LINE_STRIP);

	for (float i = 0; i < float(getWidth()); i++)
	{
		glVertex2f(i/w,*screenBuffer->getSampleData(chan, int(i))+0.5);
	}

	glEnd();

	glColor4f(1.0, 1.0, 0.1, 1.0);
	glBegin(GL_LINE_STRIP);
	glVertex2f(float(screenBufferIndex)/w,0);
	glVertex2f(float(screenBufferIndex)/w,1);
	glEnd();

}


void LfpDisplayCanvas::drawTicks()
{
	
	glViewport(0,0,getWidth(),getHeight());

	glColor4f(1.0f, 1.0f, 1.0f, 0.25f);

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
	}
}


bool LfpDisplayCanvas::checkBounds(int chan)
{
	bool isVisible;

	int lowerBound = (chan+1)*(plotHeight+yBuffer);
	int upperBound = chan*(plotHeight+yBuffer);

	if (getScrollAmount() < lowerBound && getScrollAmount() + getHeight() > upperBound)
		isVisible = true;
	else
		isVisible = false;
	
	return isVisible;

}

void LfpDisplayCanvas::setViewport(int chan)
{
	glViewport(xBuffer,
			   getHeight()-(chan+1)*(plotHeight+yBuffer)+getScrollAmount(),
	           getWidth()-2*xBuffer,
	           plotHeight);
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
	float alpha = 0.5f;

	if (isSelected)
		alpha = 1.0f;

	glColor4f(0.0f,0.0f,0.0f,alpha);
	glRasterPos2f(5.0f/getWidth(),0.9);
	String s = "";//String("Channel ");
	s += (chan+1);

	getFont(String("cpmono-bold"))->FaceSize(35);
	getFont(String("cpmono-bold"))->Render(s);
}

int LfpDisplayCanvas::getTotalHeight() 
{
	return totalHeight;
}


void LfpDisplayCanvas::mouseDown(const MouseEvent& e) 
{

	Point<int> pos = e.getPosition();
	int xcoord = pos.getX();

	if (xcoord < getWidth()-getScrollBarWidth())
	{
		int chan = (e.getMouseDownY() + getScrollAmount())/(yBuffer+plotHeight);

			selectedChan = chan;

		repaint();
	}

	mouseDownInCanvas(e);
}

// void LfpDisplayCanvas::mouseDrag(const MouseEvent& e) {mouseDragInCanvas(e);}
// void LfpDisplayCanvas::mouseMove(const MouseEvent& e) {mouseMoveInCanvas(e);}
// void LfpDisplayCanvas::mouseUp(const MouseEvent& e) 	{mouseUpInCanvas(e);}
// void LfpDisplayCanvas::mouseWheelMove(const MouseEvent& e, float a, float b) {mouseWheelMoveInCanvas(e,a,b);}

// void LfpDisplayCanvas::resized()
// {
// 	//screenBuffer = new AudioSampleBuffer(nChans, getWidth());

// 	// glClear(GL_COLOR_BUFFER_BIT);

// 	// //int h = getParentComponent()->getHeight();

// 	// if (scrollPix + getHeight() > getTotalHeight() && getTotalHeight() > getHeight())
// 	// 	scrollPix = getTotalHeight() - getHeight();
// 	// else
// 	// 	scrollPix = 0;

// 	// showScrollBars();
// 	canvasWasResized();
// }