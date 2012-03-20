#ifndef PLOT_H_
#define PLOT_H_

#if defined(__linux__)
	#include <GL/glut.h>
#else
	#include <GLUT/glut.h>
#endif
#include <list>
#include "GenericAxes.h"

class Plot: public BaseUIElement
{
	
	char *plotTitle;
	std::list<GenericAxes> axesList;
	
public:
	Plot();
	Plot(int x, int y,int w,int h, char *n);
	void redraw();
	void setTitle(char *n);
	void setEnabled(bool enabled);
	void addAxes(int axesType);
};


#endif
