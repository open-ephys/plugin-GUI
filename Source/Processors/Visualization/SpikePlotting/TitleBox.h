#ifndef TITLE_BOX_H_
#define TITLE_BOX_H_

#if defined(__linux__)
	#include <GL/glut.h>
#else
	#include <GLUT/glut.h>
#endif

#include "BaseUIElement.h"


class TitleBox : public BaseUIElement{
	
	const char *title;
	bool selected;
	void drawTitle();
	
	GLfloat titleColor[3];
	GLfloat selectedColor[3];
	
public:
	TitleBox();
	TitleBox(int x, int y,int w,int h, const char *n);

	void redraw();
	void setTitle(char *n);
	void setSelected(bool sel);
	void setColor(float, float, float);
	void setSelectedColor(float, float, float);
	void setPosition(int,int,double,double);
};


#endif
