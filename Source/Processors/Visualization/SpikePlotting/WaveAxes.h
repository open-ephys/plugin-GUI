#ifndef WAVE_AXES_H
#define WAVE_AXES_H

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

/**

  Class for drawing spike waveforms.

*/
 	
class WaveAxes: public GenericAxes{
	
	GLfloat waveColor[3];
	GLfloat thresholdColor[3];
	GLfloat gridColor[3];
		
	
  	void drawWaveformGrid(int thold, int gain);

protected:
	void plot();


public:
	WaveAxes();
	WaveAxes(int x, int y, double w, double h, int t);
    virtual ~WaveAxes() {}
    
   	void updateSpikeData(SpikeObject s);

	void setWaveformColor(GLfloat r, GLfloat g, GLfloat b);
	void setThresholdColor(GLfloat r, GLfloat g, GLfloat b);
	void setGridColor(GLfloat, GLfloat, GLfloat);
	 
	void redraw();
	
	bool drawWaveformLine;
	bool drawWaveformPoints;
	bool overlay;
	bool drawGrid;
	bool convertLabelUnits;
	
};



#endif
