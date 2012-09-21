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

// Each plot needs to update its children axes when its redraw gets called.
//  it also needs to call the parent plot  when children axes get added it
//  should place them in the correct location because it KNOWS where WAVE1 and PROJ1x3
//  should go by default. This isn't as general as it should be but its a good push in
//  the right direction

void SpikePlot::redraw(){
	//std::cout<<"SpikePlot() starting drawing"<<std::endl;\
	//BaseUIElement::clearNextDraw = true;
	//BaseUIElement::redraw();

    for (int i=0; i<nWaveAx; i++)
        wAxes[i].redraw();
//    wAxes[1].redraw();
    for (int i=0; i<nProjAx; i++)
        pAxes[i].redraw();
}

// This would normally happen for collection of axes but an electrode plot doesn't have a collection instead its a single axes
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
//    wAxes[1] = WaveAxes(0, 0, 1, 1, WAVE2);
//   wAxes[0].setWaveformColor(1.0, 1.0, 1.0);
//    wAxes[1].setWaveformColor(1.0, 1.0, 1.0);
    
    for (int i=0; i<nProjAx; i++){
        pAxes[i] = ProjectionAxes(0, 0, 1, 1, PROJ1x2 + i);
        pAxes[i].setPointColor(1.0, 1.0, 1.0);
    }

    updateAxesPositions();
    setLimitsOnAxes();
}
void SpikePlot::updateAxesPositions(){
    int minX = BaseUIElement::xpos;
	int minY = BaseUIElement::ypos;
	
    
    /*
     * This code is BUGGY it doesn't scale with the number of plot dimensions!  
     * Need to split it into a switch case
     */
	double axesWidth;// = BaseUIElement::width/2;
	double axesHeight;// = BaseUIElement::height;
	
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
        wAxes[i].setYLims(limits[0][0], limits[0][1]);

    for (int i=0; i<nProjAx; i++){
        pAxes[i].setYLims(limits[0][0], limits[0][1]);
        pAxes[i].setXLims(limits[1][0], limits[1][1]);
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
        limits[i][0] = 2209;//-1*pow(2,11);
        limits[i][1] = 11059;//pow(2,14)*1.6;
    }

}

void SpikePlot::getPreferredDimensions(double *w, double *h){
    *w = 150;
    *h = 75;
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
    if (dim>1 || dim<0)
        return;
    
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

    if (dim>1 || dim<0)
        return;
    
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


