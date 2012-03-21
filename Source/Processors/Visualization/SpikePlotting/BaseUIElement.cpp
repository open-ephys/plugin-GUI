#include "BaseUIElement.h"

BaseUIElement::BaseUIElement():
	xpos(0),  ypos(0), width(100), height(100), enabled(true), padding(0)
{	
	clearNextDraw = false;
}

BaseUIElement::BaseUIElement(int x, int y, double w, double h):
	enabled(true), padding(0)
{
	xpos = x+padding;
	ypos = y+padding;
	width = w-padding*2;
	height = h-padding*2;
	clearNextDraw = false;
}
BaseUIElement::BaseUIElement(int x, int y, double w, double h, int p):
	enabled(true), padding(0)
{
	xpos = x+padding;
	ypos = y+padding;	
	width = w-padding*2;
	height = h-padding*2;
	clearNextDraw = false;
}

void BaseUIElement::redraw(){
//	std::cout<<"BaseUIElement::redraw(), Position:"<<xpos<<","<<ypos<<" : "<<width<<","<<height<<std::endl;
	setGlViewport();
	
	if (clearNextDraw || !clearNextDraw){
		clearNextDraw = false;
		glColor3f(0.0, 0.0, 0.0);
		glRecti(-1,-1,1,1);
		
		
	}
}
void BaseUIElement::drawElementEdges(){
	// std::cout<<"BaseUIElement::drawBaseUIElementEdges(), Position:"<<xpos<<","<<ypos<<" : "<<width<<","<<height<<std::endl;
	glColor3f(1.0, 1.0, 1.0);
	setGlViewport();
	glLineWidth(2);
	drawViewportEdge();
}
void BaseUIElement::setEnabled(bool e){
	enabled = e;	
}
bool BaseUIElement::getEnabled(){
	return enabled;
}
void BaseUIElement::setGlViewport(){
	glViewport(xpos, ypos, width, height);
	glLoadIdentity();
	// std::cout<<xpos<<"x"<<ypos<<"-"<<width<<"x"<<height<<"\t"<<BaseUIElementName<<std::endl;
}
void BaseUIElement::setPosition(int x, int y, double w, double h){
	xpos = x+padding;
	ypos = y+padding;
	width = w - padding*2;
	height = h - padding*2;
}
void BaseUIElement::clearOnNextDraw(bool c){
	clearNextDraw = c;
}

bool BaseUIElement::hitTest(int x, int y){
    return (x>xpos && x<xpos+width) && (y>ypos && y<ypos+height);
}

double BaseUIElement::getHeight(){
    return height;
}
double BaseUIElement::getWidth(){
    return width;
}
int BaseUIElement::getX(){
    return xpos;
}
int BaseUIElement::getY(){
    return ypos;
}
