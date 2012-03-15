#include "GenericAxes.h"

GenericAxes::GenericAxes():
					BaseUIElement(),
					type(0),
					gotFirstSpike(false),
					resizedFlag(false)
{	
	ylims[0] = 0;
	ylims[1] = 1;
		
	//BaseUIElement::elementName = (char*) "GenericAxes";
}

GenericAxes::GenericAxes(int x, int y, double w, double h, int t):
					BaseUIElement(x,y,w,h),
					gotFirstSpike(false),
					resizedFlag(false)
{
	std::cout<<"Generic Axes!!!!!"<<std::endl;
	std::cout<<"gotFirstSpike:"<<gotFirstSpike<<std::endl;
	// if (t<WAVE1 || t>PROJ3x4)
		//("Invalid Axes type specified");
	type = t;
	
//	BaseUIElement::elementName = (char*) "GenericAxes";

}
void GenericAxes::updateSpikeData(SpikeObject newSpike){
	
	std::cout<<"GenericAxes::updateSpikeData() ";
	if (!gotFirstSpike){
		gotFirstSpike = true;	
	}

	s = newSpike;
	std::cout<<"got spike with nSamples:"<<s.nSamples<<std::endl;
}

// void GenericAxes::redraw(){
// 	BaseUIElement::redraw();
// 	if (BaseUIElement::enabled)
// 		plotData();

// 	BaseUIElement::drawElementEdges();
// }
// void GenericAxes::plotData(){

// 	if (!gotFirstSpike)
// 	{
// 		std::cout<<"\tWaiting for the first spike"<<std::endl;
// 		return;
// 	}
// 	switch(type){
// 		case WAVE1:
// 		case WAVE2:
// 		case WAVE3:
// 		case WAVE4:
// 		plotWaveform(type);
// 		break;
		
// 		case PROJ1x2:
// 		case PROJ1x3:
// 		case PROJ1x4:
// 		case PROJ2x3:
// 		case PROJ2x4:
// 		case PROJ3x4:
// 		plotProjection(type);
// 		break;
// 		default:
// 			std::cout<<"GenericAxes::plotData(), Invalid type specified, cannot plot"<<std::endl;
// 			exit(1);
// 	}
// }
void GenericAxes::setYLims(double ymin, double ymax){
	ylims[0] = ymin;
	ylims[1] = ymax;
}
void GenericAxes::getYLims(double *min, double *max){
	*min = ylims[0];
	*max = ylims[1];
}
void GenericAxes::setXLims(double xmin, double xmax){
	xlims[0] = xmin;
	xlims[1] = xmax;
}
void GenericAxes::getXLims(double *min, double *max){
	*min = xlims[0];
	*max = xlims[1];
}
void GenericAxes::setType(int t){
	 if (t<WAVE1 || t>PROJ3x4){
		std::cout<<"Invalid Axes type specified";
		return;
	}
	
	type = t;
}
int GenericAxes::getType(){
    return type;
}


// void GenericAxes::setWaveformColor(GLfloat r, GLfloat g, GLfloat b){
// 	waveColor[0] = r;
// 	waveColor[1] = g;
// 	waveColor[2] = b;
// }
// void GenericAxes::setThresholdColor(GLfloat r, GLfloat g, GLfloat b){
// 	thresholdColor[0] = r;
// 	thresholdColor[1] = g;
// 	thresholdColor[2] = b;
// }
// void GenericAxes::setPointColor(GLfloat r, GLfloat g, GLfloat b){
// 	pointColor[0] = r;
// 	pointColor[1] = g;
// 	pointColor[2] = b;
// }
// void GenericAxes::setGridColor(GLfloat r, GLfloat g, GLfloat b){
// 	gridColor[0] = r;
// 	gridColor[1] = g;
// 	gridColor[2] = b;
// }
void GenericAxes::setPosition(int x, int y, double w, double h){
	BaseUIElement::setPosition(x,y,w,h);	
	resizedFlag = true;
}
void GenericAxes::clearOnNextDraw(bool b){
	BaseUIElement::clearOnNextDraw(b);
}