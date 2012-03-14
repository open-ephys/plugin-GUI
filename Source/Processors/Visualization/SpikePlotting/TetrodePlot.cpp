#include "TetrodePlot.h"

TetrodePlot::TetrodePlot():
BaseUIElement(),
titleHeight(15),
enableTitle(true), 
limitsChanged(true){
	plotTitle = (char*) "TetrodePlot";
	//titleBox = ArteTitleBox(100-titleHeight,0,2,titleHeight, plotTitle);
//	BaseUIElement::elementName = (char*) "ArtePlot";
	
}
TetrodePlot::TetrodePlot(int x, int y, int w, int h, char *n):
BaseUIElement(x,y,w,h,1), 
titleHeight(15),
enableTitle(true), 
limitsChanged(true){
	plotTitle = n;

	titleBox = TitleBox(x, y+h-titleHeight-3, w, titleHeight+3, plotTitle);
//	std::cout<< x+h-titleHeight << "," << y << " - " << w << "," <<titleHeight<<std::endl;
//	BaseUIElement::elementName = (char*) "ArtePlot";
}


// Each plot needs to update its children axes when its redraw gets called.
//  it also needs to call the parent plot  when children axes get added it
//  should place them in the correct location because it KNOWS where WAVE1 and PROJ1x3
//  should go by default. This isn't as general as it should be but its a good push in
//  the right direction

void TetrodePlot::redraw(){
	// std::cout<<"TetrodePlot() starting drawing"<<std::endl;
	BaseUIElement::redraw();
    
	// SpikeObject tempSpike;
	// std::list<GenericAxes>::iterator i;
	// bool axesDrawnOnce = false;
	// while(tetSource.getNextSpike(&tempSpike)){
	// 	axesDrawnOnce = true;	
	// 	for (i=axesList.begin(); i!= axesList.end(); ++i){
	// 		i->updateSpikeData(tempSpike);
       
 //            if (limitsChanged){

 //       			int n = i->getType();
                
 //                if (n>=WAVE1 && n<=WAVE4)
 //                    i->setYLims(limits[n][0], limits[n][1]);
                
 //                else if( n>=PROJ1x2 && n<=PROJ3x4){
 //                    int d1, d2;
 //                    n2ProjIdx(i->getType(), &d1, &d2);
 //                    i->setXLims(limits[d1][0], limits[d1][1]);
 //                    i->setYLims(limits[d2][0], limits[d2][1]);
 //                }
 //            }
 //            i->redraw();

	// 	}
 //        if (limitsChanged)
 //            limitsChanged = false;
	// }

	// if (!axesDrawnOnce)
	// 	for (i= axesList.begin(); i!=axesList.end(); ++i)
	// 		i->redraw();

	// titleBox.redraw();
	// BaseUIElement::drawElementEdges();
	// // std::cout<<"TetrodePlot() Done drawing"<<std::endl;
}

void TetrodePlot::setTitle(char *n){
	plotTitle = n;
}

void TetrodePlot::setEnabled(bool e){
	BaseUIElement::enabled = e;
	std::list<GenericAxes>::iterator i;
	for (i = axesList.begin(); i!= axesList.end(); ++i){
		i->setEnabled(e);
	}
}

bool TetrodePlot::getEnabled(){
	return BaseUIElement::enabled;
}


void TetrodePlot::initAxes(){
	initLimits();
    
    int minX = BaseUIElement::xpos;
	int minY = BaseUIElement::ypos;
	
	double axesWidth = BaseUIElement::width/4.0;
	double axesHeight = (BaseUIElement::height - titleHeight)/2.0;
	
	int axX, axY;
	
	if (!axesList.empty()){
		std::list<GenericAxes> tmp;
		axesList = tmp;
	}

	for (int i=WAVE1; i<=WAVE4; i++) // macro constants from GenericAxes.h
	{
		// divide axes width by 2 because wave plots are half of a projection plot width
		axX = minX  + axesWidth/2 * ( i % 2);
		axY = minY +  axesHeight * ( i / 2);
		WaveAxes ax = WaveAxes(axX, axY, axesWidth/2, axesHeight, i);
		ax.setEnabled(false);
		ax.setYLims(-1*pow(2,11), pow(2,14));
		axesList.push_back(ax);
        switch(i){
            case 0:
                ax.setWaveformColor(1.0, 0.0, 0.0);
                break;
            case 1:
                ax.setWaveformColor(0.0, 1.0, 0.0);
                break;
            case 2:
                ax.setWaveformColor(0.0, 0.0, 1.0);
                break;
            case 3:
                ax.setWaveformColor(1.0, 1.0, 0.0);
                break;
        }
	}

	for (int i=PROJ1x2; i<=PROJ3x4; i++){
		// use i2 instead of i so we can index from 0 instead of 4 for the plot placement
		int i2 = i - PROJ1x2;
		axX = minX + axesWidth *  (i2%3 + 1); // add 1 to offset for the waveform plots
		axY = minY + axesHeight * (i2 / 3); // no need to offset for the y direction

		GenericAxes ax = GenericAxes(axX, axY, axesWidth, axesHeight, i);
		ax.setEnabled(true);
        int d1, d2;
        n2ProjIdx(i, &d1, &d2);
		ax.setXLims(limits[d1][0], limits[d1][1]);
		ax.setYLims(limits[d1][0], limits[d1][1]);
//        ax.setXLims(-1*pow(2,11), pow(2,14));
//		ax.setYLims(-1*pow(2,11), pow(2,14));

		axesList.push_back(ax);
	}
}

