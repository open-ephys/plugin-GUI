#include "StereotrodePlot.h"
#include "../SpikeObject.h"
#include "PlotUtils.h"

StereotrodePlot::StereotrodePlot():
	BaseUIElement(),  limitsChanged(true)
{

}

StereotrodePlot::StereotrodePlot(int x, int y, int w, int h):
	BaseUIElement(x,y,w,h,0), limitsChanged(true)
{

	initAxes();
}

StereotrodePlot::~StereotrodePlot(){
}

// Each plot needs to update its children axes when its redraw gets called.
//  it also needs to call the parent plot  when children axes get added it
//  should place them in the correct location because it KNOWS where WAVE1 and PROJ1x3
//  should go by default. This isn't as general as it should be but its a good push in
//  the right direction

void StereotrodePlot::redraw(){
	//std::cout<<"StereotrodePlot() starting drawing"<<std::endl;\
	//BaseUIElement::clearNextDraw = true;
	//BaseUIElement::redraw();

    for (int i=0; i<nWaveAx; i++)
        wAxes[i].redraw();
//    wAxes[1].redraw();
    for (int i=0; i<nProjAx; i++)
        pAxes[i].redraw();
}

// This would normally happen for collection of axes but an electrode plot doesn't have a collection instead its a single axes
void StereotrodePlot::processSpikeObject(SpikeObject s){
	//std::cout<<"ElectrdePlot::processSpikeObject()"<<std::endl;
    for (int i=0; i<nWaveAx; i++)
        wAxes[i].updateSpikeData(s);
//    wAxes[1].updateSpikeData(s);
    for (int i=0; i<nProjAx; i++)
        pAxes[i].updateSpikeData(s);
}

void StereotrodePlot::setEnabled(bool e){
	BaseUIElement::enabled = e;
    
    for (int i=0; i<nWaveAx; i++)
        wAxes[i].setEnabled(e);
//    wAxes[1].setEnabled(e);
    for (int i=0; i<nProjAx; i++)
        pAxes[i].setEnabled(e);
}

bool StereotrodePlot::getEnabled(){
	return BaseUIElement::enabled;
}


void StereotrodePlot::initAxes(){
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
void StereotrodePlot::updateAxesPositions(){
    int minX = BaseUIElement::xpos;
	int minY = BaseUIElement::ypos;
	
	double axesWidth = BaseUIElement::width/2;
	double axesHeight = BaseUIElement::height;
	
    for (int i=0; i<nWaveAx; i++)
        wAxes[i].setPosition(minX + (i%2) * axesWidth/2, minY + (i/2) * axesHeight, axesWidth/2, axesHeight);
//    wAxes[1].setPosition(minX + axesWidth/2, minY, axesWidth/2, axesHeight);
    for (int i=0; i<nProjAx; i++)
        pAxes[0].setPosition(minX + (1 + i%2) * axesWidth, minY + (i/2) * axesHeight, axesWidth, axesHeight);
}

void StereotrodePlot::setLimitsOnAxes(){
    std::cout<<"StereotrodePlot::setLimitsOnAxes()"<<std::endl;
    
    for (int i=0; i<nWaveAx; i++)
        wAxes[i].setYLims(limits[0][0], limits[0][1]);
//    wAxes[1].setYLims(limits[1][0], limits[1][1]);
    for (int i=0; i<nProjAx; i++){
        pAxes[i].setYLims(limits[0][0], limits[0][1]);
        pAxes[i].setXLims(limits[1][0], limits[1][1]);
    }
    

}
void StereotrodePlot::setPosition(int x, int y, double w, double h){
    
//    std::cout<<"StereotrodePlot::setPosition()"<<std::endl;
	BaseUIElement::setPosition(x,y,w,h);
	updateAxesPositions();
    
}

int StereotrodePlot::getNumberOfAxes(){
	return 3;
}

void StereotrodePlot::initLimits(){
    for (int i=0; i<nChannel; i++)
    {
        limits[i][0] = 3705;//-1*pow(2,11);
        limits[i][1] = 6201;//pow(2,14)*1.6;
    }
}

void StereotrodePlot::getPreferredDimensions(double *w, double *h){
    *w = 150;
    *h = 75;
}

void StereotrodePlot::clear(){
    std::cout<<"StereotrodePlot::clear()"<<std::endl;
    pAxes[0].clear();
}


bool StereotrodePlot::processKeyEvent(SimpleKeyEvent k){

    return true;
}

void StereotrodePlot::pan(int dim, bool up){

    std::cout<<"StereotrodePlot::pan() dim:"<<dim<<std::endl;
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
void StereotrodePlot::zoom(int dim, bool in){
    std::cout<<"StereotrodePlot::zoom()"<<std::endl;

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


