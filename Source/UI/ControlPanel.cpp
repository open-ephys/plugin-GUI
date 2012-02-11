/*
  ==============================================================================

    ControlPanel.cpp
    Created: 1 May 2011 2:57:48pm
    Author:  jsiegle

  ==============================================================================
*/

#include "ControlPanel.h"
#include <stdio.h>


PlayButton::PlayButton()
	: DrawableButton (T("PlayButton"), DrawableButton::ImageFitted)
{

		DrawablePath normal, over, down;

        Path p;
        p.addTriangle (0.0f, 0.0f, 0.0f, 20.0f, 18.0f, 10.0f);
        normal.setPath (p);
        normal.setFill (Colours::lightgrey);
        normal.setStrokeThickness (0.0f);

        over.setPath (p);
        over.setFill (Colours::black);
        over.setStrokeFill (Colours::black);
        over.setStrokeThickness (5.0f);

        down.setPath (p);
        down.setFill (Colours::pink);
        down.setStrokeFill (Colours::pink);
        down.setStrokeThickness (5.0f);

        setImages (&normal, &over, &over);
        setBackgroundColours(Colours::darkgrey, Colours::yellow);
        setClickingTogglesState (true);
        setTooltip ("Start/stop acquisition");
}

PlayButton::~PlayButton()
{	
}

RecordButton::RecordButton()
	: DrawableButton (T("RecordButton"), DrawableButton::ImageFitted)
{

		DrawablePath normal, over, down;

        Path p;
        p.addEllipse (0.0,0.0,20.0,20.0);
        normal.setPath (p);
        normal.setFill (Colours::lightgrey);
        normal.setStrokeThickness (0.0f);

        over.setPath (p);
        over.setFill (Colours::black);
        over.setStrokeFill (Colours::black);
        over.setStrokeThickness (5.0f);

        setImages (&normal, &over, &over);
        setBackgroundColours(Colours::darkgrey, Colours::red);
        setClickingTogglesState (true);
        setTooltip ("Start/stop writing to disk");
}

RecordButton::~RecordButton()
{	
}


CPUMeter::CPUMeter() : Label(T("CPU Meter"),"0.0"), cpu(0.0f), lastCpu(0.0f)
{
	//startTimer(100);
	//repaint();
}

CPUMeter::~CPUMeter()
{
}

void CPUMeter::updateCPU(float usage) {
	lastCpu = cpu;
	cpu = usage;
}

void CPUMeter::paint(Graphics& g)
{
	g.fillAll(Colours::grey);
	
	g.setColour(Colours::yellow);
	g.fillRect(0.0f,0.0f,getWidth()*cpu,float(getHeight()));

	g.setColour(Colours::black);
	g.drawRect(0,0,getWidth(),getHeight(),1);

}


DiskSpaceMeter::DiskSpaceMeter()

{	
	//graph = g;
	//updateDiskSpace(graph->getRecordNode()->getFreeSpace());
	//repaint();
	//startTimer(10000); // refresh every 10 seconds
}


DiskSpaceMeter::~DiskSpaceMeter()
{
}

void DiskSpaceMeter::updateDiskSpace(float percent)
{
	diskFree = percent;
}

void DiskSpaceMeter::paint(Graphics& g)
{

	g.fillAll(Colours::grey);
	
	g.setColour(Colours::pink);
	g.fillRect(0.0f,0.0f,getWidth()*diskFree,float(getHeight()));

	g.setColour(Colours::black);
	g.drawRect(0,0,getWidth(),getHeight(),1);
	
}

Clock::Clock() : Label(T("Clock"),"00:00.00")
{
	setColour(Label::textColourId, Colours::white);
}

Clock::~Clock()
{
}

