#include "WaveAxes.h"

WaveAxes::WaveAxes():
					GenericAxes(),
					drawWaveformLine(true),
					drawWaveformPoints(false),
					drawGrid(true),
					overlay(false),
					convertLabelUnits(true)
{	
	GenericAxes::gotFirstSpike = false;
	GenericAxes::resizedFlag = false;

	ylims[0] = 0;
	ylims[1] = 1;
	setWaveformColor(1.0,1.0,0.6);
	setThresholdColor(1.0, 0.1, 0.1);
	setGridColor(0.4, 0.2, 0.2);
	
	BaseUIElement::elementName = (char*) "WaveAxes";
}

WaveAxes::WaveAxes(int x, int y, double w, double h, int t):
					GenericAxes(x,y,w,h,t),
					drawWaveformLine(true),
					drawWaveformPoints(false),
					drawGrid(true),
					overlay(false),
					convertLabelUnits(true)
{	
	GenericAxes::gotFirstSpike = false;
	GenericAxes::resizedFlag = false;

	setWaveformColor(1.0,1.0,0.6);
	setThresholdColor(1.0, 0.1, 0.1);
	setGridColor(0.2, 0.2, 0.2);
	BaseUIElement::elementName = (char*) "WaveAxes";

}
void WaveAxes::updateSpikeData(SpikeObject newSpike){
	GenericAxes::updateSpikeData(newSpike);
}
void WaveAxes::redraw(){
	BaseUIElement::redraw();
	if (BaseUIElement::enabled)
		plot();

	BaseUIElement::drawElementEdges();
}


void WaveAxes::plot(){

	int chan = 0;
	if (!gotFirstSpike)
	{
		std::cout<<"\tWaiting for the first spike"<<std::endl;
		return;
	}
	
	if (s.nSamples>1024)
		return;	

	// Set the plotting range for the current axes 
	// the xlims member is ignored as the xdims are 0->number of samples per waveform minus one
    // so the line goes all the way to the edges ydims are specified by the ylims vector		
    
	setViewportRange(0, ylims[0], s.nSamples-1, ylims[1]);
	
	// Are new waves getting overlayed on top of old waves? If not clear the display
	if(!overlay){
		glColor3f(0.0,0.0,0.0);
		glRectd(0, ylims[0], s.nSamples, ylims[1]);
	}
	
	// Are we drawing the grid lines for the waveforms?
	if(drawGrid)
		drawWaveformGrid(s.threshold[chan], s.gain[chan]);
	
	//compute the spatial width for each wawveform sample	
	float dx = 1;
	float x = 0;
	int	sampIdx = 0; 

	//Draw the individual waveform points connected with a line
	// if drawWaveformPoints is set to false then force the drawing of the line, _SOMETHING_ must be drawn
	glColor3fv(waveColor);
	
	//if drawWaveformLine and drawWaveformPoints are both set
	if(drawWaveformLine){
		glLineWidth(1);
		glBegin( GL_LINE_STRIP );
		for (int i=0; i<s.nSamples; i++)
		{
			glVertex2f(x, s.data[sampIdx]);
			sampIdx +=4;
			x +=dx; 
		}
		glEnd();
	}
	

/*
	//if drawWaveformLine and drawWaveformPoints are both set false then draw the points
	//this ensures that something is always drawn
	if(drawWaveformPoints || !drawWaveformLine){
		x = 0;
		sampIdx = chan;
		glColor3fv(pointColor);
		glPointSize(1);
		glBegin( GL_POINTS );
		for (int i=0; i<s.nSamples; i++)
		{
			glVertex2f(x, s.data[sampIdx]);
			sampIdx +=4;
			x +=dx;
		}
		glEnd();
	} */

	// Draw the threshold line and label
	
	glColor3fv(thresholdColor);
	glLineWidth(1); 
	glLineStipple(4, 0xAAAA); // make a dashed line
	glEnable(GL_LINE_STIPPLE);

	glBegin( GL_LINE_STRIP );
		glVertex2f(0, s.threshold[chan]);
		glVertex2f(s.nSamples, s.threshold[chan]);
	glEnd();		

	glDisable(GL_LINE_STIPPLE);

	char str[500] = {0};
	
	/*if(convertLabelUnits)
		sprintf(str, "%duV", ad16ToUv(s.threshold[chan], s.gain[chan]));
	else
		sprintf(str, "%d", (int) s.threshold[chan]);*/
	makeLabel(s.threshold[chan], s.gain[chan], convertLabelUnits, str);
	
//	printf(str);
	
	float yOffset = (ylims[1] - ylims[0])/BaseUIElement::height * 2;
	//drawString(1 ,s.threshold[chan] + yOffset, GLUT_BITMAP_8_BY_13, str);
}

void WaveAxes::drawWaveformGrid(int thold, int gain){

	double voltRange = ylims[1] - ylims[0];
	double pixelRange = BaseUIElement::height;
	//This is a totally arbitrary value that i'll mess around with and set as a macro when I figure out a value I like
	int minPixelsPerTick = 25;
	int MAX_N_TICKS = 10;

	int nTicks = pixelRange / minPixelsPerTick;
	while(nTicks>MAX_N_TICKS){
		minPixelsPerTick += 5;
		nTicks = pixelRange / minPixelsPerTick;
	}
	int voltPerTick = (voltRange / nTicks);
	// Round to the nearest 200

	
	double meanRange = voltRange/2;
	glColor3fv(gridColor);

	glLineWidth(1);
	char str[200] = {0}; 
	
	double tickVoltage = thold;
	while(tickVoltage < ylims[1] - voltPerTick/2) // Draw the ticks above the thold line
	{
		tickVoltage = roundUp(tickVoltage + voltPerTick, 100);
		
		glBegin(GL_LINE_STRIP);
		glVertex2i(0, tickVoltage);
		glVertex2i(s.nSamples, tickVoltage);
		glEnd();
		
		makeLabel(tickVoltage, gain, convertLabelUnits, str);
		//drawString(1, tickVoltage+voltPerTick/10, GLUT_BITMAP_8_BY_13, str);
	}
	
	tickVoltage = thold;
	while(tickVoltage > ylims[0] + voltPerTick) // draw the ticks below the thold line
	{
		tickVoltage = roundUp(tickVoltage - voltPerTick, 100);

		glBegin(GL_LINE_STRIP);
		glVertex2i(0, tickVoltage);
		glVertex2i(s.nSamples, tickVoltage);
		glEnd();
			
		makeLabel(tickVoltage, gain, convertLabelUnits, str);
		//drawString(1, tickVoltage+voltPerTick/10, GLUT_BITMAP_8_BY_13, str);
	}
	
	
}

void WaveAxes::setWaveformColor(GLfloat r, GLfloat g, GLfloat b){
	waveColor[0] = r;
	waveColor[1] = g;
	waveColor[2] = b;
}
void WaveAxes::setThresholdColor(GLfloat r, GLfloat g, GLfloat b){
	thresholdColor[0] = r;
	thresholdColor[1] = g;
	thresholdColor[2] = b;
}
// void WaveAxes::setPointColor(GLfloat r, GLfloat g, GLfloat b){
// 	pointColor[0] = r;
// 	pointColor[1] = g;
// 	pointColor[2] = b;
// }
void WaveAxes::setGridColor(GLfloat r, GLfloat g, GLfloat b){
	gridColor[0] = r;
	gridColor[1] = g;
	gridColor[2] = b;
}