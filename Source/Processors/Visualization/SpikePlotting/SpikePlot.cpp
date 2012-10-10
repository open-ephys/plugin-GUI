#include "SpikePlot.h"
#include "../SpikeObject.h"
#include "PlotUtils.h"

SpikePlot::SpikePlot():
	BaseUIElement(),  limitsChanged(true), nChannels(0), plotType(0), nWaveAx(1), nProjAx(0)
{

}

SpikePlot::SpikePlot(int x, int y, int w, int h, int p):
	BaseUIElement(x,y,w,h,0), limitsChanged(true), plotType(p)
{

    switch(p){
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

SpikePlot::~SpikePlot(){
}

// As a plot is a collection of axes simply have each axes can draw itself
void SpikePlot::redraw(){
    for (int i=0; i<nWaveAx; i++)
        wAxes[i].redraw();
//    wAxes[1].redraw();
    for (int i=0; i<nProjAx; i++)
        pAxes[i].redraw();
}

// Have each axes process the spike event
void SpikePlot::processSpikeObject(SpikeObject s){
	//std::cout<<"ElectrdePlot::processSpikeObject()"<<std::endl;
    for (int i=0; i<nWaveAx; i++)
        wAxes[i].updateSpikeData(s);
//    wAxes[1].updateSpikeData(s);
    for (int i=0; i<nProjAx; i++)
        pAxes[i].updateSpikeData(s);
}


void SpikePlot::setEnabled(bool e){
	BaseUIElement::enabled = e;
    
    for (int i=0; i<nWaveAx; i++)
        wAxes[i].setEnabled(e);
//    wAxes[1].setEnabled(e);
    for (int i=0; i<nProjAx; i++)
        pAxes[i].setEnabled(e);
}

bool SpikePlot::getEnabled(){
	return BaseUIElement::enabled;
}


void SpikePlot::initAxes(){
	initLimits();
	
    for (int i=0; i<nWaveAx; i++){
        wAxes[i] = WaveAxes(0, 0, 1, 1, WAVE1 + i); // add i to increment the wave channel
        wAxes[i].setWaveformColor(1.0, 1.0, 1.0);
    }
    
    for (int i=0; i<nProjAx; i++){
        pAxes[i] = ProjectionAxes(0, 0, 1, 1, PROJ1x2 + i);
        pAxes[i].setPointColor(1.0, 1.0, 1.0);
    }

    updateAxesPositions(); // Set the position of the individual axes within the plot
    setLimitsOnAxes(); // initialize thel limits on the axes
}
void SpikePlot::updateAxesPositions(){
    int minX = BaseUIElement::xpos;
	int minY = BaseUIElement::ypos;
	
   	double axesWidth;// = BaseUIElement::width/2;
	double axesHeight;// = BaseUIElement::height;
	
    
    // to compute the axes positions we need to know how many columns of proj and wave axes should exist
    // using these two values we can calculate the positions of all of the sub axes
    int nProjCols, nWaveCols;
    switch (plotType){
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
    
    for (int i=0; i<nWaveAx; i++)
        wAxes[i].setPosition(minX + (i % nWaveCols) * axesWidth/nWaveCols, minY + (i/nWaveCols) * axesHeight, axesWidth/nWaveCols, axesHeight);

    for (int i=0; i<nProjAx; i++)
        pAxes[i].setPosition(minX + (1 + i%nProjCols) * axesWidth, minY + (i/nProjCols) * axesHeight, axesWidth, axesHeight);
}

void SpikePlot::setLimitsOnAxes(){
    std::cout<<"SpikePlot::setLimitsOnAxes()"<<std::endl;
    
    for (int i=0; i<nWaveAx; i++)
        wAxes[i].setYLims(limits[i][0], limits[i][1]);

    // Each Projection sets its limits using the limits of the two waveform dims it represents.
    // Convert projection number to indecies, and then set the limits using those indices
    int j1, j2;
    for (int i=0; i<nProjAx; i++)
    {
            n2ProjIdx(pAxes[i].getType(), &j1, &j2);
            pAxes[i].setYLims(limits[j1][0], limits[j1][1]);
            pAxes[i].setXLims(limits[j2][0], limits[j2][1]);
    }
}
void SpikePlot::setPosition(int x, int y, double w, double h){
    
//    std::cout<<"SpikePlot::setPosition()"<<std::endl;
	BaseUIElement::setPosition(x,y,w,h);
	updateAxesPositions();
    
}


void SpikePlot::initLimits(){
    for (int i=0; i<nChannels; i++)
    {
        limits[i][0] = 1209;//-1*pow(2,11);
        limits[i][1] = 11059;//pow(2,14)*1.6;
    }

}

void SpikePlot::getBestDimensions(int* w, int* h){
    switch(plotType){
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

void SpikePlot::clear(){
    std::cout<<"SpikePlot::clear()"<<std::endl;
    for (int i=0; i<nWaveAx; i++)
        wAxes[i].clear();
    for (int i=0; i<nProjAx; i++)
        pAxes[i].clear();
}


bool SpikePlot::processKeyEvent(SimpleKeyEvent k){

    return true;
}

void SpikePlot::pan(int dim, bool up){

    std::cout<<"SpikePlot::pan() dim:"<<dim<<std::endl;
    
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
void SpikePlot::zoom(int dim, bool in){
    std::cout<<"SpikePlot::zoom()"<<std::endl;
    
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


