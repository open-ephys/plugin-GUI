#ifndef BASEUIELEMENT_H_
#define BASEUIELEMENT_H_

#include "PlotUtils.h"


class BaseUIElement{

protected:	
	int xpos, ypos;
	double height, width;
	bool enabled;
	double padding;
	
	void setGlViewport();
		
public:
	BaseUIElement();
	BaseUIElement(int x, int y, double w, double h);
	BaseUIElement(int x, int y, double w, double h, int p);
	void redraw();
	void drawElementEdges();
	void setEnabled(bool e);
	bool getEnabled();
	void setPosition(int x, int y, double w, double h);
    double getHeight();
    double getWidth();
    int getX();
    int getY();
	
	void clearOnNextDraw(bool);
	bool clearNextDraw;
    bool hitTest(int x, int y);
	
};



#endif // BaseUIElement_H_
