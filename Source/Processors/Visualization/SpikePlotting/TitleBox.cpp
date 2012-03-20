#include "TitleBox.h"
#include "PlotUtils.h"

TitleBox::TitleBox():
BaseUIElement(0,0,15,100,0)
{
	setColor(.15, .15, .50);
	setSelectedColor(.15, .50, .15);
	selected = false;
	BaseUIElement::elementName = (char*) "TitleBox - Un initialized";
	title = (char *) "Tetrode:00 abcdefghijkl";
}

TitleBox::TitleBox(int x, int y,int w,int h, const char *n):
BaseUIElement(x,y,w,h,0)
{
	setColor(.15, .15, .50);
	setSelectedColor(.15, .50, .15);
	selected = false;
	BaseUIElement::elementName = (char*) "TitleBox";
	title = n;//(char *) "Tetrode:00 No Port or Label";
}

void TitleBox::redraw(){
	BaseUIElement::redraw();
	
	if(selected)
			glColor3fv(selectedColor);
		else
			glColor3fv(titleColor);

	// draw the colored background for the plot
	glRecti(-1,-1,1,1);

	// Reset color to white so we can draw the title text in white
	glColor3f(1.0, 1.0, 1.0);
	void * font;// = GLUT_BITMAP_9_BY_15;
	
	// We want the title string to be centered in the box regardless of how 
	// many chars are in the title. To do this we must compute the proper X offset
	// for the title string. Using a 9x15 font each char is 9 pixels wide. 
	// convert from chars to pixels and then normalize using the size of the window.
	// We don't have to devide by two to center the string because the dynamic
	// range of the viewport goes from -1 to 1 and the following equatition treats it as 1
	// we would then have to multiply and divide by 2 which is redundant so we dont
	float xOffset = -1*( (float) strlen(title) * 9.0) / ( BaseUIElement::width);
	if (xOffset<-.95)
		xOffset = -.95;

	drawString(xOffset, -.6, font, title);
	
	BaseUIElement::drawElementEdges();
}


void TitleBox::setTitle(char *n){
	title = n;
}
void TitleBox::setSelected(bool sel){
	selected = sel;
}

void TitleBox::setColor(GLfloat r, GLfloat g, GLfloat b){
	titleColor[0] = r;
	titleColor[1] = g;
	titleColor[2] = b;
}
void TitleBox::setSelectedColor(GLfloat r, GLfloat g, GLfloat b){
	selectedColor[0] = r;
	selectedColor[1] = g;
	selectedColor[2] = b;
}

void TitleBox::setPosition(int x, int y, double w, double h){
	BaseUIElement::setPosition(x,y,w,h);
}