ControlPanel::ControlPanel(ProcessorGraph* graph_, AudioComponent* audio_) : 
			graph (graph_), audio(audio_)
{

	audioEditor = (AudioEditor*) graph->getAudioNode()->createEditor();
	addChildComponent(audioEditor);

	playButton = new PlayButton();
	playButton->addListener (this);
	addAndMakeVisible(playButton);

	recordButton = new RecordButton();
	recordButton->addListener (this);
	addAndMakeVisible(recordButton);

	masterClock = new Clock();
	addAndMakeVisible(masterClock);

	cpuMeter = new CPUMeter();
	addAndMakeVisible(cpuMeter);

	diskMeter = new DiskSpaceMeter();
	addAndMakeVisible(diskMeter);

	startTimer(100);

	if (1) {
	MemoryInputStream mis(BinaryData::misoserialized, BinaryData::misoserializedSize, false);
	Typeface::Ptr typeface = new CustomTypeface(mis);
	font = Font(typeface);
	}
}

ControlPanel::~ControlPanel()
{
	//deleteAllChildren() -> if this is used, audioEditor will be deleted
	deleteAndZero(playButton);
	deleteAndZero(recordButton);
	deleteAndZero(masterClock);
	deleteAndZero(cpuMeter);
	deleteAndZero(diskMeter);
	//audioEditor will delete itself

	graph = 0;
}

void ControlPanel::paint(Graphics& g)
{
	g.setColour(Colour(40,40,40));
	g.fillRoundedRectangle(0,0,getWidth(),getHeight(),8.0f);

	g.setFont(font);
	g.setColour(Colours::white);
	g.drawText("CONTROL PANEL",getWidth()/2,0,getWidth(),getHeight(),Justification::left,true);
}

void ControlPanel::resized()
{
	int w = getWidth();
	int h = getHeight();

	if (playButton != 0)
		playButton->setBounds(w-h*5,5,h-5,h-10);
	
	if (recordButton != 0)
		recordButton->setBounds(w-h*4,5,h-5,h-10);

	if (masterClock != 0)
		masterClock->setBounds(w-h*3,0,h*2,h);
	
	if (cpuMeter != 0)
		cpuMeter->setBounds(20,h/4,h*4,h/2);

	if (diskMeter != 0)
		diskMeter->setBounds(150,h/4,h*4,h/2);

	if (audioEditor != 0)
		audioEditor->setBounds(w-h*12,5,h*5,h-10);
}

void ControlPanel::buttonClicked(Button* button) 

{
	if (button == recordButton)
	{
		std::cout << "Record button pressed." << std::endl;
		if (recordButton->getToggleState())
		{
			playButton->setToggleState(true,true);
			graph->getRecordNode()->setParameter(1,10.0f); // turn on recording

		} else {
			graph->getRecordNode()->setParameter(0,10.0f); // turn off recording

		}

	} else if (button == playButton) {
		std::cout << "Play button pressed." << std::endl;
		if (!playButton->getToggleState())
		{
			recordButton->setToggleState(false,true);
		}

	}

	if (playButton->getToggleState())
	{

		if (!audio->callbacksAreActive()) {
			if (graph->enableProcessors())
				audio->beginCallbacks();
			else
				playButton->setToggleState(false, false);
		}

	} else {

		if (audio->callbacksAreActive()) {
			audio->endCallbacks();
			graph->disableProcessors();
			cpuMeter->updateCPU(0.0f);
		}

	}

}

void ControlPanel::actionListenerCallback(const String & msg)
{
	//std::cout << "Message Received." << std::endl;
	if (playButton->getToggleState()) {
		cpuMeter->updateCPU(audio->getCpuUsage());
	}

	cpuMeter->repaint();

	diskMeter->updateDiskSpace(graph->getRecordNode()->getFreeSpace());
	diskMeter->repaint();
	
	
}

void ControlPanel::timerCallback()
{
	//std::cout << "Message Received." << std::endl;
	if (playButton->getToggleState()) {
		cpuMeter->updateCPU(audio->getCpuUsage());
	}

	cpuMeter->repaint();

	diskMeter->updateDiskSpace(graph->getRecordNode()->getFreeSpace());
	diskMeter->repaint();
	
	
}