// #include "ProjectionAxes.h"

// ProjectionAxes::ProjectionAxes():
// 					BaseUIElement(),
// 					type(1),
// 					drawWaveformLine(true),
// 					drawWaveformPoints(false),
// 					drawGrid(true),
// 					gotFirstSpike(false),
// 					overlay(false),
// 					convertLabelUnits(true),
// 					resizedFlag(false)
// {	
// 	ylims[0] = 0;
// 	ylims[1] = 1;
// 	setWaveformColor(1.0,1.0,0.6);
// 	setThresholdColor(1.0, 0.1, 0.1);
// 	setPointColor(1.0, 1.0, 1.0);
// 	setGridColor(0.4, 0.2, 0.2);
	
// 	BaseUIElement::elementName = (char*) "ProjectionAxes";
// }

// ProjectionAxes::ProjectionAxes(int x, int y, double w, double h, int t):
// 					BaseUIElement(x,y,w,h),
// 					drawWaveformLine(true),
// 					drawWaveformPoints(false),
// 					drawGrid(true),
// 					gotFirstSpike(false),
// 					overlay(false),
// 					convertLabelUnits(true),
// 					resizedFlag(false)
// {
// 	// if (t<WAVE1 || t>PROJ3x4)
// 		//("Invalid Axes type specified");
// 	type = t;
// 	setWaveformColor(1.0,1.0,0.6);
// 	setThresholdColor(1.0, 0.1, 0.1);
// 	setPointColor(1.0, 1.0, 1.0);
// 	setGridColor(0.2, 0.2, 0.2);
// 	BaseUIElement::elementName = (char*) "ProjectionAxes";

// }
// void ProjectionAxes::updateSpikeData(SpikeObject newSpike){
// 	if (!gotFirstSpike){
// 		gotFirstSpike = true;
// 		printf("ProjectionAxes::updateSpikeData() got first spike\n");
// 	}
// 	s = newSpike;
// }
// void ProjectionAxes::redraw(){
// 	BaseUIElement::redraw();
// 	if (BaseUIElement::enabled)
// 		plotData();

// 	BaseUIElement::drawElementEdges();
// }
// void ProjectionAxes::plotData(){

// 	if (!gotFirstSpike)
// 	{
// 		std::cout<<"\tWaiting for the first spike"<<std::endl;
// 		return;
// 	}
// 	switch(type){
// 		case WAVE1:
// 		case WAVE2:
// 		case WAVE3:
// 		case WAVE4:
// 		plotWaveform(type);
// 		break;
		
// 		case PROJ1x2:
// 		case PROJ1x3:
// 		case PROJ1x4:
// 		case PROJ2x3:
// 		case PROJ2x4:
// 		case PROJ3x4:
// 		plotProjection(type);
// 		break;
// 		default:
// 			std::cout<<"ProjectionAxes::plotData(), Invalid type specified, cannot plot"<<std::endl;
// 			exit(1);
// 	}
// }
// void ProjectionAxes::setYLims(double ymin, double ymax){
// 	ylims[0] = ymin;
// 	ylims[1] = ymax;
// }
// void ProjectionAxes::getYLims(double *min, double *max){
// 	*min = ylims[0];
// 	*max = ylims[1];
// }
// void ProjectionAxes::setXLims(double xmin, double xmax){
// 	xlims[0] = xmin;
// 	xlims[1] = xmax;
// }
// void ProjectionAxes::getXLims(double *min, double *max){
// 	*min = xlims[0];
// 	*max = xlims[1];
// }
// void ProjectionAxes::setType(int t){
// 	 if (t<WAVE1 || t>PROJ3x4){
// 		std::cout<<"Invalid Axes type specified";
// 		return;
// 	}
	
// 	type = t;
// }
// int ProjectionAxes::getType(){
//     return type;
// }


// void ProjectionAxes::plotWaveform(int chan){

// 	if (chan>WAVE4 || chan<WAVE1)
// 	{
// 		std::cout<<"ProjectionAxes::plotWaveform() invalid channel, must be between 0 and 4"<<std::endl;
// 		return;
// 	}
	
// 	if (s.n_samps_per_chan>1024)
// 		return;	

// 	// Set the plotting range for the current axes 
// 	// the xlims member is ignored as the 
//     // xdims are 0->number of samples per waveform minus one
//     // so the line goes all the way to the edges
// 	// ydims are specified by the ylims vector		
    
// 	setViewportRange(0, ylims[0], s.n_samps_per_chan-1, ylims[1]);
	
// 	if(!overlay){
// 		glColor3f(0.0,0.0,0.0);
// 		glRectd(0, ylims[0], s.n_samps_per_chan, ylims[1]);
// 	}
// 	if(drawGrid)
// 		drawWaveformGrid(s.thresh[chan], s.gains[chan]);
	
