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

#include "LfpDisplayCanvas.h"

#include <math.h>

LfpDisplayCanvas::LfpDisplayCanvas(LfpDisplayNode* processor_) :
	timebase(1.0f), displayGain(2.f), timeOffset(0.0f), processor(processor_),
	screenBufferIndex(0), displayBufferIndex(0)
{

	nChans = processor->getNumInputs();
	sampleRate = processor->getSampleRate();
	std::cout << "Setting num inputs on LfpDisplayCanvas to " << nChans << std::endl;

	displayBuffer = processor->getDisplayBufferAddress();
	displayBufferSize = displayBuffer->getNumSamples();
	std::cout << "Setting displayBufferSize on LfpDisplayCanvas to " << displayBufferSize << std::endl;

}

LfpDisplayCanvas::~LfpDisplayCanvas()
{
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

}


void LfpDisplayCanvas::setParameter(int param, float val)
{
	if (param == 0) {
		timebase = val;
		refreshScreenBuffer();
	} else {
		displayGain = val; //* 0.0001f;
	}

	repaint();
}

void LfpDisplayCanvas::refreshState()
{
	// called when the component's tab becomes visible again
	displayBufferIndex = processor->getDisplayBufferIndex();
	screenBufferIndex = 0;

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

				gain = -1.0f / (processor->channels[channel]->bitVolts * float(0x7fff));
	        	waves[channel][screenBufferIndex*2+1] = 
	        		*(displayBuffer->getSampleData(channel, displayBufferIndex))*invAlpha*gain*displayGain;

	        	waves[channel][screenBufferIndex*2+1] += 
	        		*(displayBuffer->getSampleData(channel, nextPos))*alpha*gain*displayGain;

	        	waves[channel][screenBufferIndex*2+1] += 0.5f; // to center in viewport

	       	}

	       	//// now do the event channel
	       ////	waves[nChans][screenBufferIndex*2+1] = 
	       //		*(displayBuffer->getSampleData(nChans, displayBufferIndex));


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

void LfpDisplayCanvas::paint(Graphics& g)
{


	g.fillAll(Colours::magenta);
	
	g.setColour(Colours::white);

	g.drawLine(0,0, getWidth(), getHeight());
	g.drawLine(0,getHeight(),getWidth(), 0);
	
}
