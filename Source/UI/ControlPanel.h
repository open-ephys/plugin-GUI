/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __CONTROLPANEL_H_AD81E528__
#define __CONTROLPANEL_H_AD81E528__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Audio/AudioComponent.h"
#include "../Processors/Editors/AudioEditor.h"
#include "../Processors/ProcessorGraph.h"
#include "../Processors/RecordNode.h"
#include "CustomLookAndFeel.h"
#include "../AccessClass.h"
#include "../Processors/Editors/GenericEditor.h" // for UtilityButton

#include "../OpenGL.h"


/**
  
  Toggles data acquisition on and off.

  @see ControlPanel

*/


class PlayButton : public DrawableButton
{
	public:
		PlayButton();
		~PlayButton();
};

/**
  
  Toggles recording on and off.

  @see ControlPanel

*/

class RecordButton : public DrawableButton
{
	public:
		RecordButton();
		~RecordButton();
};

/**
  
  Displays the CPU load used up by the data processing callbacks.

  @see ControlPanel

*/

class CPUMeter : public Label
{
	public:
		CPUMeter();
		~CPUMeter();

		void updateCPU(float usage);

		void paint (Graphics& g);
	
	private:

		Font font;

		float cpu;
		float lastCpu;

};

/**
  
  Displays the amount of disk space left in the current data directory.

  @see ControlPanel

*/

class DiskSpaceMeter : public Component
{
public:
	DiskSpaceMeter();
	~DiskSpaceMeter();

	void updateDiskSpace(float percent);

	void paint (Graphics& g);

private:

	Font font;

	float diskFree;
	ProcessorGraph* graph;
	
};

/**
  
  Displays the time.

  @see ControlPanel

*/

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

		void resetRecordTime();

	private:

		void drawTime();

		int64 lastTime;

		int64 totalTime;
		int64 totalRecordTime;

		bool isRunning;
		bool isRecording;

		FTPixmapFont* font;
};

/**
  
  Used to show and hide the file browser within the ControlPanel.

  @see ControlPanel

*/

class ControlPanelButton : public OpenGLComponent
{
public:
	ControlPanelButton(ControlPanel* cp_);
	~ControlPanelButton();

	bool isOpen() {return open;}
	void toggleState();

	void newOpenGLContextCreated();
	void renderOpenGL();

	void drawButton();

	void mouseDown(const MouseEvent& e);

private:	

	ControlPanel* cp;

	bool open;

};

class UtilityButton;

/**
  
  Provides general application controls.

  Displays useful information and provides buttons to control acquistion and recording.

  The ControlPanel is located along the top of the application window.

  @see UIComponent

*/

class ControlPanel : public Component, 
					 public Button::Listener,
					// public ActionListener,
					 public Timer,
					 public AccessClass

{
public:
	ControlPanel(ProcessorGraph* graph, AudioComponent* audio);
	~ControlPanel();

	void disableCallbacks();

	AccessClass* getAudioEditor() {return (AccessClass*) audioEditor;}

	void openState(bool);
	
	void toggleState();

	bool isOpen() {return open;}
 
private:	
	PlayButton* playButton;
	RecordButton* recordButton;
	Clock* masterClock;
	CPUMeter* cpuMeter;
	DiskSpaceMeter* diskMeter;
    ProcessorGraph* graph;
	AudioComponent* audio;
	AudioEditor* audioEditor;
	FilenameComponent* filenameComponent;
	UtilityButton* newDirectoryButton;

	ControlPanelButton* cpb;

	void paint(Graphics& g);

	void resized();
	void buttonClicked(Button* button);

	bool initialize;

	//void actionListenerCallback(const String& msg);

	void updateChildComponents();

	void timerCallback();
	void refreshMeters();

	bool keyPressed(const KeyPress &key);

	Font font;

	bool open;

	Path p1, p2;

	void createPaths();

};


#endif  // __CONTROLPANEL_H_AD81E528__
