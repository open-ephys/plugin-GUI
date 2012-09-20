#include "TetrodePlot.h"
#include "../SpikeObject.h"
#include "PlotUtils.h"

TetrodePlot::TetrodePlot():
    BaseUIElement(),  limitsChanged(true)
{

}

TetrodePlot::TetrodePlot(int x, int y, int w, int h):
    BaseUIElement(x,y,w,h,0), limitsChanged(true)
{

    initAxes();
}

TetrodePlot::~TetrodePlot(){
}

// Each plot needs to update its children axes when its redraw gets called.
//  it also needs to call the parent plot  when children axes get added it
//  should place them in the correct location because it KNOWS where WAVE1 and PROJ1x3
//  should go by default. This isn't as general as it should be but its a good push in
//  the right direction

void TetrodePlot::redraw(){
    //std::cout<<"TetrodePlot() starting drawing"<<std::endl;\
    //BaseUIElement::clearNextDraw = true;
    //BaseUIElement::redraw();
    for (int i = 0; i < 4; i++)
        wAxes[i].redraw();
    for (int i = 0; i < 6; i++)
        pAxes[i].redraw();
}

// This would normally happen for collection of axes but an electrode plot doesn't have a collection instead its a single axes
void TetrodePlot::processSpikeObject(SpikeObject s){
    //std::cout<<"ElectrdePlot::processSpikeObject()"<<std::endl;
    for (int i = 0; i < 4; i++)
        wAxes[i].updateSpikeData(s);
    for (int i = 0; i < 6; i++)
        pAxes[i].updateSpikeData(s);
}

void TetrodePlot::setEnabled(bool e){
    BaseUIElement::enabled = e;

    for (int i = 0; i < 4; i++)
        wAxes[i].setEnabled(e);
    for (int i = 0; i < 6; i++)
        pAxes[i].setEnabled(e);
}

bool TetrodePlot::getEnabled(){
    return BaseUIElement::enabled;
}


void TetrodePlot::initAxes(){
    initLimits();
    
    int minX = BaseUIElement::xpos;
    int minY = BaseUIElement::ypos;
    
    double axesWidth = BaseUIElement::width/4;
    double axesHeight = BaseUIElement::height/2;
    
    // wAxes[0] = WaveAxes(minX, minY, axesWidth/2, axesHeight, WAVE1);
    // wAxes[1] = WaveAxes(minX + axesWidth/2, minY, axesWidth/2, axesHeight, WAVE2);
    // wAxes[2] = WaveAxes(minX, minY + axesHeight, axesWidth/2, axesHEight, WAVE3);
    // wAxes[3] = WaveAxes(minX + axesWidth/2, minY + axesHeight, axesWidth/2, axesHEight, WAVE4);

    for (int i=0;  i<4; i++)
        wAxes[i] = WaveAxes(minX + (i/2)*(axesWidth/2), minY + (i%2)*(axesHeight), axesWidth/2, axesHeight, WAVE1 + i);

    for (int i=0; i<6; i++)    
        pAxes[i] = ProjectionAxes(minX + (1 + i/2)*axesWidth, minY + (i%2)*(axesHeight), axesWidth, axesHeight, PROJ1x2+i);

    for (int i=0; i<4; i++)
        wAxes[i].setWaveformColor(1.0, 1.0, 1.0);

    for (int i=0; i<6; i++)
        pAxes[i].setPointColor(1.0, 1.0, 1.0);

    setLimitsOnAxes();
}

void TetrodePlot::setLimitsOnAxes(){
    std::cout<<"TetrodePlot::setLimitsOnAxes()"<<std::endl;
    
    for (int i=0; i<4; i++)
        wAxes[i].setYLims(limits[0][0], limits[0][1]);

    for (int i=0; i<6; i++){
        pAxes[i].setYLims(limits[0][0], limits[0][1]);
        pAxes[i].setXLims(limits[1][0], limits[1][1]);
    }
}
void TetrodePlot::setPosition(int x, int y, double w, double h){
    
//    std::cout<<"TetrodePlot::setPosition()"<<std::endl;
    BaseUIElement::setPosition(x,y,w,h);
    int minX = BaseUIElement::xpos;
    int minY = BaseUIElement::ypos;
    
    double axesWidth = BaseUIElement::width/4;
    double axesHeight = BaseUIElement::height/2;

    for (int i=0;  i<4; i++)
        wAxes[i] = WaveAxes(minX + (i/2)*(axesWidth/2), minY + (i%2)*(axesHeight), axesWidth/2, axesHeight, WAVE1 + i);

    for (int i=0; i<6; i++)    
        pAxes[i] = ProjectionAxes(minX + (1 + i/2)*axesWidth, minY + (i%2)*(axesHeight), axesWidth, axesHeight, PROJ1x2+i);
}

int TetrodePlot::getNumberOfAxes(){
    return 10;
}

void TetrodePlot::initLimits(){
    for (int i=0; i<2; i++)
    {
        limits[i][0] = -1*pow(2,11);
        limits[i][1] = pow(2,14)*1.6;
    }

}

void TetrodePlot::getPreferredDimensions(double *w, double *h){
    *w = 75;
    *h = 75;
}

void TetrodePlot::clear(){
    std::cout<<"TetrodePlot::clear()"<<std::endl;
    for (int i=0; i<6; i++)
        pAxes[i].clear();
}


bool TetrodePlot::processKeyEvent(SimpleKeyEvent k){

    return true;
}

void TetrodePlot::pan(int dim, bool up){

    std::cout<<"TetrodePlot::pan() dim:"<<dim<<std::endl;
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
void TetrodePlot::zoom(int dim, bool in){
    std::cout<<"TetrodePlot::zoom()"<<std::endl;

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


