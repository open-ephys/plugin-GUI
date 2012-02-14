/*
  ==============================================================================

    ControlPanel.h
    Created: 1 May 2011 2:57:48pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __CONTROLPANEL_H_AD81E528__
#define __CONTROLPANEL_H_AD81E528__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Audio/AudioComponent.h"
#include "../Processors/Editors/AudioEditor.h"
#include "../Processors/ProcessorGraph.h"
#include "../Processors/RecordNode.h"
#include "CustomLookAndFeel.h"


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

class PlayButton : public DrawableButton
{
	public:
		PlayButton();
		~PlayButton();
};

class RecordButton : public DrawableButton
{
	public:
		RecordButton();
		~RecordButton();
};

class CPUMeter : public Label//, public Timer //Component
{
	public:
		CPUMeter();
		~CPUMeter();

		void updateCPU(float usage);

		void paint (Graphics& g);
	
	private:
		float cpu;
		float lastCpu;

		//void timerCallback() {repaint();}

};

class DiskSpaceMeter : public Component//, public Timer
{
public:
	DiskSpaceMeter();
	~DiskSpaceMeter();

	void updateDiskSpace(float percent);

	void paint (Graphics& g);

private:
	float diskFree;
	ProcessorGraph* graph;
	//void timerCallback() {repaint();}
	
};

class Clock : public OpenGLComponent
{
	public:
		Clock();
		~Clock();

		void newOpenGLContextCreated();
		void renderOpenGL();

		void start();
		void stop();

		void startRecording();
		void stopRecording();

	private:

		void drawTime();

		int64 lastTime;

		int64 totalTime;
		int64 totalRecordTime;

		bool isRunning;
		bool isRecording;

		FTPixmapFont* font;
};



class ControlPanel : public Component, 
					 public Button::Listener,
					 public ActionListener,
					 public Timer

{
public:
	ControlPanel(ProcessorGraph* graph, AudioComponent* audio);
	~ControlPanel();

	void disableCallbacks();

private:	
	PlayButton* playButton;
	RecordButton* recordButton;
	Clock* masterClock;
	CPUMeter* cpuMeter;
	DiskSpaceMeter* diskMeter;
	AudioComponent* audio;
	ProcessorGraph* graph;
	AudioEditor* audioEditor;

	void paint(Graphics& g);

	void resized();
	void buttonClicked(Button* button);

	void actionListenerCallback(const String& msg);

	void timerCallback();

	Font font;

};


#endif  // __CONTROLPANEL_H_AD81E528__
