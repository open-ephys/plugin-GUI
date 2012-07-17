#ifndef BASEUIELEMENT_H_
#define BASEUIELEMENT_H_

#include "PlotUtils.h"
#include "../SpikeObject.h"

class SpikeObject;

/**
	
	Base class for drawing spike plots in OpenGL.

	@see SpikeDisplayCanvas

*/

class BaseUIElement{

protected:	
	int xpos, ypos, yOffset;
	double height, width;
	bool enabled;
	double padding;
	
	void setGlViewport();
		
public:
	BaseUIElement();
	BaseUIElement(int x, int y, double w, double h);
	BaseUIElement(int x, int y, double w, double h, int p);
    
	virtual void redraw();
	void drawElementEdges();
	virtual void setEnabled(bool e);
	virtual bool getEnabled();
	virtual void setPosition(int x, int y, double w, double h);
	virtual void setPosition(int, int) {}
	virtual void getPosition(int*, int*, double*, double*);
    double getHeight();
    double getWidth();
    int getX();
    int getY();
	
	// needed for spike display object subclasses:
	virtual void processSpikeObject(SpikeObject s) {}
	virtual void pan(int, bool) {}
	virtual void zoom(int, bool) {}
	virtual void clear() {}

	// void clearOnNextDraw(bool);
	// bool clearNextDraw;
    bool hitTest(int x, int y);
	
};



#endif // BaseUIElement_H_
