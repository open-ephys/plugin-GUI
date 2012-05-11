#include "TetrodePlot.h"
#include "../SpikeObject.h"
#include "PlotUtils.h"

TetrodePlot::TetrodePlot():
    BaseUIElement(), limitsChanged(true)
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
    BaseUIElement::clearNextDraw = true;
   
    BaseUIElement::redraw();
   
    for (int i=0; i<4; i++)
        wAxes[i].redraw();

    for (int i=0; i<6; i++)
        pAxes[i].redraw();
}

// This would normally happen for collection of axes but an electrode plot doesn't have a collection instead its a single axes
void TetrodePlot::processSpikeObject(SpikeObject s){
    //std::cout<<"ElectrdePlot::processSpikeObject()"<<std::endl;
    for (int i=0; i<4; i++)
        wAxes[i].updateSpikeData(s);
    for (int i=0; i<6; i++)
        pAxes[i].updateSpikeData(s);
}

void TetrodePlot::setEnabled(bool e){
    BaseUIElement::enabled = e;
    for (int i=0; i<4; i++)
        wAxes[i].setEnabled(e);
    for (int i=0; i<6; i++)
        pAxes[i].setEnabled(e);
}

bool TetrodePlot::getEnabled(){
    return BaseUIElement::enabled;
}


void TetrodePlot::initAxes(){
    initLimits();
    
    int minX = BaseUIElement::xpos;
    int minY = BaseUIElement::ypos;
   
    int axesWidth = BaseUIElement::width/4; 
    int axesHeight = BaseUIElement::height/2;
    
    
    wAxes[0] = WaveAxes(minX, minY, axesWidth/2, axesHeight, WAVE1);
    wAxes[1] = WaveAxes(minX + axesWidth/2, minY, axesWidth/2, axesHeight, WAVE2);
    wAxes[2] = WaveAxes(minX, minY + axesHeight, axesWidth/2, axesHeight, WAVE3);
    wAxes[3] = WaveAxes(minX + axesWidth/2, minY + axesHeight, axesWidth/2, axesHeight, WAVE4);

    // for (int i=0; i<6; i++)
    // {
    //     pAxes[i] = ProjectionAxes(
    //                     minX + axesWidth * ((i%3)+1),
    //                     minY + (i/3) * axesHeight, 
    //                     axesWidth , 
    //                     axesHeight,
    //                     i);
    //}
    // 

    // because the width of the plot doesn't always divide evenly into 4 we are going to pad the width 
    // of the 3rd and 6th projection plot to cover up the space cleared by the TetrodePlotElement

    int xPad = 1;
    pAxes[0] = ProjectionAxes(minX + axesWidth*1, minY,     axesWidth, axesHeight, PROJ1x2);
    pAxes[1] = ProjectionAxes(minX + axesWidth*2, minY,     axesWidth, axesHeight, PROJ1x3);
    pAxes[2] = ProjectionAxes(minX + axesWidth*3, minY,     axesWidth+xPad, axesHeight, PROJ1x4);
    pAxes[3] = ProjectionAxes(minX + axesWidth*1, minY + axesHeight,    axesWidth, axesHeight, PROJ2x3);
    pAxes[4] = ProjectionAxes(minX + axesWidth*2, minY + axesHeight,    axesWidth, axesHeight, PROJ2x4);
    pAxes[5] = ProjectionAxes(minX + axesWidth*3, minY + axesHeight,    axesWidth+xPad, axesHeight, PROJ3x4);

    
    //axes.setEnabled(false);
    for (int i=0; i<4; i++){
        wAxes[i].setYLims(-1*pow(2,11), pow(2,14)*1.6);
        wAxes[i].setWaveformColor(1.0, 1.0, 1.0);
    }
    for (int i=0; i<6; i++)
    {
        pAxes[i].setXLims(-1*pow(2,11), pow(2,14)*1.6);
        pAxes[i].setYLims(-1*pow(2,11), pow(2,14)*1.6);
        pAxes[i].setPointColor(1.0, 1.0, 1.0);
    }

}

void TetrodePlot::setLimitsOnAxes(){
    std::cout<<"TetrodePlot::setLimitsOnAxes()"<<std::endl;
    
    wAxes[0].setYLims(limits[0][0], limits[0][1]);
    wAxes[1].setYLims(limits[1][0], limits[1][1]);
    wAxes[2].setYLims(limits[0][0], limits[0][1]);
    wAxes[3].setYLims(limits[1][0], limits[1][1]);

    for (int i=0; i<6; i++)
    {
        pAxes[i].setYLims(limits[0][0], limits[0][1]);
        pAxes[i].setXLims(limits[1][0], limits[1][1]);
    }
    

}

void TetrodePlot::setPosition(int x, int y, double w, double h){
    BaseUIElement::setPosition(x,y,w,h);
    int minX = BaseUIElement::xpos;
    int minY = BaseUIElement::ypos;
    
    int axesWidth = BaseUIElement::width/4;
    int axesHeight = BaseUIElement::height/2;
    
    wAxes[0].setPosition(minX, minY, axesWidth/2, axesHeight);
    wAxes[1].setPosition(minX + axesWidth/2, minY, axesWidth/2, axesHeight);
    wAxes[2].setPosition(minX, minY + axesHeight, axesWidth/2, axesHeight);
    wAxes[3].setPosition(minX + axesWidth/2, minY + axesHeight, axesWidth/2, axesHeight);

    // pAxes[0].setPosition(minX + axesWidth*1, minY, axesWidth, axesHeight);
    // pAxes[1].setPosition(minX + axesWidth*2, minY, axesWidth, axesHeight);
    // pAxes[2].setPosition(minX + axesWidth*3, minY, axesWidth, axesHeight);
    // pAxes[3].setPosition(minX + axesWidth*1, minY + axesHeight, axesWidth, axesHeight);
    // pAxes[4].setPosition(minX + axesWidth*2, minY + axesHeight, axesWidth, axesHeight);
    // pAxes[5].setPosition(minX + axesWidth*3, minY + axesHeight, axesWidth, axesHeight);

    //titleBox.setPosition(x, y+h-titleHeight-3, w, titleHeight+3);
}