void TetrodePlot::setPosition(int x, int y, double w, double h){
	BaseUIElement::setPosition(x,y,w,h);
	int minX = BaseUIElement::xpos;
	int minY = BaseUIElement::ypos;
	
	double axesWidth = BaseUIElement::width/4.0;
	double axesHeight = (BaseUIElement::height - titleHeight)/2.0;
	
	int axX, axY;
	
	std::list<GenericAxes>::iterator i;
	int idx = 0, idx2 = 0;
	
	for (i=axesList.begin(); i!=axesList.end(); ++i){
		if (idx<=WAVE4){
			axX = minX  + axesWidth/2 * ( idx % 2 );
			axY = minY +  axesHeight * ( idx / 2);
			i->setPosition(axX, axY, axesWidth/2, axesHeight);
		}
		else{
			idx2 = idx - PROJ1x2;
			axX = minX + axesWidth *  (idx2 % 3 + 1); // add 1 to offset for the waveform plots
			axY = minY + axesHeight * (idx2 / 3); // no need to offset for the y direction
			i->setPosition(axX, axY, axesWidth, axesHeight);
		}
		idx++;
	}
	
	titleBox.setPosition(x, y+h-titleHeight-3, w, titleHeight+3);
}

int TetrodePlot::getNumberOfAxes(){
	return axesList.size();
}

// TetrodeSource* TetrodePlot::getDataSource(){
// 	return &tetSource;
// }

void TetrodePlot::clearOnNextDraw(bool b){
	BaseUIElement::clearNextDraw = b;
}

void TetrodePlot::setTitleEnabled(bool e){

    // if the new setting does not equal the old than clear on the next draw
    clearNextDraw = !(e!=enableTitle);

    enableTitle = e;
    if (e)
        titleHeight = 15;
    else
        titleHeight = 0;
    
    setPosition(BaseUIElement::xpos, BaseUIElement::ypos, 
                BaseUIElement::width, BaseUIElement::height);
}


void TetrodePlot::mouseDown(int x, int y){

    selectedAxesN = -1;
    std::list<GenericAxes>::iterator i;
    int idx=-1;
    bool hit = false;

    selectedAxes = NULL;
    for (i=axesList.begin(); i!=axesList.end(); ++i)
    {
        if (i->hitTest(x,y))
        {
            selectedAxes = addressof(*i);
            selectedAxesN = i->getType();
            hit = true;
//            break;
        }
        idx++;
    }
    if (!hit)
        selectedAxes = NULL;
    if (selectedAxes != NULL)
        std::cout<<"TetrodePlot::mouseDown() hit:"<<selectedAxes<<" AxesType:"<<selectedAxes->getType()<<std::endl;
    else
        std::cout<<"TetrodePlot::mouseDown() NO HIT!"<<std::endl;
    
}
void TetrodePlot::mouseDragX(int dx, bool shift, bool ctrl){

    if (selectedAxes == NULL || dx==0)
        return;
//    zoomAxes(selectedAxes->getType(), true, dx>0);
    if (shift)
        zoomAxes(selectedAxesN, true, dx);
    if (ctrl)
        panAxes(selectedAxesN, true, dx);

}
void TetrodePlot::mouseDragY(int dy, bool shift, bool ctrl){
    if (selectedAxes == NULL || dy==0)
        return;
    if(shift)
        zoomAxes(selectedAxesN, false, dy);
    if(ctrl)
        panAxes(selectedAxesN, false, dy);
}

