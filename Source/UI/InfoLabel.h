/*
  ==============================================================================

    InfoLabel.h
    Created: 26 Jan 2012 12:52:07pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __INFOLABEL_H_14DA9A62__
#define __INFOLABEL_H_14DA9A62__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/Visualization/OpenGLCanvas.h"

class InfoLabel : public OpenGLCanvas

{
public: 
	InfoLabel();
	~InfoLabel();
	void newOpenGLContextCreated();
	void renderOpenGL();

private:

	int xBuffer, yBuffer;

	void drawLabel();

	int getTotalHeight();

	void resized();
	void mouseDown(const MouseEvent& e);
	void mouseDrag(const MouseEvent& e);
	void mouseMove(const MouseEvent& e);
	void mouseUp(const MouseEvent& e);
	void mouseWheelMove(const MouseEvent&, float, float);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoLabel);
	
};




#endif  // __INFOLABEL_H_14DA9A62__
