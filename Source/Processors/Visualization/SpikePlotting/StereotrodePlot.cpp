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

	wAxes[0].redraw();
    wAxes[1].redraw();
    pAxes[0].redraw();
}

// This would normally happen for collection of axes but an electrode plot doesn't have a collection instead its a single axes
void StereotrodePlot::processSpikeObject(SpikeObject s){
	//std::cout<<"ElectrdePlot::processSpikeObject()"<<std::endl;
	wAxes[0].updateSpikeData(s);
    wAxes[1].updateSpikeData(s);
    pAxes[0].updateSpikeData(s);
}

void StereotrodePlot::setEnabled(bool e){
	BaseUIElement::enabled = e;

	wAxes[0].setEnabled(e);
    wAxes[1].setEnabled(e);
    pAxes[0].setEnabled(e);
}

bool StereotrodePlot::getEnabled(){
	return BaseUIElement::enabled;
}


void StereotrodePlot::initAxes(){
	initLimits();
    
    int minX = BaseUIElement::xpos;
	int minY = BaseUIElement::ypos;
	
	double axesWidth = BaseUIElement::width/2;
	double axesHeight = BaseUIElement::height;
	
	
	wAxes[0] = WaveAxes(minX, minY, axesWidth/2, axesHeight, WAVE1);
    wAxes[1] = WaveAxes(minX + axesWidth/2, minY, axesWidth/2, axesHeight, WAVE2);
    wAxes[0].setWaveformColor(1.0, 1.0, 1.0);
    wAxes[1].setWaveformColor(1.0, 1.0, 1.0);
    
    pAxes[0] = ProjectionAxes(minX + axesWidth, minY, axesWidth, axesHeight, PROJ1x2);
    pAxes[0].setPointColor(1.0, 1.0, 1.0);

    setLimitsOnAxes();
}

void StereotrodePlot::setLimitsOnAxes(){
    std::cout<<"StereotrodePlot::setLimitsOnAxes()"<<std::endl;
    
    wAxes[0].setYLims(limits[0][0], limits[0][1]);
    wAxes[1].setYLims(limits[1][0], limits[1][1]);
    pAxes[0].setYLims(limits[0][0], limits[0][1]);
    pAxes[0].setXLims(limits[1][0], limits[1][1]);
    

}
void StereotrodePlot::setPosition(int x, int y, double w, double h){
    
//    std::cout<<"StereotrodePlot::setPosition()"<<std::endl;
	BaseUIElement::setPosition(x,y,w,h);
	int minX = BaseUIElement::xpos;
	int minY = BaseUIElement::ypos;
	
	double axesWidth = BaseUIElement::width/2;
	double axesHeight = BaseUIElement::height;
	
    wAxes[0].setPosition(minX, minY, axesWidth/2, axesHeight);
    wAxes[1].setPosition(minX + axesWidth/2, minY, axesWidth/2, axesHeight);	
    pAxes[0].setPosition(minX + axesWidth, minY, axesWidth, axesHeight);
}

int StereotrodePlot::getNumberOfAxes(){
	return 3;
}

void StereotrodePlot::initLimits(){
    for (int i=0; i<2; i++)
    {
        limits[i][0] = -1*pow(2,11);
        limits[i][1] = pow(2,14)*1.6;
    }

}

void StereotrodePlot::getPreferredDimensions(double *w, double *h){
    *w = 75;
    *h = 75;
}

void StereotrodePlot::clear(){
    std::cout<<"StereotrodePlot::clear()"<<std::endl;
    pAxes[0].clear();
}


bool StereotrodePlot::processKeyEvent(SimpleKeyEvent k){
    // std::cout<<"Key:"<<(char)k.key<<std::endl;
    // switch(k.key)
    // {
    //     case '=':
    //         for (int i=0; i<=WAVE4; i++)
    //             zoomWaveform(i, false, 3);
    //         break;        
    //     case '+':
    //         for (int i=0; i<=WAVE4; i++)
    //             panWaveform(i, false, 3);
    //         break;
    //     case '-':
    //         for (int i=0; i<=WAVE4; i++)
    //             zoomWaveform(i, false, -3);
    //         break;    
    //     case '_':
    //         for (int i=0; i<=WAVE4; i++)
    //             panWaveform(i, false, -3);
    //         break;
    //     case 'C':
    //         clearOnNextDraw(true);
    //         break;
    // }
    
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


