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

    g.fillAll(Colours::darkgrey);

}

void SpikeDisplayCanvas::refresh()
{
    processSpikeEvents();
}

void SpikeDisplayCanvas::processSpikeEvents()
{

    if (spikeBuffer->getNumEvents() > 0)
    {

        MidiBuffer::Iterator i(*spikeBuffer);
        MidiMessage message(0xf4);

        int samplePosition = 0;

        i.setNextSamplePosition(samplePosition);

        while (i.getNextEvent(message, samplePosition))
        {

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

   // tetrodePlotMinWidth = 500;
   // stereotrodePlotMinWidth = 400;
  //  singleElectrodePlotMinWidth = 200;

  //  tetrodePlotRatio = 0.5;
  //  stereotrodePlotRatio = 0.2;
  //  singleElectrodePlotRatio = 1.0;

    totalHeight = 1000;

    // for (int i = 0; i < 10; i++)
    // {
    //     TetrodePlot* tetrodePlot = new TetrodePlot(canvas, i);
    //     tetrodePlots.add(tetrodePlot);
    //     addAndMakeVisible(tetrodePlot);
    // }

}

SpikeDisplay::~SpikeDisplay()
{

}

void SpikeDisplay::addSpikePlot(int numChannels)
{

}

void SpikeDisplay::paint(Graphics& g)
{

    g.fillAll(Colours::grey);
}

void SpikeDisplay::resized()
{

    // int w = getWidth();

    // int numColumns = w / tetrodePlotMinWidth;
    // int column, row;

    // float width = (float) w / (float) numColumns;
    // float height = width * tetrodePlotRatio;

    // for (int i = 0; i < tetrodePlots.size(); i++)
    // {

    //     column = i % numColumns;
    //     row = i / numColumns;
    //     tetrodePlots[i]->setBounds(width*column,row*height,width,height);

    // }

    // totalHeight = (int)(height*(float(row)+1));

    // if (totalHeight < getHeight())
    // {
    //     canvas->resized();
    // }

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

SpikePlot::SpikePlot(SpikeDisplayCanvas* sdc, int elecNum, int p) :
    canvas(sdc), electrodeNumber(elecNum), plotType(p), isSelected(false),
    limitsChanged(true)

{
    isSelected = false;

     switch (p)
    {
        case SINGLE_PLOT:
            std::cout<<"SpikePlot as SINGLE_PLOT"<<std::endl;
            nWaveAx = 1;
            nProjAx = 0;
            nChannels = 1;
            break;
        case STEREO_PLOT:
            std::cout<<"SpikePlot as STEREO_PLOT"<<std::endl;
            nWaveAx = 2;
            nProjAx = 1;
            nChannels = 2;
            break;
        case TETRODE_PLOT:
            std::cout<<"SpikePlot as TETRODE_PLOT"<<std::endl;
            nWaveAx = 4;
            nProjAx = 6;
            nChannels = 4;
            break;
            //        case HIST_PLOT:
            //            nWaveAx = 1;
            //            nProjAx = 0;
            //            nHistAx = 1;
            //            break;
        default: // unsupported number of axes provided
            std::cout<<"SpikePlot as UNKNOWN, defaulting to SINGLE_PLOT"<<std::endl;
            nWaveAx = 1;
            nProjAx = 0;
            plotType = SINGLE_PLOT;
            nChannels = 1;
    }

    initAxes();

}

SpikePlot::~SpikePlot()
{

}

void SpikePlot::paint(Graphics& g)
{

    //g.setColour(Colours::darkgrey);
    //g.fillRect(2, 2, getWidth()-4, getHeight()-4);

}

void SpikePlot::processSpikeObject(SpikeObject s)
{
    //std::cout<<"ElectrodePlot::processSpikeObject()"<<std::endl;
    for (int i = 0; i < nWaveAx; i++)
        wAxes[i]->updateSpikeData(s);
    //    wAxes[1].updateSpikeData(s);
    for (int i = 0; i < nProjAx; i++)
        pAxes[i]->updateSpikeData(s);
}

void SpikePlot::select()
{
    isSelected = true;
}

void SpikePlot::deselect()
{
    isSelected = false;
}

void SpikePlot::initAxes()
{
    initLimits();

    for (int i = 0; i < nWaveAx; i++)
    {
    	WaveAxes* wAx = new WaveAxes(WAVE1 + i);
    	wAxes.add(wAx);
    	addAndMakeVisible(wAx);
    }

    for (int i = 0; i < nProjAx; i++)
    {
    	ProjectionAxes* pAx = new ProjectionAxes(PROJ1x2 + i);
    	pAxes.add(pAx);
    	addAndMakeVisible(pAx);
    }

    setLimitsOnAxes(); // initialize thel limits on the axes
}

void SpikePlot::resized()
{

	float width = getWidth();
	float height = getHeight();

	float axesWidth, axesHeight;

  	// to compute the axes positions we need to know how many columns of proj and wave axes should exist
    // using these two values we can calculate the positions of all of the sub axes
    int nProjCols, nWaveCols;

    switch (plotType)
    {
        case SINGLE_PLOT:
            nProjCols = 0;
            nWaveCols = 1;
            axesWidth = width;
            axesHeight = height;
            break;

        case STEREO_PLOT:
            nProjCols = 1;
            nWaveCols = 2;
            axesWidth = width/2;
            axesHeight = height;
            break;
        case TETRODE_PLOT:
            nProjCols = 3;
            nWaveCols = 2;
            axesWidth = width/4;
            axesHeight = height/2;
            break;
    }

    for (int i = 0; i < nWaveAx; i++)
        wAxes[i]->setBounds((i % nWaveCols) * axesWidth/nWaveCols, (i/nWaveCols) * axesHeight, axesWidth/nWaveCols, axesHeight);

    for (int i = 0; i < nProjAx; i++)
        pAxes[i]->setBounds((1 + i%nProjCols) * axesWidth, (i/nProjCols) * axesHeight, axesWidth, axesHeight);

}

void SpikePlot::setLimitsOnAxes()
{
 	std::cout<<"SpikePlot::setLimitsOnAxes()"<<std::endl;

    for (int i = 0; i < nWaveAx; i++)
        wAxes[i]->setYLims(limits[i][0], limits[i][1]);

    // Each Projection sets its limits using the limits of the two waveform dims it represents.
    // Convert projection number to indecies, and then set the limits using those indices
    int j1, j2;
    for (int i = 0; i < nProjAx; i++)
    {
        n2ProjIdx(pAxes[i]->getType(), &j1, &j2);
        pAxes[i]->setYLims(limits[j1][0], limits[j1][1]);
        pAxes[i]->setXLims(limits[j2][0], limits[j2][1]);
    }
}

void SpikePlot::initLimits()
{
    for (int i = 0; i < nChannels; i++)
    {
        limits[i][0] = 1209;//-1*pow(2,11);
        limits[i][1] = 11059;//pow(2,14)*1.6;
    }

}

void SpikePlot::getBestDimensions(int* w, int* h)
{
    switch (plotType)
    {
        case TETRODE_PLOT:
            *w = 4;
            *h = 2;
            break;
        case STEREO_PLOT:
            *w = 2;
            *h = 1;
            break;
        case SINGLE_PLOT:
            *w = 1;
            *h = 1;
            break;
        default:
            *w = 1;
            *h = 1;
            break;
    }
}

void SpikePlot::clear()
{
    std::cout << "SpikePlot::clear()" << std::endl;

    for (int i = 0; i < nWaveAx; i++)
        wAxes[i]->clear();
    for (int i = 0; i < nProjAx; i++)
        pAxes[i]->clear();
}

void SpikePlot::pan(int dim, bool up)
{

    std::cout << "SpikePlot::pan() dim:" << dim << std::endl;

    int mean = (limits[dim][0] + limits[dim][1])/2;
    int dLim = limits[dim][1] - mean;

    if (up)
        mean = mean + dLim/20;
    else
        mean = mean - dLim/20;

    limits[dim][0] = mean-dLim;
    limits[dim][1] = mean+dLim;

    setLimitsOnAxes();
}

void SpikePlot::zoom(int dim, bool in)
{
    std::cout << "SpikePlot::zoom()" << std::endl;

    int mean = (limits[dim][0] + limits[dim][1])/2;
    int dLim = limits[dim][1] - mean;

    if (in)
        dLim = dLim * .90;
    else
        dLim = dLim / .90;

    limits[dim][0] = mean-dLim;
    limits[dim][1] = mean+dLim;

    setLimitsOnAxes();
}


void SpikePlot::n2ProjIdx(int proj, int* p1, int* p2)
{
    int d1, d2;
    if (proj==PROJ1x2)
    {
        d1 = 0;
        d2 = 1;
    }
    else if (proj==PROJ1x3)
    {
        d1 = 0;
        d2 = 2;
    }
    else if (proj==PROJ1x4)
    {
        d1 = 0;
        d2 = 3;
    }
    else if (proj==PROJ2x3)
    {
        d1 = 1;
        d2 = 2;
    }
    else if (proj==PROJ2x4)
    {
        d1 = 1;
        d2 = 3;
    }
    else if (proj==PROJ3x4)
    {
        d1 = 2;
        d2 = 3;
    }
    else
    {
        std::cout<<"Invalid projection:"<<proj<<"! Cannot determine d1 and d2"<<std::endl;
        *p1 = -1;
        *p2 = -1;
        return;
    }
    *p1 = d1;
    *p2 = d2;
}

// --------------------------------------------------


// TetrodePlot::TetrodePlot(SpikeDisplayCanvas* sdc, int elecNum) :
//     SpikePlot(sdc, elecNum, 4)
// {

//     for (int i = 0; i < numChannels; i++)
//     {
//         WaveformPlot* wp = new WaveformPlot();
//         addAndMakeVisible(wp);
//         waveformPlots.add(wp);
//     }

//     for (int i = 0; i < 6; i++)
//     {
//         ProjectionPlot* pp = new ProjectionPlot();
//         addAndMakeVisible(pp);
//         projectionPlots.add(pp);
//     }

// }

// void TetrodePlot::resized()
// {
//     float w = (float) getWidth() / 5.0f;
//     float h = (float) getHeight() / 2.0f;

//     waveformPlots[0]->setBounds(0, 0, w, h);
//     waveformPlots[1]->setBounds(w, 0, w, h);
//     waveformPlots[2]->setBounds(0, h, w, h);
//     waveformPlots[3]->setBounds(w, h, w, h);

//     projectionPlots[0]->setBounds(w*2, 0, w, h);
//     projectionPlots[1]->setBounds(w*3, 0, w, h);
//     projectionPlots[2]->setBounds(w*4, 0, w, h);
//     projectionPlots[3]->setBounds(w*2, h, w, h);
//     projectionPlots[4]->setBounds(w*3, h, w, h);
//     projectionPlots[5]->setBounds(w*4, h, w, h);

// }

// StereotrodePlot::StereotrodePlot(SpikeDisplayCanvas* sdc, int elecNum) :
//     SpikePlot(sdc, elecNum, 2)
// {

//     for (int i = 0; i < numChannels; i++)
//     {
//         WaveformPlot* wp = new WaveformPlot();
//         addAndMakeVisible(wp);
//         waveformPlots.add(wp);
//     }

//     ProjectionPlot* pp = new ProjectionPlot();
//     addAndMakeVisible(pp);
//     projectionPlots.add(pp);

// }

// void StereotrodePlot::resized()
// {
//     float w = (float) getWidth() / 3.0f;
//     float h = (float) getHeight() / 1.0f;

//     waveformPlots[0]->setBounds(0, 0, w, h);
//     waveformPlots[1]->setBounds(w, 0, w, h);

//     projectionPlots[0]->setBounds(w*2, 0, w, h);

// }

// SingleElectrodePlot::SingleElectrodePlot(SpikeDisplayCanvas* sdc, int elecNum) :
//     SpikePlot(sdc, elecNum, 1)
// {

//     WaveformPlot* wp = new WaveformPlot();
//     addAndMakeVisible(wp);
//     waveformPlots.add(wp);

// }

// void SingleElectrodePlot::resized()
// {
//     float w = (float) getWidth() / 1.0f;
//     float h = (float) getHeight() / 1.0f;

//     waveformPlots[0]->setBounds(0, 0, w, h);

// }

// -----------------------------------------------------

WaveAxes::WaveAxes(int channel) : GenericAxes(channel)
{

}

void WaveAxes::paint(Graphics& g)
{
    g.setColour(Colours::black);
    g.fillRect(5,5,getWidth()-5, getHeight()-5);
}

void WaveAxes::clear()
{
	
}


// --------------------------------------------------

ProjectionAxes::ProjectionAxes(int projectionNum) : GenericAxes(projectionNum)
{

}

void ProjectionAxes::paint(Graphics& g)
{
    g.setColour(Colours::orange);
    g.fillRect(5,5,getWidth()-5, getHeight()-5);
}

void ProjectionAxes::clear()
{

}

// --------------------------------------------------

GenericAxes::GenericAxes(int t)
    : gotFirstSpike(false), type(t)
{
    ylims[0] = 0;
    ylims[1] = 1;

    xlims[0] = 0;
    xlims[1] = 1;

    font = Font("Default", 12, Font::plain);

}

GenericAxes::~GenericAxes()
{

}

void GenericAxes::updateSpikeData(SpikeObject newSpike)
{
    if (!gotFirstSpike)
    {
        gotFirstSpike = true;
    }

    s = newSpike;
}

void GenericAxes::setYLims(double ymin, double ymax)
{

    std::cout << "setting y limits to " << ymin << " " << ymax << std::endl;
    ylims[0] = ymin;
    ylims[1] = ymax;
}
void GenericAxes::getYLims(double* min, double* max)
{
    *min = ylims[0];
    *max = ylims[1];
}
void GenericAxes::setXLims(double xmin, double xmax)
{
    xlims[0] = xmin;
    xlims[1] = xmax;
}
void GenericAxes::getXLims(double* min, double* max)
{
    *min = xlims[0];
    *max = xlims[1];
}


void GenericAxes::setType(int t)
{
    if (t < WAVE1 || t > PROJ3x4)
    {
        std::cout<<"Invalid Axes type specified";
        return;
    }
    type = t;
}

int GenericAxes::getType()
{
    return type;
}