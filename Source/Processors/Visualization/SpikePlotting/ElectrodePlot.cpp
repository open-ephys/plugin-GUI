#include "ElectrodePlot.h"
#include "../SpikeObject.h"
#include "PlotUtils.h"

ElectrodePlot::ElectrodePlot():
	BaseUIElement(), limitsChanged(true)
{
}

ElectrodePlot::ElectrodePlot(int x, int y, int w, int h):
	BaseUIElement(x,y,w,h,1),  limitsChanged(true)
{

	initAxes();
}

ElectrodePlot::~ElectrodePlot(){
}

// Each plot needs to update its children axes when its redraw gets called.
//  it also needs to call the parent plot  when children axes get added it
//  should place them in the correct location because it KNOWS where WAVE1 and PROJ1x3
//  should go by default. This isn't as general as it should be but its a good push in
//  the right direction

void ElectrodePlot::redraw(){
	 //std::cout<<"ElectrodePlot() starting drawing"<<std::endl;\
	BaseUIElement::clearNextDraw = true;
	BaseUIElement::redraw();

	axes.redraw();
}

// This would normally happen for collection of axes but an electrode plot doesn't have a collection instead its a single axes
void ElectrodePlot::processSpikeObject(SpikeObject s){
	//std::cout<<"ElectrdePlot::processSpikeObject()"<<std::endl;
	axes.updateSpikeData(s);
}

void ElectrodePlot::setEnabled(bool e){
	BaseUIElement::enabled = e;

	axes.setEnabled(e);
}

bool ElectrodePlot::getEnabled(){
	return BaseUIElement::enabled;
}


void ElectrodePlot::initAxes(){
	initLimits();
    
    int minX = BaseUIElement::xpos;
	int minY = BaseUIElement::ypos;
	
	double axesWidth = BaseUIElement::width;
	double axesHeight = BaseUIElement::height;
	
	
	axes = WaveAxes(minX, minY, axesWidth, axesHeight, WAVE1);
	
    //axes.setEnabled(false);
	axes.setYLims(-1*pow(2,11), pow(2,14)*1.6);
	axes.setWaveformColor(1.0, 1.0, 1.0);

}

void ElectrodePlot::setPosition(int x, int y, double w, double h){
	BaseUIElement::setPosition(x,y,w,h);
	int minX = BaseUIElement::xpos;
	int minY = BaseUIElement::ypos;
	
	double axesWidth = BaseUIElement::width;
	double axesHeight = BaseUIElement::height;
	
	axes.setPosition(minX, minY, axesWidth, axesHeight);
	
}

int ElectrodePlot::getNumberOfAxes(){
	return 1;;
}

void ElectrodePlot::clearOnNextDraw(bool b){
	BaseUIElement::clearNextDraw = b;
}

void ElectrodePlot::initLimits(){
    for (int i=0; i<4; i++)
    {
        limits[i][0] = -1*pow(2,11);
        limits[i][1] = pow(2,14);
    }

}

void ElectrodePlot::getPreferredDimensions(double *w, double *h){
    *w = 75;
    *h = 75;
}
// void ElectrodePlot::mouseDown(int x, int y){

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
// //         std::cout<<"ElectrodePlot::mouseDown() hit:"<<selectedAxes<<" AxesType:"<<selectedaxes.getType()<<std::endl;
// //     else
// //         std::cout<<"ElectrodePlot::mouseDown() NO HIT!"<<std::endl;
    
// }
// void ElectrodePlot::mouseDragX(int dx, bool shift, bool ctrl){

// //     if (selectedAxes == NULL || dx==0)
// //         return;
// // //    zoomAxes(selectedaxes.getType(), true, dx>0);
// //     if (shift)
// //         zoomAxes(selectedAxesN, true, dx);
// //     if (ctrl)
// //         panAxes(selectedAxesN, true, dx);

// }
// void ElectrodePlot::mouseDragY(int dy, bool shift, bool ctrl){
//     if (selectedAxes == NULL || dy==0)
//         return;
//     if(shift)
//         zoomAxes(selectedAxesN, false, dy);
//     if(ctrl)
//         panAxes(selectedAxesN, false, dy);
// }

// void ElectrodePlot::zoomAxes(int n, bool xdim, int zoom){
// //    std::cout<<"ElectrodePlot::zoomAxes() n:"<< n<<" xdim"<< xdim<<" in:"<<zoomin<<std::endl;
//     // If trying to zoom an invalid axes type
//     if (n<WAVE1 || n>PROJ3x4)
//         return;
//     if (n<=WAVE4)
//         zoomWaveform(n, xdim, zoom);
//     else
//         zoomProjection(n, xdim, zoom);
// }

// void ElectrodePlot::zoomWaveform(int n, bool xdim, int zoom){

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

// void ElectrodePlot::panAxes(int n, bool xdim, int panval){
//     //    std::cout<<"ElectrodePlot::zoomAxes() n:"<< n<<" xdim"<< xdim<<" in:"<<zoomin<<std::endl;
//     // If trying to zoom an invalid axes type
//     if (n<WAVE1 || n>PROJ3x4)
//         return;
//     if (n<=WAVE4)
//         panWaveform(n, xdim, panval);
//     else
//         panProjection(n, xdim, panval);
// }

// void ElectrodePlot::panWaveform(int n, bool xdim, int pan){
    
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



bool ElectrodePlot::processKeyEvent(SimpleKeyEvent k){
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


