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

#include "SpikeDisplayCanvas.h"

SpikeDisplayCanvas::SpikeDisplayCanvas(SpikeDisplayNode* n) : processor(n),
	 	xBuffer(0), yBuffer(0),  newSpike(false)
{

	

	nSources = 0; //processor->getNumInputs();
	std::cout<<"SpikeDisplayNode has :"<<nSources<<" outputs!"<<std::endl;
	
	//memset(nChannels, 0, sizeof(nChannels[0]) * MAX_NUMBER_OF_SPIKE_SOURCES);
	for (int i=0; i<nSources; i++)
		nChannels[i] = processor->getNumberOfChannelsForInput(i);

	// sampleRate = processor->getSampleRate();
	std::cout << "Setting num inputs on SpikeDisplayCanvas to " << nSources << std::endl;

	//generateEmptySpike(&spike, 1);
	
	initializeSpikePlots();
	
	// displayBuffer = processor->getDisplayBufferAddress();
	// displayBufferSize = displayBuffer->getNumSamples();
	// std::cout << "Setting displayBufferSize on SpikeDisplayCanvas to " << displayBufferSize << std::endl;


	// totalHeight = (plotHeight+yBuffer)*nChans + yBuffer;

	// screenBuffer = new AudioSampleBuffer(nChans, 10000);	
}

SpikeDisplayCanvas::~SpikeDisplayCanvas()
{
	
}

void SpikeDisplayCanvas::initializeSpikePlots(){
	std::cout<<"Initializing Plots"<<std::endl;
	int xPadding = 10;
	int yPadding = 10;

	int nPlots = 4;

	int totalWidth = 900; // This is a hack the width as the width isn't known before its drawn
	
	int plotWidth =  (totalWidth  - (nPlots + 1 ) * xPadding) / nPlots;
	int plotHeight = plotWidth / 2;
	for (int i=0; i<nPlots; i++)
	{
		StereotrodePlot p = StereotrodePlot( xPadding + i * (plotWidth + xPadding) , yPadding, plotWidth, plotHeight, "");
		plots.push_back(p);
	}
}



void SpikeDisplayCanvas::newOpenGLContextCreated()
{

	std::cout<<"SpikeDisplayCanvas::newOpenGLContextCreated()"<<std::endl;
	setUp2DCanvas();
	//activateAntiAliasing();

	glClearColor (0.667, 0.698, 0.718, 1.0);
	resized();
	endAnimation();
	//startTimer(50);

}

void SpikeDisplayCanvas::beginAnimation()
{
	std::cout << "Beginning animation." << std::endl;

	// displayBufferSize = displayBuffer->getNumSamples();

	// screenBuffer->clear();

	//displayBufferIndex = 0;
//	screenBufferIndex = 0;
	
	startCallbacks();
}

void SpikeDisplayCanvas::endAnimation()
{
	std::cout << "Ending animation." << std::endl;
	stopCallbacks();
}

void SpikeDisplayCanvas::update()
{
	// nChans = processor->getNumInputs();
	// sampleRate = processor->getSampleRate();

	// std::cout << "Setting num inputs on SpikeDisplayCanvas to " << nChans << std::endl;
	// if (nChans < 200 && nChans > 0)
	// 	screenBuffer->setSize(nChans, 10000);
	// //sampleRate = processor->getSampleRate();

    // screenBuffer->clear();

	repaint();

	// totalHeight = (plotHeight+yBuffer)*nChans + yBuffer;
}


void SpikeDisplayCanvas::setParameter(int param, float val)
{
	// if (param == 0)
	// 	timebase = val;
	// else
	// 	displayGain = val;
	
}

void SpikeDisplayCanvas::refreshState()
{
	// called when the component's tab becomes visible again
	// displayBufferIndex = processor->getDisplayBufferIndex();
	// screenBufferIndex = 0;

	//resized();

}

void SpikeDisplayCanvas::canvasWasResized()
{
	//std::cout << "Resized!" << std::endl;	
}

void SpikeDisplayCanvas::renderOpenGL()
{
	glClear(GL_COLOR_BUFFER_BIT); // clear buffers to preset values
	std::cout<<"SpikeDisplayCanvas::renderOpenGL"<<std::endl;


	// Get Spikes from the processor
	// Iterate through each spike, passing them individually to the appropriate plots and calling redraw before moving on to the next spike
	
	//while(processor->getNextSpike(&spike))
	//{
		
		// Identify which plot the spike should go to
			
		// Distribute those spike to the appropriate plot object
	
	SpikeObject tmpSpike;
	for (int i=0; i<plots.size(); i++){
		generateSimulatedSpike(&tmpSpike, 0, 100);
		plots[i].processSpikeObject(tmpSpike);
		plots[i].redraw();
	}
	
	//}
	//std::cout << getHeight()<<" "<< getTotalHeight()<<" "<<std::endl;
 	drawScrollBars();
}

void SpikeDisplayCanvas::drawTicks()
{
	
}

int SpikeDisplayCanvas::getTotalHeight() 
{
	return totalHeight;
}


void SpikeDisplayCanvas::mouseDownInCanvas(const MouseEvent& e) 
{

	// Point<int> pos = e.getPosition();
	// int xcoord = pos.getX();

	// if (xcoord < getWidth()-getScrollBarWidth())
	// {
	// 	int chan = (e.getMouseDownY() + getScrollAmount())/(yBuffer+plotHeight);

	// 		selectedChan = chan;

	// 	repaint();
	// }

}

// void SpikeDisplayCanvas::mouseDrag(const MouseEvent& e) {mouseDragInCanvas(e);}
// void SpikeDisplayCanvas::mouseMove(const MouseEvent& e) {mouseMoveInCanvas(e);}
// void SpikeDisplayCanvas::mouseUp(const MouseEvent& e) 	{mouseUpInCanvas(e);}
// void SpikeDisplayCanvas::mouseWheelMove(const MouseEvent& e, float a, float b) {mouseWheelMoveInCanvas(e,a,b);}

// void SpikeDisplayCanvas::resized()
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