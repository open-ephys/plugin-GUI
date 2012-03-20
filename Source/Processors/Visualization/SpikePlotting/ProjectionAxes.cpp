#include "ProjectionAxes.h"

ProjectionAxes::ProjectionAxes():
					GenericAxes(),
					drawGrid(false),
					overlay(false),
					convertLabelUnits(true)
{	
	GenericAxes::gotFirstSpike = false;
	GenericAxes::resizedFlag = false;

	ylims[0] = 0;
	ylims[1] = 1;
	setPointColor(1.0,1.0,1.0);

	BaseUIElement::elementName = (char*) "ProjectionAxes";
}

ProjectionAxes::ProjectionAxes(int x, int y, double w, double h, int t):
					GenericAxes(x,y,w,h,t),
					drawGrid(true),
					overlay(false),
					convertLabelUnits(true)
{	
	GenericAxes::gotFirstSpike = false;
	GenericAxes::resizedFlag = false;

	setPointColor(1.0,1.0,1.0);
	
	BaseUIElement::elementName = (char*) "ProjectionAxes";

}

void ProjectionAxes::updateSpikeData(SpikeObject newSpike){
	//std::cout<<"ProjectionAxes::updateSpikeData()"<<std::endl;
	GenericAxes::updateSpikeData(newSpike);

}

void ProjectionAxes::redraw(){
	
	BaseUIElement::redraw();
	
	plot();
	
	BaseUIElement::drawElementEdges();
}


void ProjectionAxes::plot(){

	setViewportRange(xlims[0], ylims[0], xlims[1], ylims[1]);

	int d1, d2;
    n2ProjIdx(type, &d1, &d2);
	
	int idx1, idx2;
	calcWaveformPeakIdx(d1,d2,&idx1, &idx2);

	//if (drawGrid)
	//	drawProjectionGrid(s.gain[d1], s.gain[d2]);

	glColor3f(1.0, 1.0, 1.0);
	glPointSize(1);

	glBegin(GL_POINTS);
        glVertex2f(s.data[idx1], s.data[idx2]);
	glEnd();

	// std::cout<<"ProjectionAxes Limits:"<<ylims[0]<<" "<<ylims[1]<<std::endl;
	// std::cout<<"ProjectionAxes::plot()"<<s.data[idx1] << " " << s.data[idx2]<<std::endl;

}

 void ProjectionAxes::calcWaveformPeakIdx(int d1, int d2, int *idx1, int *idx2){

	int max1 = -1*pow(2,15);
	int max2 = max1;
	
	for (int i=0; i<s.nSamples ; i++){
		if (s.data[d1*s.nSamples + i] > max1)
		{	
			*idx1 = d1*s.nSamples+i;
			max1 = s.data[*idx1];
		}
		if (s.data[d2*s.nSamples+i] > max2)
		{	
			*idx2 = d2*s.nSamples+i;
			max2 = s.data[*idx2];
		}
	}
}

void ProjectionAxes::setPointColor(GLfloat r, GLfloat g, GLfloat b){
	pointColor[0] = r;
	pointColor[1] = g;
	pointColor[2] = b;
}
void ProjectionAxes::setGridColor(GLfloat r, GLfloat g, GLfloat b){
	gridColor[0] = r;
	gridColor[1] = g;
	gridColor[2] = b;
}
