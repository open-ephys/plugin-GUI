#ifndef TETRODE_PLOT_H
#define TETRODE_PLOT_H

#if defined(__linux__)
	#include <GL/glut.h>
#else
	#include <GLUT/glut.h>
#endif
#include <list>
#include <math.h>

#include "GenericAxes.h"
#include "WaveAxes.h"
#include "ProjectionAxes.h"
#include "TitleBox.h"
#include "BaseUIElement.h"
#include "TetrodeSource.h"
#include "PlotUtils.h"
#include "SimpleKeyEvent.h"


class TetrodePlot : public BaseUIElement{
	
	char *plotTitle;
	std::list<GenericAxes> axesList;
	TitleBox titleBox;

	bool enabled;
    bool enableTitle;
	void drawTitle();
	
	double titleHeight;
	void *ptr;
    
    bool limitsChanged;
    double limits[4][2];
    
    GenericAxes* selectedAxes;
    int selectedAxesN;
    
    void zoomAxes(int n, bool xdim, int zoomval);
    void zoomProjection (int n, bool xdim, int zoomval);
    void zoomWaveform (int n, bool xdim, int zoomval);
    
    void panAxes(int n, bool xdim, int panval);
    void panProjection (int n, bool xdim, int panval);
    void panWaveform(int n, bool xdim, int panval);
    
    void initLimits();
	
public:
	TetrodePlot();
	TetrodePlot(int x, int y,int w,int h, char *n);

	void initAxes();
	void redraw();
	void setTitle(char *n);
	void setEnabled(bool enabled);
	bool getEnabled();
	void setPosition(int,int,double,double);

	int getNumberOfAxes();
	void clearOnNextDraw(bool c);
    void setTitleEnabled(bool e);
    
    void mouseDown(int x, int y);
    
    void mouseDragX(int dx, bool shift, bool ctr);
    void mouseDragY(int dy, bool shift, bool ctr);

    bool processKeyEvent(SimpleKeyEvent k);
};



#endif
