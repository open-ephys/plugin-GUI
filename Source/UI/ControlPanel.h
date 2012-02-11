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

class Clock : public Label
{
	public:
		Clock();
		~Clock();
};



class ControlPanel : public Component, 
					 public Button::Listener,
					 public ActionListener,
					 public Timer

{
public:
	ControlPanel(ProcessorGraph* graph, AudioComponent* audio);
	~ControlPanel();

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
