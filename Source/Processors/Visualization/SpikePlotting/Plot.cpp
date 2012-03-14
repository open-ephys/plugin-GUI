#include "Plot.h"

Plot::Plot(){
	BaseUIElement();
	plotTitle = (char*) "TetrodePlot";
}
Plot::Plot(int x, int y, int w, int h, char *n):
BaseUIElement(x,y,x,h){
	plotTitle = n;
	
}

// Each plot needs to update its children axes when its redraw gets called. it also needs to call the parent plot
// when children axes get added it should place them in the correct location because it KNOWS where WAVE1 and PROJ1x3
// should go by default. This isn't as general as it should be but its a good push in the right direction

