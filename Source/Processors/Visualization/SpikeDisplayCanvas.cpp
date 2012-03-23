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
	 	xBuffer(25), yBuffer(25),  newSpike(false), plotsInitialized(false)
{

	

	nSources = 0; //processor->getNumInputs();
	std::cout<<"SpikeDisplayNode has :"<<nSources<<" outputs!"<<std::endl;
	
	for (int i=0; i<nSources; i++)
		nChannels[i] = processor->getNumberOfChannelsForInput(i);

	std::cout << "Setting num inputs on SpikeDisplayCanvas to " << nSources << std::endl;
	
	
	
}

SpikeDisplayCanvas::~SpikeDisplayCanvas()
{
	
}

void SpikeDisplayCanvas::initializeSpikePlots(){
	std::cout<<"Initializing Plots"<<std::endl;


	int nPlots = 4;
	int nCols = 2;

	int totalWidth = getWidth(); // This is a hack the width as the width isn't known before its drawn
	
	int plotWidth =  (totalWidth - yBuffer * ( nCols+1)) / nCols + .99;
	int plotHeight = plotWidth / 2 + .5;
	int rowCount = 0;

	for (int i=0; i<nPlots; i++)
	{

		StereotrodePlot p = StereotrodePlot( 
									xBuffer + i%nCols * (plotWidth + xBuffer) , 
									yBuffer + rowCount * (plotHeight + yBuffer), 
									plotWidth, 
									plotHeight); // deprecated conversion from string constant to char
		SpikeObject tmpSpike;
		generateEmptySpike(&tmpSpike, 4);
		p.processSpikeObject(tmpSpike);
		
		plots.push_back(p);
		if (i%nCols == nCols-1)
			rowCount++;


	
	 }
	// Set the total height of the Canvas to the top of the top most plot
	totalHeight = yBuffer + (rowCount + 1) * (plotHeight + yBuffer);
	plotsInitialized = true;
	//repositionSpikePlots();
}

void SpikeDisplayCanvas::repositionSpikePlots(){
	int nPlots = 4;
	int nCols = 2;

	int totalWidth = getWidth(); // This is a hack the width as the width isn't known before its drawn
	
	int plotWidth =  (totalWidth - yBuffer * ( nCols+1)) / nCols + .99;
	int plotHeight = plotWidth / 2 + .5;
	int rowCount = 0;

	for (int i=0; i<nPlots; i++)
	{

		plots[i].setPosition(	xBuffer + i%nCols * (plotWidth + xBuffer) , 
								yBuffer + rowCount * (plotHeight + yBuffer), 
								plotWidth, 
								plotHeight); // deprecated conversion from string constant to char
		if (i%nCols == nCols-1)
			rowCount++;	
	 }
	// Set the total height of the Canvas to the top of the top most plot
	totalHeight = yBuffer + (rowCount + 1) * (plotHeight + yBuffer);
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
	//std::cout<<"Got Param:"<< param<< " with value:"<<val<<std::endl;
	switch (param)
	{
		case SPIKE_CMD_CLEAR_ALL :
			for (int i=0; i<plots.size(); i++)
				plots[i].clear();
			break;
	
		case SPIKE_CMD_CLEAR_SEL:
		//clear plot number val
			break;
		default:
			std::cout<<"Unkown Commad specified! "<<param<<std::endl;
	}

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
	if(!plotsInitialized)
			initializeSpikePlots();
	glClearColor (0.667, 0.698, 0.718, 1.0);
	glClear(GL_COLOR_BUFFER_BIT); // clear buffers to preset values
//	std::cout<<"SpikeDisplayCanvas::renderOpenGL"<<std::endl;
	// Get Spikes from the processor
	// Iterate through each spike, passing them individually to the appropriate plots and calling redraw before moving on to the next spike
	
	//while(processor->getNextSpike(&spike))
	//{
		
	// Identify which plot the spike should go to
	
	// Distribute those spike to the appropriate plot object
	
	SpikeObject tmpSpike;
	 for (int i=0; i<plots.size(); i++){
		generateSimulatedSpike(&tmpSpike, 0, 150);
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

	/*
	Point<int> pos = e.getPosition();
	int xcoord = pos.getX();

	if (xcoord < getWidth()-getScrollBarWidth())
	{
		int chan = (e.getMouseDownY() + getScrollAmount())/(yBuffer+plotHeight);

			selectedChan = chan;

		repaint();
	}*/

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