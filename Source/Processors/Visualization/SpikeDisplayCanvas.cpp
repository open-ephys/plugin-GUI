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

#include "SpikeDisplayCanvas.h"

SpikeDisplayCanvas::SpikeDisplayCanvas(SpikeDisplayNode* n) :
	 	newSpike(false), processor(n)
{
    
	update();
	
	spikeBuffer = processor->getSpikeBufferAddress();

	viewport = new Viewport();
	spikeDisplay = new SpikeDisplay(this, viewport);

	viewport->setViewedComponent(spikeDisplay, false);
	viewport->setScrollBarsShown(true, false);

	scrollBarThickness = viewport->getScrollBarThickness();

	addAndMakeVisible(viewport);
	
}

SpikeDisplayCanvas::~SpikeDisplayCanvas()
{
	
}

// void SpikeDisplayCanvas::initializeSpikePlots(){

// 	std::cout<<"Initializing Plots"<<std::endl;
    
//     // This layout system really only works if plot types are aggregated together.
//     // It might be worthwhile to investigate the merits of using a grid system
//     // The canvas is defined as N grid widths wide. Each SpikePlot defines its
//     // dimensions in grid widths.
//     //
//     // Plots are added from left to right, top to bottom.  A plot is put into place
//     // if it can fit into the next grid location w/o its top going above the current
//     // row and w/o its bottom going below the current row
//     //
//     // This would lead to dead space but it would allow the plots to all scale accoring
//     // to how much space they need.  The current system of deciding plot sizes, isn't going
//     // to scale well.... this needs more thought

// // 	if (plots.size() != nPlots)
// // 	{

// // 	int totalWidth = getWidth(); 

// // 	int plotWidth =  (totalWidth - yBuffer * ( nCols+1)) / nCols + .99;
// // 	int plotHeight = plotWidth / 2 + .5;
// // 	int rowCount = 0;

// // 	plots.clear();

// // 	for (int i = 0; i < nPlots; i++)
// // 	{
// //         int pType;
// // 		switch (processor->getNumberOfChannelsForElectrode(i)){
// // 			case 1:
// //                 pType = SINGLE_PLOT;
// //                 break;
// // 			case 2:
// //                 pType = STEREO_PLOT;
// //                 break;
// // 			case 4:
// //                 pType = TETRODE_PLOT;
// //                 break;
// //             default:
// //                 pType = SINGLE_PLOT;
// //                 break;
// //         }
        
// // //        bool use_generic_plots_flag = true;
        
// // //        BaseUIElement *sp;
        
// //   //      if (use_generic_plots_flag)
// //         SpikePlot *sp = new SpikePlot(xBuffer + i%nCols * (plotWidth + xBuffer) ,
// //                                yBuffer + rowCount * (plotHeight + yBuffer),
// //                                plotWidth, plotHeight, pType);
        
// // //        else
// // //            sp = new StereotrodePlot(xBuffer + i%nCols * (plotWidth + xBuffer) ,
// // //                                      yBuffer + rowCount * (plotHeight + yBuffer),
// // //                                      plotWidth, plotHeight);
// //         plots.add(sp);

// // 		if (i%nCols == nCols-1)
// // 			rowCount++;
// // 	}
// // 	//totalHeight = rowCount * (plotHeight + yBuffer) + yBuffer * 2;
// // 	// Set the total height of the Canvas to the top of the top most plot

// //     plotsInitialized = true;
// // 	repositionSpikePlots();
// // 	}
// }

// void SpikeDisplayCanvas::repositionSpikePlots(){
	
// // 	int canvasWidth = getWidth();
// // 	int gridSize = canvasWidth / nCols;
    
// //     gridSize = (gridSize > MIN_GRID_SIZE) ? gridSize : MIN_GRID_SIZE;
// //     gridSize = (gridSize < MAX_GRID_SIZE) ? gridSize : MAX_GRID_SIZE;
        
    
    
