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

    spikeBuffer = processor->getSpikeBufferAddress();

    viewport = new Viewport();
    spikeDisplay = new SpikeDisplay(this, viewport);

    viewport->setViewedComponent(spikeDisplay, false);
    viewport->setScrollBarsShown(true, false);

    scrollBarThickness = viewport->getScrollBarThickness();

    addAndMakeVisible(viewport);

    update();

}

SpikeDisplayCanvas::~SpikeDisplayCanvas()
{

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

    int nPlots = processor->getNumElectrodes();
    spikeDisplay->clear();
    //numChannelsPerPlot.clear();

    for (int i = 0; i < nPlots; i++)
    {
    	spikeDisplay->addSpikePlot(processor->getNumberOfChannelsForElectrode(i), i);
    }

    //initializeSpikePlots();

    spikeDisplay->resized();
    spikeDisplay->repaint();
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

    repaint();
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
            //SpikeObject simSpike;

            unpackSpike(&newSpike, dataptr, bufferSize);

            int electrodeNum = newSpike.source;

           // generateSimulatedSpike(&simSpike, 0, 0);

           // for (int i = 0; i < newSpike.nChannels * newSpike.nSamples; i++)
           // {
           //     simSpike.data[i] = newSpike.data[i%80] + 5000;// * 3 - 10000;
           // }

           // simSpike.nSamples = 40;

            spikeDisplay->plotSpike(newSpike, electrodeNum);

        }

    }

    spikeBuffer->clear();

}

// ----------------------------------------------------------------

SpikeDisplay::SpikeDisplay(SpikeDisplayCanvas* sdc, Viewport* v) :
    canvas(sdc), viewport(v)
{

    totalHeight = 1000;

}

SpikeDisplay::~SpikeDisplay()
{

}

void SpikeDisplay::clear()
{
	if (spikePlots.size() > 0)
		spikePlots.clear();
}

void SpikeDisplay::addSpikePlot(int numChannels, int electrodeNum)
{

	std::cout << "Adding new spike plot." << std::endl;

	SpikePlot* spikePlot = new SpikePlot(canvas, electrodeNum, 1000 + numChannels);
    spikePlots.add(spikePlot);
    addAndMakeVisible(spikePlot);
}

void SpikeDisplay::paint(Graphics& g)
{

	g.fillAll(Colours::grey);

}

void SpikeDisplay::resized()
{
	// this is kind of a mess -- is there any way to optimize it?

	if (spikePlots.size() > 0)
	{

	    int w = getWidth();

	    int numColumns = 1;
	    int column, row;

	    int stereotrodeStart = 0;
	    int tetrodeStart = 0;

	    int singlePlotIndex = -1;
	    int stereotrodePlotIndex = -1;
	    int tetrodePlotIndex = -1;
	    int index;

	    float width, height;

	    for (int i = 0; i < spikePlots.size(); i++)
	    {

	    	if (spikePlots[i]->nChannels == 1)
	    	{
	    		index = ++singlePlotIndex;
	    		numColumns = (int) jmax(w / spikePlots[i]->minWidth, 1.0f);
	    		width = jmin((float) w / (float) numColumns, (float) getWidth());
	    		height = width * spikePlots[i]->aspectRatio;

	    	} else if (spikePlots[i]->nChannels == 2)
	    	{
	    		index = ++stereotrodePlotIndex;
	    		numColumns = (int) jmax(w / spikePlots[i]->minWidth, 1.0f);
	    		width = jmin((float) w / (float) numColumns, (float) getWidth());
	    		height = width * spikePlots[i]->aspectRatio;

	    	} else if (spikePlots[i]->nChannels == 4)
	  		{
	  			index = ++tetrodePlotIndex;
	  			numColumns = (int) jmax(w / spikePlots[i]->minWidth, 1.0f);
	    		width = jmin((float) w / (float) numColumns, (float) getWidth());
	    		height = width * spikePlots[i]->aspectRatio;
	  		}	

	        column = index % numColumns;

	        row = index / numColumns;

	        spikePlots[i]->setBounds(width*column, row*height, width, height);

	        if (spikePlots[i]->nChannels == 1)
	        {
	        	stereotrodeStart = (int)(height*(float(row)+1));
	        } else if (spikePlots[i]->nChannels == 2)
	        {
	        	tetrodeStart = (int)(height*(float(row)+1));
	        }

	    }

	    for (int i = 0; i < spikePlots.size(); i++)
	    {

	    	int x = spikePlots[i]->getX();
	    	int y = spikePlots[i]->getY();
	    	int w2 = spikePlots[i]->getWidth();
	    	int h2 = spikePlots[i]->getHeight();

	    	if (spikePlots[i]->nChannels == 2)
	    	{
	    		spikePlots[i]->setBounds(x, y+stereotrodeStart, w2, h2);

	    	} else if (spikePlots[i]->nChannels == 4)
	  		{
	  			spikePlots[i]->setBounds(x, y+stereotrodeStart+tetrodeStart, w2, h2);
	  		}	

	    }

	    totalHeight = 5000; // don't even deal with making the display the correct height

	    if (totalHeight < getHeight())
	    {
	        canvas->resized();
	    }
	}

}

