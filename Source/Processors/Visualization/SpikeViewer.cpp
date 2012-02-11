/*
  ==============================================================================

    SpikeViewer.cpp
    Created: 16 Aug 2011 8:34:41pm
    Author:  jsiegle

  ==============================================================================
*/

#include "SpikeViewer.h"


SpikeViewer::SpikeViewer(AudioSampleBuffer* sBuffer, MidiBuffer* eBuffer, UIComponent* ui)
	: Renderer(sBuffer, eBuffer, ui)
{

}



SpikeViewer::~SpikeViewer() {}

void SpikeViewer::resized() 
{
	
	xBox = getWidth()/4;
	yBox = getHeight()/2;
	yPadding = 5;
	xPadding = 5;

	glClear(GL_COLOR_BUFFER_BIT);

	clearWaveforms();
	clearProjections();

}

void SpikeViewer::newOpenGLContextCreated()
{

	glClearDepth (1.0);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho(0, 1, 1, 0, 0, 1);
	glMatrixMode (GL_MODELVIEW);
	
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	resized();

	glFlush();

}

void SpikeViewer::clearWaveforms()
{
	
	for (int n = 0; n < 4; n++)
	{
		setViewportForWaveN(n);
		drawBorder();
	}

}

void SpikeViewer::clearProjections()
{
	for (int n = 0; n < 6; n++)
	{
		setViewportForProjectionN(n);
		drawBorder();
	}

}

void SpikeViewer::renderOpenGL()
{
		
	 if (eventBuffer->getNumEvents() > 0) {

	   	glRasterPos2f(0.1,0.1);

	   	//const char* str = "i";
	  // 	void* font = GLUT_BITMAP_8_BY_13;

	  // 	glutBitmapCharacter(font,54);
	   //	drawBorder();

		//std::cout << "Events received by Spike Viewer." << std::endl;

		MidiBuffer::Iterator i (*eventBuffer);
		MidiMessage message(0xf4);

		int samplePosition;
		i.setNextSamplePosition(samplePosition);

		//Array<int> peaks;


		clearWaveforms();

		while (i.getNextEvent (message, samplePosition)) {

			int numbytes = message.getRawDataSize();
			int numSamples = (numbytes-2)/2;
			uint8* dataptr = message.getRawData();

			int chan = (*dataptr<<8) + *(dataptr+1);
			int electrode = config->getSource(0)->getElectrodeNumberForChannel(chan);

			//std::cout << chan << "::" << electrode << std::endl;

 			dataptr += 2;

			//glViewport(0,0,getWidth()/2,getHeight());

			if (electrode == 0)
			{
			//for (int n = 0; n < 4; n++) {
				setViewportForWaveN(chan);
				float peak = drawWaveform(dataptr, numSamples);

				peaks.set(chan,peak*1.25);
				//peaks.set(chan,peak);
				
			}

			if (peaks.size() == 4)
			{
				drawProjections();
				peaks.clear();
			}

			//std::cout << " Bytes received: " << numbytes << std::endl;
			//std::cout << " Message timestamp = " << message.getTimeStamp() << std::endl;
			//std::cout << " Message channel: " << chan << std::endl;

 			//std::cout << "   ";
			
 			//AudioDataConverters::convertInt16BEToFloat ( dataptr, // source
    		//			spikeData, // dest
    		//			numSamples, // numSamples
    		//			2 ); // destBytesPerSample = 2

			//for (int n = 0; n < numSamples; n++) {
			//	std::cout << String(spikeData[n]) << " ";
			//}
					
			//std::cout << std::endl << std::endl;
		}

		// for (int ch = 0; ch < 4; ch++)
		// {
			
		// }

		//eventBuffer->clear();

	}

	//glOrtho(0, 0.5, 0.5, 0, 0, 1);
	glFlush();

}

