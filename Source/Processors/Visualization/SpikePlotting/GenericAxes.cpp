#include "GenericAxes.h"

GenericAxes::GenericAxes():
					BaseUIElement(),
					type(0),
					gotFirstSpike(false)
{	
	ylims[0] = 0;
	ylims[1] = 1;
}

GenericAxes::GenericAxes(int x, int y, double w, double h, int t):
					BaseUIElement(x,y,w,h),
					gotFirstSpike(false)
{
	type = t;
}
void GenericAxes::updateSpikeData(SpikeObject newSpike){
	if (!gotFirstSpike){
		gotFirstSpike = true;	
	}

	s = newSpike;
}


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

void GenericAxes::setPosition(int x, int y, double w, double h){
	BaseUIElement::setPosition(x,y,w,h);	
}