void SpikeDisplay::mouseDown(const MouseEvent& event)
{

}

void SpikeDisplay::plotSpike(const SpikeObject& spike, int electrodeNum)
{
	spikePlots[electrodeNum]->processSpikeObject(spike);
}


// ----------------------------------------------------------------

SpikePlot::SpikePlot(SpikeDisplayCanvas* sdc, int elecNum, int p) :
    canvas(sdc), electrodeNumber(elecNum), plotType(p), isSelected(false),
    limitsChanged(true)

{

    font = Font("Default", 15, Font::plain);

     switch (p)
    {
        case SINGLE_PLOT:
           // std::cout<<"SpikePlot as SINGLE_PLOT"<<std::endl;
            nWaveAx = 1;
            nProjAx = 0;
            nChannels = 1;
            minWidth = 200;
            aspectRatio = 1.0f;
            break;
        case STEREO_PLOT:
          //  std::cout<<"SpikePlot as STEREO_PLOT"<<std::endl;
            nWaveAx = 2;
            nProjAx = 1;
            nChannels = 2;
            minWidth = 300;
            aspectRatio = 0.5f;
            break;
        case TETRODE_PLOT:
           // std::cout<<"SpikePlot as TETRODE_PLOT"<<std::endl;
            nWaveAx = 4;
            nProjAx = 6;
            nChannels = 4;
            minWidth = 500;
            aspectRatio = 0.5f;
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

  	g.setColour(Colours::white);
    g.drawRect(0,0,getWidth(),getHeight());

    g.setFont(font);

	g.drawText(String(electrodeNumber),10,0,50,20,Justification::left,false);

}

void SpikePlot::processSpikeObject(const SpikeObject& s)
{
    //std::cout<<"ElectrodePlot::processSpikeObject()"<<std::endl;

    for (int i = 0; i < nWaveAx; i++)
        wAxes[i]->updateSpikeData(s);

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

	float width = getWidth()-10;
	float height = getHeight()-20;

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
        wAxes[i]->setBounds(5 + (i % nWaveCols) * axesWidth/nWaveCols, 15 + (i/nWaveCols) * axesHeight, axesWidth/nWaveCols, axesHeight);

    for (int i = 0; i < nProjAx; i++)
        pAxes[i]->setBounds(5 + (1 + i%nProjCols) * axesWidth, 15 + (i/nProjCols) * axesHeight, axesWidth, axesHeight);

}

void SpikePlot::setLimitsOnAxes()
{
 	//std::cout<<"SpikePlot::setLimitsOnAxes()"<<std::endl;

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


WaveAxes::WaveAxes(int channel) : GenericAxes(channel)
{

}

void WaveAxes::paint(Graphics& g)
{
    g.setColour(Colours::black);
    g.fillRect(5,5,getWidth()-10, getHeight()-10);

    int chan = 0;

    // if no spikes have been received then don't plot anything
    if (!gotFirstSpike)
    {
    	return; 
    }

    // draw the grid lines for the waveforms
   // if (drawGrid)
    //	drawWaveformGrid(s.threshold[chan], s.gain[chan]);

    // compute the spatial width for each waveform sample
    //compute the spatial width for each waveform sample
    float dx = getWidth()/s.nSamples;
    float x = 0;

    // type corresponds to channel so we need to calculate the starting
    // sample based upon which channel is getting plotted
    int	sampIdx = s.nSamples * type; //
    
    // draw the individual waveform points
    g.setColour(Colours::white);

    int dSamples = 1;

    for (int i = 0; i < s.nSamples-1; i++)
    {
    	//std::cout << s.data[sampIdx] << std::endl;
    	g.drawLine(x, (s.data[sampIdx]-32768)/100, x+dx, (s.data[sampIdx+1]-32768)/100);
    	sampIdx += dSamples;
    	x += dx;
    }

    // draw the threshold line and labels

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

void GenericAxes::updateSpikeData(const SpikeObject& newSpike)
{
    if (!gotFirstSpike)
    {
        gotFirstSpike = true;
    }

    s = newSpike;
}

void GenericAxes::setYLims(double ymin, double ymax)
{

    //std::cout << "setting y limits to " << ymin << " " << ymax << std::endl;
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