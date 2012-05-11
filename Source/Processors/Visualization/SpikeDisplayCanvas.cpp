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
	 	xBuffer(25), yBuffer(25), newSpike(false), plotsInitialized(false),
	 	totalScrollPix(0)
{

	nCols = 3;

	update();
	
	spikeBuffer = processor->getSpikeBufferAddress();
	// std::cout<<"SpikeDisplayNode has :"<<nPlots<<" outputs!"<<std::endl;
	
	// // for (int i=0; i<nPlots; i++)
	// // 	nChannels[i] = processor->getNumberOfChannelsForElectrode(i);

	// std::cout << "Setting num inputs on SpikeDisplayCanvas to " << nPlots << std::endl;
	
	
	
}

SpikeDisplayCanvas::~SpikeDisplayCanvas()
{
	
}

void SpikeDisplayCanvas::initializeSpikePlots(){

	std::cout<<"Initializing Plots"<<std::endl;

	if (plots.size() != nPlots)
	{

	int totalWidth = getWidth(); 
	
	int plotWidth =  (totalWidth - yBuffer * ( nCols+1)) / nCols + .99;
	int plotHeight = plotWidth / 2 + .5;
	int rowCount = 0;

	plots.clear();

	for (int i = 0; i < nPlots; i++)
	{

		switch (processor->getNumberOfChannelsForElectrode(i))
		{
			case 1:
			{

				std::cout << "Creating single electrode plot." << std::endl;

				ElectrodePlot* p1 = new ElectrodePlot( 
									xBuffer + i%nCols * (plotWidth + xBuffer) , 
									yBuffer + rowCount * (plotHeight + yBuffer), 
									plotWidth, 
									plotHeight);
				plots.add(p1);
				break;
			}
			case 2:
			{

				std::cout << "Creating stereotrode plot." << std::endl;

				StereotrodePlot* p2 = new StereotrodePlot( 
									xBuffer + i%nCols * (plotWidth + xBuffer) , 
									yBuffer + rowCount * (plotHeight + yBuffer), 
									plotWidth, 
									plotHeight);
				plots.add(p2);
				break;
			}
			case 4:
			{
				std::cout << "Creating tetrode plot." << std::endl;

				TetrodePlot* p3 = new TetrodePlot( 
									xBuffer + i%nCols * (plotWidth + xBuffer) , 
									yBuffer + rowCount * (plotHeight + yBuffer), 
									plotWidth, 
									plotHeight);
				plots.add(p3);
				break;
			}
			default:
			{

				std::cout << "Not sure what to do, creating single electrode plot." << std::endl;

				ElectrodePlot* p4 = new ElectrodePlot( 
									xBuffer + i%nCols * (plotWidth + xBuffer) , 
									yBuffer + rowCount * (plotHeight + yBuffer), 
									plotWidth, 
									plotHeight);
				plots.add(p4);
			}
		}

		// SpikeObject tmpSpike;
		// generateEmptySpike(&tmpSpike, 4);
		// p.processSpikeObject(tmpSpike);
		
		// plots.push_back(p);

		if (i%nCols == nCols-1)
			rowCount++;

	}

	//totalHeight = rowCount * (plotHeight + yBuffer) + yBuffer * 2;

	// Set the total height of the Canvas to the top of the top most plot
	plotsInitialized = true;

	repositionSpikePlots();
	}
}

void SpikeDisplayCanvas::repositionSpikePlots(){
	
	int totalWidth = getWidth(); 
	
	int plotWidth =  (totalWidth - yBuffer * ( nCols+1)) / nCols + .99;
	int plotHeight = plotWidth / 2 + .5;
	int rowCount = 0;

	for (int i=0; i < plots.size(); i++)
	{

		plots[i]->setPosition(	xBuffer + i%nCols * (plotWidth + xBuffer) , 
								getHeight() - ( yBuffer + plotHeight + rowCount * (plotHeight + yBuffer)) + getScrollAmount(), 
								plotWidth, 
								plotHeight); // deprecated conversion from string constant to char

		if (i%nCols == nCols-1)
			rowCount++;	
	 }

	// Set the total height of the Canvas to the top of the top most plot
	totalHeight = (rowCount + 1) * (plotHeight + yBuffer) + yBuffer;
}

void SpikeDisplayCanvas::newOpenGLContextCreated()
{
	std::cout<<"SpikeDisplayCanvas::newOpenGLContextCreated()"<<std::endl;
	setUp2DCanvas();
	activateAntiAliasing();
	disablePointSmoothing();

	glClearColor (0.667, 0.698, 0.718, 1.0);
	resized();
	//endAnimation();
}

void SpikeDisplayCanvas::beginAnimation()
{
	std::cout << "Beginning animation." << std::endl;
	
	startCallbacks();
}

void SpikeDisplayCanvas::endAnimation()
{
	std::cout << "Ending animation." << std::endl;
	stopCallbacks();
}