void TetrodePlot::zoomAxes(int n, bool xdim, int zoom){
//    std::cout<<"TetrodePlot::zoomAxes() n:"<< n<<" xdim"<< xdim<<" in:"<<zoomin<<std::endl;
    // If trying to zoom an invalid axes type
    if (n<WAVE1 || n>PROJ3x4)
        return;
    if (n<=WAVE4)
        zoomWaveform(n, xdim, zoom);
    else
        zoomProjection(n, xdim, zoom);
}

void TetrodePlot::zoomWaveform(int n, bool xdim, int zoom){

    // waveform plots don't have a xlimits
    if (xdim)
        return;
//    std::cout<<"Zooming Waveform:"<<n<<" zoomin:"<<zoomin<<" ";
    double min, max;
    
    if(xdim)
        return;

    min = limits[n][0];
    max = limits[n][1];
    
    double mean = (max + min)/2.0f;
    double delta = max - mean;
    delta = delta / pow(.99, -1*zoom);

    min = mean - delta;
    max = mean + delta;

    limits[n][0] = min;
    limits[n][1] = max;
    
    limitsChanged = true;
}
void TetrodePlot::zoomProjection(int n, bool xdim, int zoom){
    int d1, d2;
    n2ProjIdx(n, &d1, &d2);
    
    if(xdim)
        n = d1;
    else
        n = d2;

    double min, max;
    
    min = limits[n][0];
    max = limits[n][1];
    
    double mean = (max + min)/2.0f;
    double delta = max - mean;
    delta = delta / pow(.99, -1*zoom);
    
    min = mean - delta;
    max = mean + delta;
    
    
    
    limits[n][0] = min;
    limits[n][1] = max;
    
    limitsChanged = true;

}
void TetrodePlot::panAxes(int n, bool xdim, int panval){
    //    std::cout<<"TetrodePlot::zoomAxes() n:"<< n<<" xdim"<< xdim<<" in:"<<zoomin<<std::endl;
    // If trying to zoom an invalid axes type
    if (n<WAVE1 || n>PROJ3x4)
        return;
    if (n<=WAVE4)
        panWaveform(n, xdim, panval);
    else
        panProjection(n, xdim, panval);
}

void TetrodePlot::panWaveform(int n, bool xdim, int pan){
    
    // waveform plots don't have a xlimits
    if (xdim)
        return;
    //    std::cout<<"Panning Waveform:"<<n<<" pan:"<<pan<<" "<<std::endl;
    double min, max;
    
    if(xdim)
        return;
    
    min = limits[n][0];
    max = limits[n][1];
    
    double dy = max-min;
    
    // Need to grab something if pan event is driven by the keyboard, which means that all the plots are getting panned so this should be okay
    if (selectedAxes == NULL)
        selectedAxes = &axesList.front();
    
    double yPixels = (BaseUIElement::height - titleHeight)/2.0;
    
    double pixelWidth = -1 * dy/yPixels;
    
    double delta = pan * pixelWidth;
    min = min + delta;
    max = max + delta;
    
    limits[n][0] = min;
    limits[n][1] = max;
    
    limitsChanged = true;
}
void TetrodePlot::panProjection(int n, bool xdim, int pan){
    int d1, d2;
    n2ProjIdx(n, &d1, &d2);
    
    if(xdim)
        n = d1;
    else
        n = d2;
    
    double min, max;
    
    min = limits[n][0];
    max = limits[n][1];
    
    double dy = max-min;
    
    double yPixels = (BaseUIElement::height - titleHeight)/2.0;
    
    double pixelWidth = -1 * dy/yPixels;
    
    double delta = pan * pixelWidth;
    min = min + delta;
    max = max + delta;
    
    limits[n][0] = min;
    limits[n][1] = max;
    
    
    limits[n][0] = min;
    limits[n][1] = max;
    
    limitsChanged = true;
    
}

void TetrodePlot::initLimits(){
    for (int i=0; i<4; i++)
    {
        limits[i][0] = -1*pow(2,11);
        limits[i][1] = pow(2,14);
    }

}

bool TetrodePlot::processKeyEvent(SimpleKeyEvent k){
    std::cout<<"Key:"<<(char)k.key<<std::endl;
    switch(k.key)
    {
        case '=':
            for (int i=0; i<=WAVE4; i++)
                zoomWaveform(i, false, 3);
            break;        
        case '+':
            for (int i=0; i<=WAVE4; i++)
                panWaveform(i, false, 3);
            break;
        case '-':
            for (int i=0; i<=WAVE4; i++)
                zoomWaveform(i, false, -3);
            break;    
        case '_':
            for (int i=0; i<=WAVE4; i++)
                panWaveform(i, false, -3);
            break;
        case 'C':
            clearOnNextDraw(true);
            break;
    }
}


