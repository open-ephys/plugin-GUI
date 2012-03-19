#ifndef PROJECTION_AXES_H_
#define PROJECTION_AXES_H_

#if defined(__linux__)
	#include <GL/glut.h>
#else
	#include <GLUT/glut.h>
#endif
#include <stdlib.h>
#include "BaseUIElement.h"
#include "../SpikeObject.h"
#include "PlotUtils.h"
#include "GenericAxes.h"

 	
class ProjectionAxes: public GenericAxes{
	
	GLfloat pointColor[3];
	GLfloat gridColor[3];
		
	
  	void drawProjectionGrid(int thold, int gain);
  	void calcWaveformPeakIdx(int, int, int*, int*);

protected:
	void plot();


public:
	ProjectionAxes();
	ProjectionAxes(int x, int y, double w, double h, int t);
    
   	void updateSpikeData(SpikeObject s);

	void setPointColor(GLfloat r, GLfloat g, GLfloat b);
	void setGridColor(GLfloat, GLfloat, GLfloat);
	 
	void redraw();
	
	
	
	bool overlay;
	bool drawGrid;
	bool convertLabelUnits;
	
};



#endif  // PROJECTION_AXES_H_




// #endif