// void TetrodePlot::setPosition(int x, int y){
//     setPosition(x,y, BaseUIElement::width, BaseUIElement::height );
// }

// void TetrodePlot::setDimensions(double w, double h){
//     setPosition(BaseUIElement::xpos, BaseUIElement::ypos, w, h);
// }

int TetrodePlot::getNumberOfAxes(){
    return 4;
}


void TetrodePlot::initLimits(){
    for (int i=0; i<4; i++)
    {
        limits[i][0] = -1*pow(2,11);
        limits[i][1] = pow(2,14);
    }

}

void TetrodePlot::getPreferredDimensions(double *w, double *h){
    *w = 75;
    *h = 75;
}
// void TetrodePlot::mouseDown(int x, int y){

// //     selectedAxesN = -1;
// //     std::list<GenericAxes>::iterator i;
// //     int idx=-1;
// //     bool hit = false;

// //     selectedAxes = NULL;
// //     for (i=axesList.begin(); i!=axesList.end(); ++i)
// //     {
// //         if (i->hitTest(x,y))
// //         {
// //             selectedAxes = addressof(*i);
// //             selectedAxesN = i->getType();
// //             hit = true;
// // //            break;
// //         }
// //         idx++;
// //     }
// //     if (!hit)
// //         selectedAxes = NULL;
// //     if (selectedAxes != NULL)
// //         std::cout<<"TetrodePlot::mouseDown() hit:"<<selectedAxes<<" AxesType:"<<selectedaxes.getType()<<std::endl;
// //     else
// //         std::cout<<"TetrodePlot::mouseDown() NO HIT!"<<std::endl;
    
// }
// void TetrodePlot::mouseDragX(int dx, bool shift, bool ctrl){

// //     if (selectedAxes == NULL || dx==0)
// //         return;
// // //    zoomAxes(selectedaxes.getType(), true, dx>0);
// //     if (shift)
// //         zoomAxes(selectedAxesN, true, dx);
// //     if (ctrl)
// //         panAxes(selectedAxesN, true, dx);

// }
// void TetrodePlot::mouseDragY(int dy, bool shift, bool ctrl){
//     if (selectedAxes == NULL || dy==0)
//         return;
//     if(shift)
//         zoomAxes(selectedAxesN, false, dy);
//     if(ctrl)
//         panAxes(selectedAxesN, false, dy);
// }

// void TetrodePlot::zoomAxes(int n, bool xdim, int zoom){
// //    std::cout<<"TetrodePlot::zoomAxes() n:"<< n<<" xdim"<< xdim<<" in:"<<zoomin<<std::endl;
//     // If trying to zoom an invalid axes type
//     if (n<WAVE1 || n>PROJ3x4)
//         return;
//     if (n<=WAVE4)
//         zoomWaveform(n, xdim, zoom);
//     else
//         zoomProjection(n, xdim, zoom);
// }

// void TetrodePlot::zoomWaveform(int n, bool xdim, int zoom){

//     // waveform plots don't have a xlimits
//     if (xdim)
//         return;
// //    std::cout<<"Zooming Waveform:"<<n<<" zoomin:"<<zoomin<<" ";
//     double min, max;
    
//     if(xdim)
//         return;

//     min = limits[n][0];
//     max = limits[n][1];
    
//     double mean = (max + min)/2.0f;
//     double delta = max - mean;
//     delta = delta / pow(.99, -1*zoom);

//     min = mean - delta;
//     max = mean + delta;

//     limits[n][0] = min;
//     limits[n][1] = max;
    
//     limitsChanged = true;
// }

// void TetrodePlot::panAxes(int n, bool xdim, int panval){
//     //    std::cout<<"TetrodePlot::zoomAxes() n:"<< n<<" xdim"<< xdim<<" in:"<<zoomin<<std::endl;
//     // If trying to zoom an invalid axes type
//     if (n<WAVE1 || n>PROJ3x4)
//         return;
//     if (n<=WAVE4)
//         panWaveform(n, xdim, panval);
//     else
//         panProjection(n, xdim, panval);
// }

// void TetrodePlot::panWaveform(int n, bool xdim, int pan){
    
//     // waveform plots don't have a xlimits
//     if (xdim)
//         return;
//     //    std::cout<<"Panning Waveform:"<<n<<" pan:"<<pan<<" "<<std::endl;
//     double min, max;
    
//     if(xdim)
//         return;
    
//     min = limits[n][0];
//     max = limits[n][1];
    
//     double dy = max-min;
    
//     // Need to grab something if pan event is driven by the keyboard, which means that all the plots are getting panned so this should be okay
//     if (selectedAxes == NULL)
//         selectedAxes = &axesList.front();
    
//     double yPixels = (BaseUIElement::height - titleHeight)/2.0;
    
//     double pixelWidth = -1 * dy/yPixels;
    
//     double delta = pan * pixelWidth;
//     min = min + delta;
//     max = max + delta;
    
//     limits[n][0] = min;
//     limits[n][1] = max;
    
//     limitsChanged = true;
// }



bool TetrodePlot::processKeyEvent(SimpleKeyEvent k){
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


void TetrodePlot::clear(){
    std::cout<<"TetrodePlot::clear()"<<std::endl;
    
    for (int i = 0; i < 6; i++)
        pAxes[i].clear();
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

