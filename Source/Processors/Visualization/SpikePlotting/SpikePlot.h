#ifndef SPIKE_PLOT_H_
#define SPIKE_PLOT_H_

#if defined(__linux__)
	#include <GL/glut.h>
#else
	#include <GLUT/glut.h>
#endif
#include <list>
#include <math.h>

#include "WaveAxes.h"
#include "ProjectionAxes.h"
#include "BaseUIElement.h"
#include "PlotUtils.h"
#include "SimpleKeyEvent.h"

#define TETRODE_PLOT 1004
#define STEREO_PLOT  1002
#define SINGLE_PLOT  1001
//#define HIST_PLOT    1000 // perhaps we'll use hist plots at a later date but not for now


/**

  Class for drawing the waveforms and projections of incoming spikes.

*/

class SpikePlot : public BaseUIElement{
	
	bool enabled;	
    
    bool limitsChanged;

    static const int MAX_N_CHAN = 4;
    static const int MAX_N_PROJ = 6;

    int nChannels;
    int plotType;
    int nWaveAx;
    int nProjAx;
    
    double limits[MAX_N_CHAN][2];

    WaveAxes wAxes[MAX_N_CHAN];
    ProjectionAxes pAxes[MAX_N_PROJ];
    
    
    
    // void zoomAxes(int n, bool xdim, int zoomval);
    // void zoomProjection (int n, bool xdim, int zoomval);
    // void zoomWaveform (int n, bool xdim, int zoomval);
    
    // void panAxes(int n, bool xdim, int panval);
    // void panProjection (int n, bool xdim, int panval);
    // void panWaveform(int n, bool xdim, int panval);
    
    void initLimits();
    void setLimitsOnAxes();
    void updateAxesPositions();

	
public:
	SpikePlot();
	SpikePlot(int x, int y,int w,int h, int pType);
	virtual ~SpikePlot();

	void initAxes();
	void redraw();

	void setEnabled(bool enabled);
	bool getEnabled();
	void setPosition(int,int,double,double);
    
	void getBestDimensions(int*, int*);
    
    void mouseDown(int x, int y);
    
    void mouseDragX(int dx, bool shift, bool ctr);
    void mouseDragY(int dy, bool shift, bool ctr);

    bool processKeyEvent(SimpleKeyEvent k);

    void processSpikeObject(SpikeObject s);

    void clear();
    void zoom(int, bool);
    void pan(int, bool);
};



#endif