// 	//compute the spatial width for each wawveform sample	
// 	float dx = 1;
// 	float x = 0;
// 	int	sampIdx = chan; 

// 	//Draw the individual waveform points connected with a line
// 	// if drawWaveformPoints is set to false then force the drawing of the line, _SOMETHING_ must be drawn
// 	glColor3fv(waveColor);
	
// 	//if drawWaveformLine and drawWaveformPoints are both set
// 	if(drawWaveformLine){
// 		glLineWidth(1);
// 		glBegin( GL_LINE_STRIP );
// 		for (int i=0; i<s.n_samps_per_chan; i++)
// 		{
// 			glVertex2f(x, s.data[sampIdx]);
// 			sampIdx +=4;
// 			x +=dx; 
// 		}
// 		glEnd();
// 	}
	

// 	//if drawWaveformLine and drawWaveformPoints are both set false then draw the points
// 	//this ensures that something is always drawn
// 	if(drawWaveformPoints || !drawWaveformLine){
// 		x = 0;
// 		sampIdx = chan;
// 		glColor3fv(pointColor);
// 		glPointSize(1);
// 		glBegin( GL_POINTS );
// 		for (int i=0; i<s.n_samps_per_chan; i++)
// 		{
// 			glVertex2f(x, s.data[sampIdx]);
// 			sampIdx +=4;
// 			x +=dx;
// 		}
// 		glEnd();
// 	}
// 	// Draw the threshold line and label
	
// 	glColor3fv(thresholdColor);
// 	glLineWidth(1); 
// 	glLineStipple(4, 0xAAAA); // make a dashed line
// 	glEnable(GL_LINE_STIPPLE);

// 	glBegin( GL_LINE_STRIP );
// 		glVertex2f(0, s.thresh[chan]);
// 		glVertex2f(s.n_samps_per_chan, s.thresh[chan]);
// 	glEnd();		

// 	glDisable(GL_LINE_STIPPLE);

// 	char str[500] = {0};
	
// 	/*if(convertLabelUnits)
// 		sprintf(str, "%duV", ad16ToUv(s.thresh[chan], s.gains[chan]));
// 	else
// 		sprintf(str, "%d", (int) s.thresh[chan]);*/
// 	makeLabel(s.thresh[chan], s.gains[chan], convertLabelUnits, str);
	
// //	printf(str);
	
// 	float yOffset = (ylims[1] - ylims[0])/BaseUIElement::height * 2;
// 	drawString(1 ,s.thresh[chan] + yOffset, GLUT_BITMAP_8_BY_13, str);
// }


// void ProjectionAxes::plotProjection(int proj){
// //	std::cout<<"ProjectionAxes::plotProjection():"<<proj<<" not yet implemented"<<std::endl;
// 	// if (proj<PROJ1x2 || proj>PROJ3x4)
// 		// error("ProjectionAxes:plotProjection() invalid projection specified");
    
// 	setViewportRange(xlims[0], ylims[0], xlims[1], ylims[1]);

// 	int d1, d2;
//     n2ProjIdx(proj, &d1, &d2);
	
// 	int idx1, idx2;
// 	calcWaveformPeakIdx(d1,d2,&idx1, &idx2);

// 	if (drawGrid)
// 		drawProjectionGrid(s.gains[d1], s.gains[d2]);

// 	glColor3fv(pointColor);
// 	glPointSize(1);

// 	glBegin(GL_POINTS);
//         glVertex2f(s.data[idx1], s.data[idx2]);
// 	glEnd();
// }

// void ProjectionAxes::calcWaveformPeakIdx(int d1, int d2, int *idx1, int *idx2){

// 	int max1 = -1*pow(2,15);
// 	int max2 = max1;
	
// 	for (int i=0; i<s.n_samps_per_chan ; i++){
// 		if (s.data[i*s.n_chans + d1] > max1)
// 		{	
// 			*idx1 = i*s.n_chans + d1;
// 			max1 = s.data[*idx1];
// 		}
// 		if (s.data[i*s.n_chans + d2] > max2)
// 		{	
// 			*idx2 = i*s.n_chans + d2;
// 			max2 = s.data[*idx2];
// 		}
// 	}
// }
// void ProjectionAxes::drawWaveformGrid(int thold, int gain){

// 	double voltRange = ylims[1] - ylims[0];
// 	double pixelRange = BaseUIElement::height;
// 	//This is a totally arbitrary value that i'll mess around with and set as a macro when I figure out a value I like
// 	int minPixelsPerTick = 25;
// 	int MAX_N_TICKS = 10;