// //     int x = xBuffer;
// //     int y = getHeight() - yBuffer;
// //     int p = 0;
// //     int w,h;
// //     int yIncrement = 0;
// //     bool loopCheck = false;
// //     //std::cout<<"Positioning Spike Plots"<<std::endl;
// //     while (p < plots.size()){
        
// //         // Ask the current plot for its desired dims
// //         plots[p]->getBestDimensions(&w, &h);
// //         w *= gridSize;
// //         h *= gridSize;
        
// //         // Check to see if plot exceeds width of canvas, if yes, set x back to 0 and go to the bottom most plot on the canvas
// //         if ( (x + w + xBuffer > canvasWidth - xBuffer) && !loopCheck){
// //             //std::cout<<"Collision with the edge of the canvas, going down a row"<<std::endl;
// //             x = xBuffer;
// //             y = y - yIncrement - yBuffer;
// //             yIncrement = 0;
// //             loopCheck = true;
// //             continue;
// //         }
// //         // else place the plot
// //         else{
// //             //std::cout<<"Positioning p:"<<p<<" at "<<x<<","<<y - h<<"  "<<w<<","<<h<<std::endl;
// //            // plots[p]->setPosition(x, y - h + getScrollAmount(), w, h);
// //             x = x + w + xBuffer;

// //             // set a new minimum
// //             if (h > yIncrement)
// //                 yIncrement = h;
            
// //             // increment p
// //             p++;
// //             loopCheck = false;
// //         }
// //     }

// // //  int plotWidth =  (totalWidth - yBuffer * ( nCols+1)) / nCols + .99;
// // //	int plotHeight = plotWidth / 2 + .5;
// // //	int rowCount = 0;

// // //	for (int i=0; i < plots.size(); i++)
// // //	{
// // //
// // //		plots[i]->setPosition(	xBuffer + i%nCols * (plotWidth + xBuffer) , 
// // //								getHeight() - ( yBuffer + plotHeight + rowCount * (plotHeight + yBuffer)) + getScrollAmount(), 
// // //								plotWidth, 
// // //								plotHeight); // deprecated conversion from string constant to char
// // //
// // //		if (i%nCols == nCols-1)
// // //			rowCount++;	
// // //	 }

// // 	// Set the total height of the Canvas to the top of the top most plot
// // //	totalHeight = (rowCount + 1) * (plotHeight + yBuffer) + yBuffer;
// //     totalHeight = getHeight() + (y + yIncrement);
// }

// void SpikeDisplayCanvas::newOpenGLContextCreated()
// {
// 	std::cout<<"SpikeDisplayCanvas::newOpenGLContextCreated()"<<std::endl;
// 	setUp2DCanvas();
// 	activateAntiAliasing();
// 	disablePointSmoothing();

// 	glClearColor (0.667, 0.698, 0.718, 1.0);
// 	resized();
// 	//endAnimation();
// }

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

	//nPlots = processor->getNumElectrodes();
	// numChannelsPerPlot.clear();

	// for (int i = 0; i < nPlots; i++)
	// {
	// 	numChannelsPerPlot.add(processor->getNumberOfChannelsForElectrode(i));
	// }

	//initializeSpikePlots();

	repaint();
}


void SpikeDisplayCanvas::refreshState()
{
	// called when the component's tab becomes visible again
	// displayBufferIndex = processor->getDisplayBufferIndex();
	// screenBufferIndex = 0;
	resized();
}

void SpikeDisplayCanvas::resized()
{
	viewport->setBounds(0,0,getWidth(),getHeight()-90);

	spikeDisplay->setBounds(0,0,getWidth()-scrollBarThickness, spikeDisplay->getTotalHeight());
}

void SpikeDisplayCanvas::paint(Graphics& g)
{
	processSpikeEvents();

	g.fillAll(Colours::cyan);

	g.setColour(Colours::white);

	g.drawLine(0,0, getWidth(), getHeight());
	g.drawLine(0,getHeight(),getWidth(), 0);
 	
}

