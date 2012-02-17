/*
  ==============================================================================

    UIComponent.h
    Created: 30 Apr 2011 8:33:05pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __UICOMPONENT_H_D97C73CF__
#define __UICOMPONENT_H_D97C73CF__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "InfoLabel.h"
#include "ControlPanel.h"
#include "FilterList.h"
#include "FilterViewport.h"
#include "DataViewport.h"
#include "MessageCenter.h"
#include "Configuration.h"
#include "../Processors/DisplayNode.h"
#include "../Processors/ProcessorGraph.h"
#include "../Audio/AudioComponent.h"

#ifdef _WIN32
#include <windows.h>
#endif

#if JUCE_WINDOWS
#include <gl/gl.h>
#include <gl/glu.h>
#elif JUCE_LINUX
#include <GL/gl.h>
#include <GL/glut.h>
#undef KeyPress
#elif JUCE_IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#elif JUCE_MAC
#include <GLUT/glut.h>
#endif

#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80e1
#endif

#include <FTGL/ftgl.h>

class FilterViewportButton;

class UIComponent : public Component,
				    public ActionBroadcaster,
				    public DragAndDropContainer // required for 
				    				            // drag-and-drop
				    				            // internal components

{
public: 
	UIComponent(ProcessorGraph* pgraph, AudioComponent* audio);
	~UIComponent();

	FilterViewport* getFilterViewport() {return filterViewport;}
	DataViewport* getDataViewport() {return dataViewport;}
	Configuration* getConfiguration() {return config;}

	//void transmitMessage(const String& message);
	void disableCallbacks();

	void childComponentChanged();

private:

	DataViewport* dataViewport;
	FilterViewport* filterViewport;
	FilterViewportButton* filterViewportButton;
	FilterList* filterList;
	ControlPanel* controlPanel;
	MessageCenter* messageCenter;
	Configuration* config;
	InfoLabel* infoLabel;

	ProcessorGraph* processorGraph;
	AudioComponent* audio;

	void resized();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIComponent);
	
};



class FilterViewportButton : public OpenGLComponent
{
public:
	FilterViewportButton(UIComponent* ui);
	~FilterViewportButton();

	bool isOpen() {return open;}

	void newOpenGLContextCreated();
	void renderOpenGL();

	void drawName();
	void drawButton();

	void mouseDown(const MouseEvent& e);

private:	

	UIComponent* UI;
	bool open;

	FTPixmapFont* font;

};

#endif  // __UICOMPONENT_H_D97C73CF__
