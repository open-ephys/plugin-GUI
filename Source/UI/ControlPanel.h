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

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Audio/AudioComponent.h"
#include "../Processors/Editors/AudioEditor.h"
#include "../Processors/ProcessorGraph.h"
#include "../Processors/RecordNode.h"
#include "CustomLookAndFeel.h"
#include "../AccessClass.h"
#include "../Processors/Editors/GenericEditor.h" // for UtilityButton
#include "../Processors/Visualization/OpenGLCanvas.h"

#include "../OpenGL.h"


/**
  
  Toggles data acquisition on and off.

  The PlayButton is located in the ControlPanel. Clicking it toggles the state
  of the ProcessorGraph to either begin the callbacks that drive data through
  the graph (acquisition on) or end these callbacks (acquisition off). 

  Acquisition can also be started by pressing the RecordButton
  (assuming callbacks are not already active).

  @see ControlPanel, ProcessorGraph

*/


class PlayButton : public DrawableButton
{
	public:
		PlayButton();
		~PlayButton();
};

/**
  
  Toggles recording on and off.

  The RecordButton is located in the ControlPanel. Clicking it toggles the 
  state of the RecordNode to either begin saving data (recording on) or 
  stop saving data (recording off).

  If the RecordButton is pressed while data acquisition is inactive, it 
  will automatically start data acquisition before recording.

  @see ControlPanel, RecordNode

*/

class RecordButton : public DrawableButton
{
	public:
		RecordButton();
		~RecordButton();
};

/**
  
  Displays the CPU load used up by the data processing callbacks.

  The CPUMeter is located in the ControlPanel. Whenever acquisition is active,
  it uses a built-in JUCE method to display the CPU load required to run the ProcessorGraph.

  It's not clear how accurate the meter is, nor how it deals with CPUs using multiple cores.

  For a more accurate measurement of CPU load, it's recommended to use a graphical
  interface or type 'top' inside a terminal. 

  @see ControlPanel

*/

class CPUMeter : public Label
{
	public:
		CPUMeter();
		~CPUMeter();

		/** Updates the load level displayed by the CPUMeter. Called by
		     the ControlPanel. */
		void updateCPU(float usage);

		/** Draws the CPUMeter. */
		void paint (Graphics& g);
	
	private:

		Font font;

		float cpu;
		float lastCpu;

};

/**
  
  Displays the amount of disk space left in the current data directory.

  The DiskSpaceMeter is located in the ControlPanel. When the GUI is launched (or the data directory
  is changed), a built-in JUCE method is used to find the amount of free space.

  Note that the DiskSpaceMeter currently displays only relative, not absolute disk space.

  @see ControlPanel

*/

class DiskSpaceMeter : public Component
{
public:
	DiskSpaceMeter();
	~DiskSpaceMeter();

	/** Updates the free disk space displayed by the DiskSpaceMeter. Called by
		the ControlPanel. */
	void updateDiskSpace(float percent);

	/** Draws the DiskSpaceMeter. */
	void paint (Graphics& g);

private:

	Font font;

	float diskFree;
	
};

/**
  
  Displays the time.

  The Clock is located in the ControlPanel. If acquisition (but not recording) is 
  active, it displays (in yellow) the cumulative amount of time that the GUI has been acquiring data since
  the application was launched. If recording is active, the Clock displays (in red) the 
  cumulative amount of time that recording has been active.

  The Clock uses built-in JUCE functions for getting the system time. It does not
  currently interact with timestamps from ProcessorGraph sources.

  The Clock draws the time using OpenGL (and the FTGL font library).

  @see ControlPanel

*/

class Clock : public OpenGLCanvas
{
	public:
		Clock();
		~Clock();

		/** Initializes an OpenGL context in which drawing occurs.*/
		void newOpenGLContextCreated();

		/** Draws the current time.*/
		void renderOpenGL();

		/** Starts the acquisition (yellow) clock.*/
		void start();

		/** Stops the acquisition (yellow) clock.*/
		void stop();

		/** Starts the recording (red) clock.*/
		void startRecording();

		/** Stops the recording (red) clock.*/
		void stopRecording();

		/** Sets the cumulative recording time to zero.*/
		void resetRecordTime();

	private:

		/** Draws the current time.*/
		void drawTime();

		int64 lastTime;

		int64 totalTime;
		int64 totalRecordTime;

		bool isRunning;
		bool isRecording;

		//FTPixmapFont* font;
};

/**
  
  Used to show and hide the file browser within the ControlPanel.

  The ControlPanel contains a JUCE FilenameComponent used to change the 
  data directory. When not in use, this component can be hidden using
  the ControlPanelButton.

  @see ControlPanel

*/

class ControlPanelButton : public OpenGLCanvas
{
public:
	ControlPanelButton(ControlPanel* cp_);
	~ControlPanelButton();

	/** Returns the open/closed state of the ControlPanelButton.*/
	bool isOpen() {return open;}

	/** Toggles the open/closed state of the ControlPanelButton.*/
	void toggleState();

	/** Initializes an OpenGL context in which drawing occurs.*/
	void newOpenGLContextCreated();

	/** Draws the button. */
	void renderOpenGL();

	/** Draws the button. */
	void drawButton();

	/** Responds to mouse clicks within the button. */
	void mouseDown(const MouseEvent& e);

private:	

	ControlPanel* cp;

	bool open;

};

class UtilityButton;

/**
  
  Provides general application controls along the top of the MainWindow.

  Displays useful information and provides buttons to control acquistion and recording.

  The ControlPanel contains the PlayButton, the RecordButton, the CPUMeter,
  the DiskSpaceMeter, the Clock, the AudioEditor, and a FilenameComponent for switching the
  current data directory.

  @see UIComponent

*/

class ControlPanel : public Component, 
					 public Button::Listener,
					 public Timer,
					 public AccessClass

{
public:
	ControlPanel(ProcessorGraph* graph, AudioComponent* audio);
	~ControlPanel();

	/** Disables the callbacks of the ProcessorGraph (used to
	    drive data acquisition).*/
	void disableCallbacks();

	/** Returns a pointer to the AudioEditor.*/
	AccessClass* getAudioEditor() {return (AccessClass*) audioEditor;}

	/** Sets whether or not the FilenameComponent is visible.*/
	void openState(bool isOpen);
	
	/** Toggles the visibility of the FilenameComponent.*/
	void toggleState();

	/** Used to manually turn recording on and off.*/
	void setRecordState(bool isRecording);

	/** Returns a boolean that indicates whether or not the FilenameComponet
	    is visible. */
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

	/** Adds the RecordNode as a listener of the FilenameComponent
	(so it knows when the data directory has changed).*/
	void updateChildComponents();

	void timerCallback();

	/** Updates the values displayed by the CPUMeter and DiskSpaceMeter.*/
	void refreshMeters();

	bool keyPressed(const KeyPress &key);

	Font font;

	bool open;

	Path p1, p2;

	/** Draws the boundaries around the FilenameComponent.*/
	void createPaths();

};


#endif  // __CONTROLPANEL_H_AD81E528__
