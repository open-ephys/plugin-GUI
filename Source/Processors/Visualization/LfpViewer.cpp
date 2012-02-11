/*
  ==============================================================================

    LfpViewer.cpp
    Created: 16 Aug 2011 8:46:57pm
    Author:  jsiegle

  ==============================================================================
*/

#include "LfpViewer.h"


LfpViewer::LfpViewer(AudioSampleBuffer* sBuffer, MidiBuffer* eBuffer, UIComponent* ui)
	: Renderer(sBuffer, eBuffer, ui)
{
	addMouseListener (this, true);
}

LfpViewer::~LfpViewer() {}

void LfpViewer::newOpenGLContextCreated()
{
	glClearColor(0.6f, 0.6f, 0.4f, 1.0f);
	glClearDepth (1.0);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho(0, 1, 1, 0, 0, 1);
	glMatrixMode (GL_MODELVIEW);

		
	//glDepthFunc (GL_LESS);
	//glEnable (GL_DEPTH_TEST);
	//glEnable (GL_TEXTURE_2D);
	glEnable (GL_BLEND);
	glShadeModel(GL_FLAT);
	
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor3f(1,1,1);

	glFlush();

}

void LfpViewer::renderOpenGL()
{

	//std::cout << "Painting..." << std::endl;
		
	glClear(GL_COLOR_BUFFER_BIT);

	//glMatrixMode (GL_PROJECTION);
	//glLoadIdentity ();
	//glOrtho(0, 1, 1, 0, 0, 1);
	//glMatrixMode (GL_MODELVIEW);


	int skip = 1;

	int nSamples = streamBuffer->getNumSamples();


	for (int chan = 0; chan < streamBuffer->getNumChannels(); chan++) {
	
		glBegin(GL_LINE_STRIP);

		//std::cout << "Message Received." << std::endl;
		if (chan % 4 == 0)
			glColor3f(0.0,0.0,0);//1.0*chan/16,1.0*chan/16,1.0*chan/16);
		else if (chan % 4 == 1)
			glColor3f(0,0.0,0);
		else if (chan % 4 == 2)
			glColor3f(0.0,0.0,0);
		else
			glColor3f(0.0,0.0,0.0);

	
		for (int n = 0; n < nSamples-skip; n+= skip )
		{
			glVertex2f(float(n)/nSamples,*streamBuffer->getSampleData(chan,n)/0.5+0.03+chan*0.06);
			glVertex2f(float(n+skip)/nSamples,*streamBuffer->getSampleData(chan,n+skip)/0.5+0.03+chan*0.06);
		}
		
		//std::cout << *streamBuffer->getSampleData(0,0) << std::endl;

		glEnd();

	}



	glFlush();
		
	
}

void LfpViewer::mouseDown (const MouseEvent& e)
{
	
	// std::cout << "Mouse click at " << 
	// 			 e.getMouseDownX() <<
	// 			 ", " << 
	// 			 e.getMouseDownY() << std::endl;

}