void SpikeViewer::drawProjections() 
{
	
	for (int i = 0; i < 6; i++)
	{
		setViewportForProjectionN(i);

		int d1,d2;
		
		if (i == 0){
			d1 = 0;
			d2 = 1;	
		} else if (i == 1) {
			d1 = 0;
			d2 = 2;
		} else if (i == 2) {
			d1 = 0;
			d2 = 3;
		} else if (i == 3) {
			d1 = 1;
			d2 = 2;
		} else if (i == 4) {
			d1 = 1;
			d2 = 3;
		} else if (i == 5) {
			d1 = 2;
			d2 = 3;
		}

		glColor3f(0.0, 0.0, 0.0);
		glBegin(GL_POINTS);
			glVertex2f(1-peaks[d1],peaks[d2]);
		glEnd();

	}

}

float SpikeViewer::drawWaveform(uint8* dataptr, int numSamples)
{

	glColor3f(0,0.0,0);
	
	glBegin(GL_LINE_STRIP);

	float maxVal = 0;

 	for (int n = 0; n < numSamples; n++)
 	{
 		uint16 sampleValue = (*dataptr << 8) + *(dataptr+1);

 		float sampleVal = -float(sampleValue - 32768)/32768 + 0.6;

 		(sampleVal > maxVal) ? maxVal = sampleVal : maxVal = maxVal;

 		glVertex2f(float(n)/numSamples + 0.01, 
 					sampleVal);
 		dataptr += 2;

 	}

	glEnd();

	return maxVal;

}

void SpikeViewer::drawBorder()
{

	 glColor3f(0.9,0.7,0.2);

	 glRectf(0.01f,0.01f,0.99f,0.99f);

    // glBegin(GL_LINE_STRIP);
 	//  glVertex2f(0.01f, 0.01f);
 	//  glVertex2f(0.99f, 0.01f);
 	//  glVertex2f(0.99f, 0.99f);
 	//  glVertex2f(0.01f, 0.99f);
 	//  glVertex2f(0.01f, 0.01f);
 	//  glEnd();
}

void SpikeViewer::setViewportForProjectionN(int n)
{
	//	std::cout<<"Setting viewport on projection:"<<n<<std::endl;
    float viewDX = xBox;
    float viewDY = yBox;
    float viewX,viewY;

    switch (n){
    case 0:
        viewX=xBox;
        viewY=yBox;
        break;
    case 1:
        viewX = xBox*2;
        viewY = yBox;
        break;
    case 2:
        viewX=xBox*3;
        viewY=yBox;
        break;
    case 3:
        viewX = xBox;
        viewY = 0;
        break;
    case 4:
        viewX = xBox*2;
        viewY = 0;
        break;
    case 5:
        viewX = xBox*3;
        viewY = 0;
        break;
    default:
        std::cout<<"drawing of more than 4 channels is not supported, returning! Requested:"<<n<<std::endl;
        return;
    }
	viewX = viewX + xPadding;
	viewY = viewY + yPadding;
	viewDX = viewDX - 2*xPadding;
	viewDY = viewDY - 2*yPadding;

	glViewport(viewX, viewY, viewDX, viewDY);

}


void SpikeViewer::setViewportForWaveN(int n){
	float viewDX = xBox/2;
	float viewDY = yBox;
	float viewX,viewY;
	switch (n){
	case 0:
		viewX=0;
		viewY=yBox;
		break;
	case 1:
		viewX = xBox/2;
		viewY = yBox;
		break;
	case 2:
		viewX=0;
		viewY=0;
		break;
	case 3:
		viewX = xBox/2;
		viewY = 0;
		break;
	default:
		std::cout<<"drawing of more than 4 channels is not supported, returning! Requested:"<<n<<std::endl;
		return;
	}
	viewX = viewX + xPadding;
	viewY = viewY + yPadding;
	viewDX = viewDX - 2*xPadding;
	viewDY = viewDY - 2*yPadding;
	glViewport(viewX,viewY,viewDX,viewDY);
}