void SpikeDisplayCanvas::update()
{

	std::cout << "UPDATING SpikeDisplayCanvas" << std::endl;

	nPlots = processor->getNumElectrodes();
	// numChannelsPerPlot.clear();

	// for (int i = 0; i < nPlots; i++)
	// {
	// 	numChannelsPerPlot.add(processor->getNumberOfChannelsForElectrode(i));
	// }

	initializeSpikePlots();

	repaint();
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
			for (int i=0; i < plots.size(); i++)
				plots[i]->clear();
			break;
	
		case SPIKE_CMD_CLEAR_SEL:
		//clear plot number val
			break;
		default:
			std::cout<<"Unknown command specified! "<<param<<std::endl;
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
	//totalScrollPix = 0;
}

void SpikeDisplayCanvas::canvasWasResized()
{
	repositionSpikePlots();
}

void SpikeDisplayCanvas::renderOpenGL()
{
	//if(!plotsInitialized)
	//	initializeSpikePlots();

	glClearColor (0.667, 0.698, 0.718, 1.0);
	glClear(GL_COLOR_BUFFER_BIT); // clear buffers to preset values

	// Get Spikes from the processor
	// Iterate through each spike, passing them individually to the appropriate plots and calling redraw before moving on to the next spike

	
	//while(processor->getNextSpike(&spike))
	//{
		
	// Identify which plot the spike should go to
	
	// Distribute those spike to the appropriate plot object
	
	
	SpikeObject tmpSpike;
	 for (int i=0; i<plots.size(); i++){
		generateSimulatedSpike(&tmpSpike, 0, 150);
		plots[i]->processSpikeObject(tmpSpike);
 		plots[i]->redraw();
 		drawPlotTitle( i );
	 }
	
// 	//}
// 	//std::cout << getHeight()<<" "<< getTotalHeight()<<" "<<std::endl;
//  	glLoadIdentity();
//  	drawScrollBars();
// =======
	// processSpikeEvents();

	// for (int i = 0; i < plots.size(); i++){
	// 	plots[i]->redraw();
	// 	drawPlotTitle(i);
	// }

	drawScrollBars();
 	
}

void SpikeDisplayCanvas::processSpikeEvents()
{

	if (spikeBuffer->getNumEvents() > 0) 
	{
			
		int m = spikeBuffer->getNumEvents();
		//std::cout << m << " events received by node " << getNodeId() << std::endl;

		MidiBuffer::Iterator i (*spikeBuffer);
		MidiMessage message(0xf4);

		int samplePosition;
		i.setNextSamplePosition(samplePosition);

		while (i.getNextEvent (message, samplePosition)) {
			
			 uint8* dataptr = message.getRawData();
			// int bufferSize = message.getRawDataSize();
			// int nSamples = (bufferSize-4)/2;

			SpikeObject newSpike;

			generateSimulatedSpike(&newSpike, 0, 0);

			int chan = *(dataptr+2);
			//newSpike.nChannels = 1;

			// int16 waveform[nSamples];

			// for (int i = 0; i < nSamples; i++)
			// {
			// 	waveform[i] = (*(dataptr+4+i*2) << 8) + *(dataptr+4+i*2+1);
			// }

			plots[chan]->processSpikeObject(newSpike);

		}

	}

	spikeBuffer->clear();

}

void SpikeDisplayCanvas::drawPlotTitle(int chan){

	glViewport(0,0,getWidth(), getHeight());
	setViewportRange(0, 0, getWidth(), getHeight());

	int x, y;
	double w,h;
 	plots[chan]->getPosition(&x,&y,&w,&h);

	float alpha = 0.50f;

	glColor4f(0.0f,0.0f,0.0f,alpha);
	glRasterPos2f(x, y+h+2);
	
	String s = "Source:";//String("Channel ");
	s += (chan+1);

	getFont(String("cpmono-bold"))->FaceSize(15);
	getFont(String("cpmono-bold"))->Render(s);
}

int SpikeDisplayCanvas::getTotalHeight() 
{
	//std::cout << "TOTAL HEIGHT = " << totalHeight << std::endl;
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
void SpikeDisplayCanvas::mouseUpInCanvas(const MouseEvent& e) 	{
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
void SpikeDisplayCanvas::mouseWheelMoveInCanvas(const MouseEvent& e, float wheelIncrementX, float wheelIncrementY)
{

	repositionSpikePlots();

	repaint();
	//repaint();

	// mouseWheelMoveInCanvas(e, wheelIncrementX, wheelIncrementY);
}

void SpikeDisplayCanvas::panPlot(int p, int c, bool up){

	std::cout << "SpikeDisplayCanvas::panPlot()" << std::endl;
	if (p < 0 || p > plots.size())
		return;
	plots[p]->pan(c, up);

}
void SpikeDisplayCanvas::zoomPlot(int p, int c, bool in){

	std::cout << "SpikeDisplayCanvas::panPlot()" << std::endl;
	if (p < 0 || p > plots.size())
		return;
	plots[p]->zoom(c, in);
}

void SpikeDisplayCanvas::disablePointSmoothing()
{
	glDisable(GL_POINT_SMOOTH); // needed to make projections visible
}
