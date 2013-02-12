#include "WaveAxes.h"

WaveAxes::WaveAxes()
	: GenericAxes(),
	  drawWaveformLine(true),
	  drawWaveformPoints(false),
	  overlay(false),
	  drawGrid(true),
	  convertLabelUnits(true)
{
	GenericAxes::gotFirstSpike = false;

	ylims[0] = 0;
	ylims[1] = 1;
	setWaveformColor(1.0,1.0,0.6);
	setThresholdColor(1.0, 0.1, 0.1);
	setGridColor(0.4, 0.2, 0.2);
}

WaveAxes::WaveAxes(int x, int y, double w, double h, int t)
	: GenericAxes(x,y,w,h,t),
	  drawWaveformLine(true),
	  drawWaveformPoints(false),
	  overlay(false),
	  drawGrid(true),
	  convertLabelUnits(true)
{
	GenericAxes::gotFirstSpike = false;

	setWaveformColor(1.0,1.0,0.6);
	setThresholdColor(1.0, 0.1, 0.1);
	setGridColor(0.2, 0.2, 0.2);

}

void WaveAxes::updateSpikeData(SpikeObject newSpike){
	//std::cout<<"WaveAxes::updateSpikeData()"<<std::endl;
	GenericAxes::updateSpikeData(newSpike);

}

void WaveAxes::redraw(){
	
	BaseUIElement::redraw();
	
	plot();
	
	BaseUIElement::drawElementEdges();
}


void WaveAxes::plot(){

	int chan = 0;
	
	// If no spikes have been received then don't plot anything
	if (!gotFirstSpike)
	{
		//std::cout<<"\tWaiting for the first spike"<<std::endl;
		return;
	}

	//setYLims(-20000,20000);
	
	// Set the plotting range for the current axes the xlims member is ignored as the xdims are 0->number of samples per waveform minus one
    // so the line goes all the way to the edges ydims are specified by the ylims vector		
	setViewportRange(0, ylims[0], s.nSamples-1, ylims[1]);
	//setViewportRange(0, -2000, s.nSamples-1, 20000);

	//std::cout << "ylims set to " << ylims[0] << " " << ylims[1] << std::endl;

	// draw the grid lines for the waveforms?
	 if (drawGrid)
	 	drawWaveformGrid(s.threshold[chan], s.gain[chan]);
	

	//std::cout << "ylims set to " << ylims[0] << " " << ylims[1] << std::endl;

	//compute the spatial width for each waveform sample	
	float dx = 1;
	float x = 0;

	// type corresponds to channel so we need to calculate the starting
	// sample based upon which channel is getting plotted
	// type values are defined in PlotUtils.h 
	int	sampIdx = s.nSamples * type; //  
	//std::cout<<"Starting with idx:"<<sampIdx<<std::endl;


	//Draw the individual waveform points connected with a line
	glColor3fv(waveColor);
	glLineWidth(2);
	glBegin( GL_LINE_STRIP );
	
	int dSamples = 1;
	for (int i = 0; i < s.nSamples; i++)
	{
		//std::cout<<"\t"<<s.data[sampIdx];
		glVertex2f(x, s.data[sampIdx] - 32768);
		sampIdx += dSamples;
		x +=dx; 
	}
	
	glEnd();
	

	//Draw the threshold line and label
	glColor3fv(thresholdColor);
	glLineWidth(1); 
	glLineStipple(4, 0xAAAA); // make a dashed line
	glEnable(GL_LINE_STIPPLE);

	glBegin( GL_LINE_STRIP );
		glVertex2f(0, s.threshold[chan]);
		glVertex2f(s.nSamples, s.threshold[chan]);
	glEnd();		

	glDisable(GL_LINE_STIPPLE);

	char cstr[100] = {0};
	
	makeLabel(s.threshold[chan], s.gain[chan], convertLabelUnits, cstr);
	String str = String(cstr);
	
	float yOffset = (ylims[1] - ylims[0])/BaseUIElement::height * 2;
	drawString(1 ,s.threshold[chan] + yOffset, 15, str, font);
}

void WaveAxes::drawWaveformGrid(int thold, int gain){

	//setYLims(-20000,20000);

	double voltRange = ylims[1] - ylims[0];
	double pixelRange = BaseUIElement::height;
	//This is a totally arbitrary value that seemed to lok the best for me
	int minPixelsPerTick = 25;
	int MAX_N_TICKS = 10;

	int nTicks = pixelRange / minPixelsPerTick;
	while(nTicks > MAX_N_TICKS){
		minPixelsPerTick += 5;
		nTicks = pixelRange / minPixelsPerTick;
	}

	int voltPerTick = (voltRange / nTicks);

	double meanRange = voltRange/2;
	glColor3fv(gridColor);

	glLineWidth(1);
	char cstr[200] = {0}; 
	String str;
	
	double tickVoltage = thold;

	// If the limits are bad we don't want to hang the program trying to draw too many ticks
	// so count the number of ticks drawn and kill the routine after 100 draws
	int tickCount=0;
	while (tickVoltage < ylims[1] - voltPerTick*1.5) // Draw the ticks above the thold line
	{
		tickVoltage = roundUp(tickVoltage + voltPerTick, 100);
		
		glBegin(GL_LINE_STRIP);
		glVertex2i(0, tickVoltage);
		glVertex2i(s.nSamples, tickVoltage);
		glEnd();
		
		makeLabel(tickVoltage, gain, convertLabelUnits, cstr);
		str = String(cstr);
		drawString(1, tickVoltage+voltPerTick/10, 15, str, font);
		
		if (tickCount++>100)
			return;
	}
	
	tickVoltage = thold;
	tickCount = 0;

	while (tickVoltage > ylims[0] + voltPerTick) // draw the ticks below the thold line
	{
		tickVoltage = roundUp(tickVoltage - voltPerTick, 100);

		glBegin(GL_LINE_STRIP);
		glVertex2i(0, tickVoltage);
		glVertex2i(s.nSamples, tickVoltage);
		glEnd();
			
		makeLabel(tickVoltage, gain, convertLabelUnits, cstr);
		str = String(cstr);
		drawString(1, tickVoltage+voltPerTick/10, 15, str, font);

		if (tickCount++>100)
			return;
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
