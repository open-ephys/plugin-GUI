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
    pAxes.redraw();
}

// This would normally happen for collection of axes but an electrode plot doesn't have a collection instead its a single axes
void StereotrodePlot::processSpikeObject(SpikeObject s){
	//std::cout<<"ElectrdePlot::processSpikeObject()"<<std::endl;
	wAxes[0].updateSpikeData(s);
    wAxes[1].updateSpikeData(s);
    pAxes.updateSpikeData(s);
}

void StereotrodePlot::setEnabled(bool e){
	BaseUIElement::enabled = e;

	wAxes[0].setEnabled(e);
    wAxes[1].setEnabled(e);
    pAxes.setEnabled(e);
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
    pAxes = ProjectionAxes(minX + axesWidth, minY, axesWidth, axesHeight, PROJ1x2);
	
    //axes.setEnabled(false);
	wAxes[0].setYLims(-1*pow(2,11), pow(2,14)*1.6);
    wAxes[1].setYLims(-1*pow(2,11), pow(2,14)*1.6);
    pAxes.setYLims(-1*pow(2,11), pow(2,14)*1.6);
    pAxes.setXLims(-1*pow(2,11), pow(2,14)*1.6);
	
    wAxes[0].setWaveformColor(1.0, 1.0, 1.0);
    wAxes[1].setWaveformColor(1.0, 1.0, 1.0);
    pAxes.setPointColor(1.0, 1.0, 1.0);

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
    pAxes.setPosition(minX + axesWidth, minY, axesWidth, axesHeight);
}

int StereotrodePlot::getNumberOfAxes(){
	return 1;;
}

void StereotrodePlot::initLimits(){
    for (int i=0; i<4; i++)
    {
        limits[i][0] = -1*pow(2,11);
        limits[i][1] = pow(2,14);
    }

}

void StereotrodePlot::getPreferredDimensions(double *w, double *h){
    *w = 75;
    *h = 75;
}

void StereotrodePlot::clear(){
    std::cout<<"StereotrodePlot::clear()"<<std::endl;
    pAxes.clear();
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
}


