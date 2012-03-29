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
	 	xBuffer(25), yBuffer(25),  newSpike(false), plotsInitialized(false),
	 	totalScrollPix(0)
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


	int nPlots = 6;
	int nCols = 2;

	int totalWidth = getWidth(); 
	
	int plotWidth =  (totalWidth - yBuffer * ( nCols+1)) / nCols + .99;
	int plotHeight = plotWidth / 2 + .5;
	int rowCount = 0;
	int i;

	for (i=0; i<nPlots; i++)
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
	totalHeight = yBuffer + rowCount * (plotHeight + yBuffer) + yBuffer;

	// Set the total height of the Canvas to the top of the top most plot
	plotsInitialized = true;
	repositionSpikePlots();
}

void SpikeDisplayCanvas::repositionSpikePlots(){
	
	int nPlots = plots.size();
	int nCols = 2;

	int totalWidth = getWidth(); 
	
	int plotWidth =  (totalWidth - yBuffer * ( nCols+1)) / nCols + .99;
	int plotHeight = plotWidth / 2 + .5;
	int rowCount = 0;

	for (int i=0; i<plots.size(); i++)
	{

		plots[i].setPosition(	xBuffer + i%nCols * (plotWidth + xBuffer) , 
								yBuffer + rowCount * (plotHeight + yBuffer), 
								plotWidth, 
								plotHeight); // deprecated conversion from string constant to char

		if (i%nCols == nCols-1)
			rowCount++;	
	 }

	// Set the total height of the Canvas to the top of the top most plot
	totalHeight = yBuffer + rowCount * (plotHeight + yBuffer) + yBuffer;
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


// Used for Plot specific commands, this commands target a specific PLOT and have
// no additional information, IE PARAM-> CLEAR  val->plot6  
// for more complex messages use the other version of setParameter
void SpikeDisplayCanvas::setParameter(int param, float val)
{
	std::cout<<"Got Param:"<< param<< " with value:"<<val<<std::endl;
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

void SpikeDisplayCanvas::setParameter(int param, int p2, int p3, float value){
	std::cout<<"Got Parameter:"<<param<<" p2:"<<p2<<" p3:"<<p3<<" value:"<<value<<std::endl;
	switch (param){
		case SPIKE_CMD_PAN_AXES:
			panPlot(p2, p3, value<=0);
			break;
		case SPIKE_CMD_ZOOM_AXES:
			zoomPlot(p2, p3, value<=0);
			break;
	}
}


void SpikeDisplayCanvas::refreshState()
{
	// called when the component's tab becomes visible again
	// displayBufferIndex = processor->getDisplayBufferIndex();
	// screenBufferIndex = 0;

	//resized();
	totalScrollPix = 0;
}

void SpikeDisplayCanvas::canvasWasResized()
{
	repositionSpikePlots();
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
 		drawPlotTitle( i );
	 }
	
	//}
	//std::cout << getHeight()<<" "<< getTotalHeight()<<" "<<std::endl;
 	drawScrollBars();
}

void SpikeDisplayCanvas::drawPlotTitle(int chan){

	glViewport(0,0,getWidth(), getHeight());
	setViewportRange(0, 0, getWidth(), getHeight());

	int x, y;
	double w,h;
 	plots[chan].getPosition(&x,&y,&w,&h);

	float alpha = 0.50f;

	glColor4f(0.0f,0.0f,0.0f,alpha);
	glRasterPos2f(x, y+h+2);
	
	String s = "Source:";//String("Channel ");
	s += (chan+1);

	getFont(String("cpmono-bold"))->FaceSize(25);
	getFont(String("cpmono-bold"))->Render(s);
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
void SpikeDisplayCanvas::mouseUp(const MouseEvent& e) 	{
	// std::cout<<"Mouse Event!"<<std::endl;

	// bool inout = false;

	// if (e.getMouseDownX() < getWidth()/2)
	// 	inout = false;
	// else
	// 	inout = true;

	// if (e.getMouseDownY() < getHeight()/2)
	// 	zoomPlot(0,0, inout);
	// else
	// 	panPlot(0,0,inout);
}
void SpikeDisplayCanvas::mouseWheelMove(const MouseEvent& e, float wheelIncrementX, float wheelIncrementY) {

	// std::cout<<"Mouse Wheel Move:"<< wheelIncrementX<<","<<wheelIncrementY;
	// std::cout<<" Scroll Pix:"<<scrollPix<<std::endl;

	int scrollAmount = 0;
	// std::cout<<getTotalHeight()<<" "<<getHeight()<<std::endl;

	if (getTotalHeight() > getHeight()) {
		//if (wheelIncrementY > 0 )
			scrollAmount += int(100.0f*wheelIncrementY);	
		//else if (wheelIncrementY < 0)
			
		totalScrollPix += scrollAmount;
		
		// don't let the user scroll too far down
		int minScrollDown = (-1 * totalHeight) + getHeight();
		int maxScrollUp = 0; // never scroll plots up, there is nothing below the bottom plot

		// std::cout<<"TotalScrollPix:"<<totalScrollPix<<" min:"<<minScrollDown<<" max:"<<maxScrollUp<<std::endl;
		
		if (totalScrollPix < minScrollDown){
			totalScrollPix= minScrollDown;
			scrollAmount = 0;
		}
		
		else if (totalScrollPix > maxScrollUp)
		{
			totalScrollPix = maxScrollUp;
			scrollAmount = 0;
		}

		for (int i=0; i<plots.size(); i++){
			int x,y;
			double w,h;
			plots[i].getPosition(&x, &y, &w, &h);
			plots[i].setPosition(x,y+scrollAmount, w, h);
		}

		scrollPix = 0;//totalScrollPix;

		repaint();

		showScrollBars();

	}

	mouseWheelMoveInCanvas(e, wheelIncrementX, wheelIncrementY);
}

void SpikeDisplayCanvas::panPlot(int p, int c, bool up){
	std::cout<<"SpikeDisplayCanvas::panPlot()"<<std::endl;
	if (p<0 || p>plots.size())
		return;
	plots[p].pan(c, up);

}
void SpikeDisplayCanvas::zoomPlot(int p, int c, bool in){
	std::cout<<"SpikeDisplayCanvas::panPlot()"<<std::endl;
	if (p<0 || p>plots.size())
		return;
	plots[p].zoom(c, in);
}

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