// 	int nTicks = pixelRange / minPixelsPerTick;
// 	while(nTicks>MAX_N_TICKS){
// 		minPixelsPerTick += 5;
// 		nTicks = pixelRange / minPixelsPerTick;
// 	}
// 	int voltPerTick = (voltRange / nTicks);
// 	// Round to the nearest 200

	
// 	double meanRange = voltRange/2;
// 	glColor3fv(gridColor);

// 	glLineWidth(1);
// 	char str[200] = {0}; 
	
// 	double tickVoltage = thold;
// 	while(tickVoltage < ylims[1] - voltPerTick/2) // Draw the ticks above the thold line
// 	{
// 		tickVoltage = roundUp(tickVoltage + voltPerTick, 100);
		
// 		glBegin(GL_LINE_STRIP);
// 		glVertex2i(0, tickVoltage);
// 		glVertex2i(s.n_samps_per_chan, tickVoltage);
// 		glEnd();
		
// 		makeLabel(tickVoltage, gain, convertLabelUnits, str);
// 		drawString(1, tickVoltage+voltPerTick/10, GLUT_BITMAP_8_BY_13, str);
// 	}
	
// 	tickVoltage = thold;
// 	while(tickVoltage > ylims[0] + voltPerTick) // draw the ticks below the thold line
// 	{
// 		tickVoltage = roundUp(tickVoltage - voltPerTick, 100);

// 		glBegin(GL_LINE_STRIP);
// 		glVertex2i(0, tickVoltage);
// 		glVertex2i(s.n_samps_per_chan, tickVoltage);
// 		glEnd();
			
// 		makeLabel(tickVoltage, gain, convertLabelUnits, str);
// 		drawString(1, tickVoltage+voltPerTick/10, GLUT_BITMAP_8_BY_13, str);
// 	}
	
	
// }
// void ProjectionAxes::drawProjectionGrid(int gain1, int gain2){
// 	return; // Disabled method, figure out how you want to implement this in the future
	
// 	double voltRange = ylims[1] - ylims[0];
// 	double threeQuarters  = voltRange * 3.0f / 4.0f;
	
// 	double pixelRange = BaseUIElement::height;
// 	//This is a totally arbitrary value that i'll mess around with and set as a macro when I figure out a value I like
// 	int minPixelsPerTick = 50;

// 	int nTicks = pixelRange / minPixelsPerTick;
// 	int voltPerTick = (voltRange / nTicks);
// 	// Round to the nearest 200


// 	double meanRange = voltRange/2;
// 	glColor3fv(gridColor);

// 	glLineWidth(1);
// 	char str[100] = {0};
	
// 	makeLabel(threeQuarters, gain1, convertLabelUnits, str);
// 	drawString(threeQuarters, (ylims[1] - ylims[0]) / 100, GLUT_BITMAP_8_BY_13, str );
// 	glBegin(GL_LINE_STRIP);
// 	glVertex2f(threeQuarters, ylims[0]);
// 	glVertex2f(threeQuarters, ylims[1]);
// 	glEnd();
// 	glBegin(GL_LINE_STRIP);
// 	glVertex2f(ylims[0], threeQuarters);
// 	glVertex2f(ylims[1], threeQuarters);
// 	glEnd();
	
	
// 	makeLabel(threeQuarters, gain2, convertLabelUnits, str);
	
// 	drawString( ylims[0] + (ylims[1] - ylims[0])/25, threeQuarters, GLUT_BITMAP_8_BY_13, str );
	
		
// }
// void ProjectionAxes::setWaveformColor(GLfloat r, GLfloat g, GLfloat b){
// 	waveColor[0] = r;
// 	waveColor[1] = g;
// 	waveColor[2] = b;
// }
// void ProjectionAxes::setThresholdColor(GLfloat r, GLfloat g, GLfloat b){
// 	thresholdColor[0] = r;
// 	thresholdColor[1] = g;
// 	thresholdColor[2] = b;
// }
// void ProjectionAxes::setPointColor(GLfloat r, GLfloat g, GLfloat b){
// 	pointColor[0] = r;
// 	pointColor[1] = g;
// 	pointColor[2] = b;
// }
// void ProjectionAxes::setGridColor(GLfloat r, GLfloat g, GLfloat b){
// 	gridColor[0] = r;
// 	gridColor[1] = g;
// 	gridColor[2] = b;
// }
// void ProjectionAxes::setPosition(int x, int y, double w, double h){
// 	BaseUIElement::setPosition(x,y,w,h);	
// 	resizedFlag = true;
// }
// void ProjectionAxes::clearOnNextDraw(bool b){
// 	BaseUIElement::clearOnNextDraw(b);
// }