void SpikeDisplayCanvas::processSpikeEvents()
{

	if (spikeBuffer->getNumEvents() > 0) 
	{
	
		MidiBuffer::Iterator i (*spikeBuffer);
	 	MidiMessage message(0xf4);

	 	int samplePosition = 0;

	 	i.setNextSamplePosition(samplePosition);
			
	 	while (i.getNextEvent (message, samplePosition)) {

	 		 const uint8_t* dataptr = message.getRawData();
	 		 int bufferSize = message.getRawDataSize();
	 		 int nSamples = (bufferSize-4)/2;

	 		SpikeObject newSpike;
	 		SpikeObject simSpike;

	 		unpackSpike(&newSpike, dataptr, bufferSize);

	 		int chan = newSpike.source;

	 		generateSimulatedSpike(&simSpike, 0, 0);

	 		for (int i = 0; i < newSpike.nChannels * newSpike.nSamples; i++)
	 		{
                     simSpike.data[i] = newSpike.data[i%80] + 5000;// * 3 - 10000;
	 		}

	 		simSpike.nSamples = 40;

			spikeDisplay->plotSpike(simSpike);

		}

	}

	spikeBuffer->clear();

}


// ----------------------------------------------------------------

SpikeDisplay::SpikeDisplay(SpikeDisplayCanvas* sdc, Viewport* v) :
	canvas(sdc), viewport(v)
{

	minWidth = 300; 
	maxWidth = 500;
	minHeight = 100;
	maxHeight = 200;

	totalHeight = 1000;

	for (int i = 0; i < 10; i++)
	{
		SpikePlot* spikePlot = new SpikePlot(canvas, i, 2);
		spikePlots.add(spikePlot);
		addAndMakeVisible(spikePlot);
	}

}

SpikeDisplay::~SpikeDisplay()
{

}

void SpikeDisplay::addSpikePlot(int numChannels)
{

}

void SpikeDisplay::paint(Graphics& g)
{

	g.fillAll(Colours::blue);
}

void SpikeDisplay::resized()
{

	int w = getWidth();

	int numColumns = w / minWidth;
	int column, row;

	float width = (float) w / (float) numColumns;
	float height = width * 0.75;

	for (int i = 0; i < spikePlots.size(); i++)
	{

		column = i % numColumns;
		row = i / numColumns;
		spikePlots[i]->setBounds(width*column,row*height,width,height);

	}

	totalHeight = (int) (height*(float(row)+1));

	if (totalHeight < getHeight())
	{
		canvas->resized();
	}

	//setBounds(0,0,getWidth(), totalHeight);


	 // layoutManagerX.layOutComponents((Component**) spikePlots.getRawDataPointer(), 
	 // 							   spikePlots.size(),
	 // 							   0,
	 // 							   0,
	 // 							   getWidth(),
	 // 							   getHeight(),
	 // 							   false,
	 // 							   false);
}

void SpikeDisplay::mouseDown(const MouseEvent& event)
{

}

void SpikeDisplay::plotSpike(const SpikeObject& spike)
{

}




// ----------------------------------------------------------------

SpikePlot::SpikePlot(SpikeDisplayCanvas* sdc, int elecNum, int numChans) :
	canvas(sdc), electrodeNumber(elecNum), numChannels(numChans)

{
	isSelected = false;

}

SpikePlot::~SpikePlot()
{

}

void SpikePlot::paint(Graphics& g)
{

	g.setColour(Colours::yellow);
	g.fillRect(10, 10, getWidth()-20, getHeight()-20);

	g.setColour(Colours::black);

	g.drawLine(0, 0, getWidth(), getHeight());
	g.drawLine(0, getHeight(), getWidth(), 0);

}

void SpikePlot::select()
{
	isSelected = true;
}

void SpikePlot::deselect()
{
	isSelected = false;
}

void SpikePlot::resized()
{

}


