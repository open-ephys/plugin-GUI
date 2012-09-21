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

	nCols = 2;

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
    
    // This layout system really only works if plot types are aggregated together.
    // It might be worthwhile to investigate the merits of using a grid system
    // The canvas is defined as N grid widths wide. Each SpikePlot defines its
    // dimensions in grid widths.
    //
    // Plots are added from left to right, top to bottom.  A plot is put into place
    // if it can fit into the next grid location w/o its top going above the current
    // row and w/o its bottom going below the current row
    //
    // This would lead to dead space but it would allow the plots to all scale accoring
    // to how much space they need.  The current system of deciding plot sizes, isn't going
    // to scale well.... this needs more thought

	if (plots.size() != nPlots)
	{

	int totalWidth = getWidth(); 

	int plotWidth =  (totalWidth - yBuffer * ( nCols+1)) / nCols + .99;
	int plotHeight = plotWidth / 2 + .5;
	int rowCount = 0;

	plots.clear();

	for (int i = 0; i < nPlots; i++)
	{
        int pType;
		switch (processor->getNumberOfChannelsForElectrode(i)){
			case 1:
                pType = SINGLE_PLOT;
                break;
			case 2:
                pType = STEREO_PLOT;
                break;
			case 4:
                pType = TETRODE_PLOT;
                break;
            default:
                pType = SINGLE_PLOT;
                break;
        }
        
//        bool use_generic_plots_flag = true;
        
//        BaseUIElement *sp;
        
  //      if (use_generic_plots_flag)
        SpikePlot *sp = new SpikePlot(xBuffer + i%nCols * (plotWidth + xBuffer) ,
                               yBuffer + rowCount * (plotHeight + yBuffer),
                               plotWidth, plotHeight, pType);
        
//        else
//            sp = new StereotrodePlot(xBuffer + i%nCols * (plotWidth + xBuffer) ,
//                                      yBuffer + rowCount * (plotHeight + yBuffer),
//                                      plotWidth, plotHeight);
        plots.add(sp);

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
	 processSpikeEvents();

	 for (int i = 0; i < plots.size(); i++)
	 {
	 	plots[i]->redraw();
	 	drawPlotTitle(i);
	 }

	drawScrollBars();
 	
}

void SpikeDisplayCanvas::processSpikeEvents()
{


	if (spikeBuffer->getNumEvents() > 0) 
	{
		
		//int m = spikeBuffer->getNumEvents();

		//std::cout << "Received " << m << " events." << std::endl;
			
		//std::cout << m << " events received by node " << getNodeId() << std::endl;
		MidiBuffer::Iterator i (*spikeBuffer);
		MidiMessage message(0xf4);

		int samplePosition = 0;

		i.setNextSamplePosition(samplePosition);
		
		//int eventCount = 0;
		
		while (i.getNextEvent (message, samplePosition)) {
			//eventCount++;
			 uint8_t* dataptr = message.getRawData();
			 int bufferSize = message.getRawDataSize();
			// int nSamples = (bufferSize-4)/2;

			SpikeObject newSpike;
			SpikeObject simSpike;

			unpackSpike(&newSpike, dataptr, bufferSize);

			//

			int chan = newSpike.source;

			generateSimulatedSpike(&simSpike, 0, 0);


			for (int i = 0; i < newSpike.nChannels * newSpike.nSamples; i++)
			{
				simSpike.data[i] = newSpike.data[i] + 5000;// * 3 - 10000;
			}

			simSpike.nSamples = 40;

			

			// std::cout << "Received spike on electrode " << chan << std::endl;

			// std::cout << "Spike has " << newSpike.nChannels << " channels and " <<
			//              newSpike.nSamples << " samples." << std::endl;

			// std::cout << "Data: ";

			// for (int n = 0; n < newSpike.nSamples; n++)
			// {
			// 	std::cout << newSpike.data[n] << " ";
			// }

			//	std::cout << std::endl;

			plots[chan]->processSpikeObject(simSpike);

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

	getFont(cpmono_bold)->FaceSize(15);
	getFont(cpmono_bold)->Render(s);
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
