#ifndef PROJECTION_AXES_H_
#define PROJECTION_AXES_H_

#define GL_GLEXT_PROTOTYPES

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
#include <stdint.h>

#define AMP_BUFF_MAX_SIZE 50000
 	
/**

  Class for drawing the peak projections of spike waveforms.

*/

class ProjectionAxes: public GenericAxes
{
	
	GLfloat pointColor[3];
	GLfloat gridColor[3];

	int ampBuffer[2][AMP_BUFF_MAX_SIZE];
	uint16_t buffIdx; // points to the most recent spike
	uint64_t totalSpikes;


	int ampDim1, ampDim2;
		
	
  	void drawProjectionGrid(int thold, int gain);
  	void calcWaveformPeakIdx(int, int, int*, int*);

  	void createTexture();
  	void createFBO();

  	void drawSpikesToTexture(bool allSpikes);
  	void drawTexturedQuad();
  	void plotOldSpikes(bool allSpikes);
  	void plotNewestSpike();

  	bool newSpike;

  	
  	GLuint fboId; // Frame Buffer Object 
  	GLuint textureId; // Texture 
  	GLuint rboId; // Render Buffer 

  	int texWidth;
  	int texHeight;

  	bool clearOnNextDraw;
 	bool isTextureValid;
    bool fboCreated;
  	void clearTexture();

  	void validateTexture();

  	bool allSpikesNextRender;

protected:
	void plot();


public:
	ProjectionAxes();
	ProjectionAxes(int x, int y, double w, double h, int t);
    virtual ~ProjectionAxes() {}
    
   	void setPosition(int, int, int, int);
   	void updateSpikeData(SpikeObject s);

	void setPointColor(GLfloat r, GLfloat g, GLfloat b);
	void setGridColor(GLfloat, GLfloat, GLfloat);
	
	void clear();
	void invalidateTexture();

	void redraw();	
	
	bool overlay;
	bool drawGrid;
	bool convertLabelUnits;

	
};

#endif  // PROJECTION_AXES_H_




// #endif
