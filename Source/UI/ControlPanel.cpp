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

#include "ControlPanel.h"
#include "UIComponent.h"
#include <stdio.h>
#include <math.h>

PlayButton::PlayButton()
	: DrawableButton ("PlayButton", DrawableButton::ImageFitted)
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
	: DrawableButton ("RecordButton", DrawableButton::ImageFitted)
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


CPUMeter::CPUMeter() : Label("CPU Meter","0.0"), cpu(0.0f), lastCpu(0.0f)
{
	MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
        Typeface::Ptr typeface = new CustomTypeface(mis);
        font = Font(typeface);
        font.setHeight(12);
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

	g.setFont(font);
	g.drawSingleLineText("CPU",65,12);

}


DiskSpaceMeter::DiskSpaceMeter()

{	
	MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
        Typeface::Ptr typeface = new CustomTypeface(mis);
        font = Font(typeface);
        font.setHeight(12);
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
	
	g.setColour(Colours::lightgrey);
	if (diskFree > 0)
		g.fillRect(0.0f,0.0f,getWidth()*diskFree,float(getHeight()));

	g.setColour(Colours::black);
	g.drawRect(0,0,getWidth(),getHeight(),1);

	g.setFont(font);
	g.drawSingleLineText("DF",75,12);
	
}

Clock::Clock() : isRunning(false), isRecording(false)
{
	// const unsigned char* buffer = reinterpret_cast<const unsigned char*>(BinaryData::cpmono_light_otf);
	// size_t bufferSize = BinaryData::cpmono_light_otfSize;

	// font = new FTPixmapFont(buffer, bufferSize);

	totalTime = 0;
	totalRecordTime = 0;
}

Clock::~Clock()
{
}

void Clock::newOpenGLContextCreated()
{
	setUp2DCanvas();
	activateAntiAliasing();
	setClearColor(darkgrey);
}

void Clock::renderOpenGL()
{
	glClear(GL_COLOR_BUFFER_BIT);
	drawTime();
}

void Clock::drawTime()
{

	if (isRunning)
	{
		int64 now = Time::currentTimeMillis();
		int64 diff = now - lastTime;
		totalTime += diff;

		if (isRecording)
		{
			totalRecordTime += diff;
		}

		lastTime = Time::currentTimeMillis();
	}

	int m;
	int s;

	if (isRecording)
	{
		glColor4f(1.0, 0.0, 0.0, 1.0);
		m = floor(totalRecordTime/60000.0);
		s = floor((totalRecordTime - m*60000.0)/1000.0);

	} else {
		if (isRunning)
			glColor4f(1.0, 1.0, 0.0, 1.0);
		else
			glColor4f(1.0, 1.0, 1.0, 1.0);
		m = floor(totalTime/60000.0);
		s = floor((totalTime - m*60000.0)/1000.0);
	}

	String timeString = "";

	// if (m < 10)
	// 	String timeString = "  ";
	// else if (m < 100)
		
	timeString += m;
	timeString += " min ";
	timeString += s;
	timeString += " s";

	glRasterPos2f(8.0/getWidth(),0.75f);

	getFont(cpmono_light)->FaceSize(23);
	getFont(cpmono_light)->Render(timeString);


} 

void Clock::start()
{
	if (!isRunning)
	{
		isRunning = true;
		lastTime = Time::currentTimeMillis();
	}
}

void Clock::resetRecordTime()
{
	totalRecordTime = 0;
}

void Clock::startRecording()
{
	if (!isRecording)
	{
		isRecording = true;
		start();
	}
}

void Clock::stop()
{
	if (isRunning)
	{
		isRunning = false;
		isRecording = false;
	}
}

void Clock::stopRecording()
{
	if (isRecording)
	{
		isRecording = false;
	}

}


ControlPanelButton::ControlPanelButton(ControlPanel* cp_) : cp(cp_)
{
	open = false;

}

ControlPanelButton::~ControlPanelButton()
{
	
}

void ControlPanelButton::newOpenGLContextCreated()
{

	setUp2DCanvas();
	activateAntiAliasing();
	setClearColor(darkgrey);
}


void ControlPanelButton::renderOpenGL()
{
	glClear(GL_COLOR_BUFFER_BIT);
	drawButton();
}

void ControlPanelButton::drawButton()
{
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glLineWidth(1.0f);

	glBegin(GL_LINE_LOOP);

	if (open)
	{
		glVertex2f(0.5, 0.8);
		glVertex2f(0.2, 0.2);
	} else {
		glVertex2f(0.8, 0.8);
		glVertex2f(0.2, 0.5);
	}
	glVertex2f(0.8, 0.2);
	glEnd();

}

void ControlPanelButton::mouseDown(const MouseEvent& e)
{
	open = !open;
	cp->openState(open);
	repaint();

}

void ControlPanelButton::toggleState()
{
	open = !open;
	repaint();
}



ControlPanel::ControlPanel(ProcessorGraph* graph_, AudioComponent* audio_) : 
			graph (graph_), audio(audio_), open(false), initialize(true)
{

	if (1) {
		MemoryInputStream mis(BinaryData::misoserialized, BinaryData::misoserializedSize, false);
		Typeface::Ptr typeface = new CustomTypeface(mis);
		font = Font(typeface);
		font.setHeight(15);
	}

	audioEditor = (AudioEditor*) graph->getAudioNode()->createEditor();
	addAndMakeVisible(audioEditor);

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

	cpb = new ControlPanelButton(this);
	addAndMakeVisible(cpb);

	newDirectoryButton = new UtilityButton("+", font);
	newDirectoryButton->setEnabledState(false);
	newDirectoryButton->addListener (this);
	addChildComponent(newDirectoryButton);



	filenameComponent = new FilenameComponent("folder selector",
		 									  File::getCurrentWorkingDirectory().getFullPathName(), 
		 									  true,
		 									  true,
		 									  true,
		 									  "*",
		 									  "",
		 									  "");
	addChildComponent(filenameComponent);

	//diskMeter->updateDiskSpace(graph->getRecordNode()->getFreeSpace());
	//diskMeter->repaint();
	//refreshMeters();
	startTimer(10);

	setWantsKeyboardFocus(true);

}

ControlPanel::~ControlPanel()
{
	//deleteAllChildren() -> if this is used, audioEditor will be deleted
	deleteAndZero(playButton);
	deleteAndZero(recordButton);
	deleteAndZero(masterClock);
	deleteAndZero(cpuMeter);
	deleteAndZero(diskMeter);
	deleteAndZero(cpb);
	deleteAndZero(filenameComponent);
	deleteAndZero(newDirectoryButton);
	//audioEditor will delete itself

	graph = 0;
}

void ControlPanel::setRecordState(bool t)
{

	recordButton->setToggleState(t, true);

}

void ControlPanel::updateChildComponents()
{

	filenameComponent->addListener(getProcessorGraph()->getRecordNode());

}

void ControlPanel::createPaths()
{
	int w = 150;
	int h1 = 32;
	int h2 = 64;
	int indent = 5;

	p1.clear();
	p1.startNewSubPath(0, h1);
	p1.lineTo(w, h1);
	p1.lineTo(w + indent, h1 + indent);
	p1.lineTo(w + indent, h2 - indent);
	p1.lineTo(w + indent*2, h2);
	p1.lineTo(0, h2);
	p1.closeSubPath();

	p2.clear();
	p2.startNewSubPath(getWidth(), h2-indent);
	p2.lineTo(getWidth(), h2);
	p2.lineTo(getWidth()-indent, h2);
	p2.closeSubPath();

}

void ControlPanel::paint(Graphics& g)
{
	g.setColour(Colour(58,58,58));
	g.fillRect(0,0,getWidth(),getHeight());

	if (open)
	{
		g.setColour(Colours::black);
		g.fillPath(p1);
		g.fillPath(p2);
	}
	
}

void ControlPanel::resized()
{
	int w = getWidth();
	int h = 32; //getHeight();

	if (playButton != 0)
		playButton->setBounds(w-h*9,5,h-5,h-10);
	
	if (recordButton != 0)
		recordButton->setBounds(w-h*8,5,h-5,h-10);

	if (masterClock != 0)
		masterClock->setBounds(w-h*6-15,0,h*6-15,h);
	
	if (cpuMeter != 0)
		cpuMeter->setBounds(8,h/4,h*3,h/2);

	if (diskMeter != 0)
		diskMeter->setBounds(16+h*3,h/4,h*3,h/2);

	if (audioEditor != 0)
		audioEditor->setBounds(h*7,5,h*8,h-10);

	if (cpb != 0)
		cpb->setBounds(w-28,5,h-10,h-10);

	createPaths();

	if (open)
	{
		filenameComponent->setBounds(200, h+5, w-250, h-10);
		filenameComponent->setVisible(true);

		newDirectoryButton->setBounds(165, h+5, h-10, h-10);
		newDirectoryButton->setVisible(true);

	} else {
		filenameComponent->setVisible(false);
		newDirectoryButton->setVisible(false);
	}

	repaint();
}

void ControlPanel::openState(bool os)
{
	open = os;

	getUIComponent()->childComponentChanged();
}

void ControlPanel::buttonClicked(Button* button) 

{
	if (button == recordButton)
	{
		std::cout << "Record button pressed." << std::endl;
		if (recordButton->getToggleState())
		{

			playButton->setToggleState(true,false);
			//graph->getRecordNode()->setParameter(1,10.0f);
			masterClock->startRecording(); // turn on recording


		} else {
			graph->getRecordNode()->setParameter(0,10.0f); // turn off recording
			masterClock->stopRecording();
			newDirectoryButton->setEnabledState(true);
		}

	} else if (button == playButton) {
		std::cout << "Play button pressed." << std::endl;
		if (!playButton->getToggleState())
		{
			if (recordButton->getToggleState())
			{
				recordButton->setToggleState(false,false);
				newDirectoryButton->setEnabledState(true);
			}
			
		}

	} else if (button == newDirectoryButton && newDirectoryButton->getEnabledState())
	{
		getProcessorGraph()->getRecordNode()->createNewDirectory();
		newDirectoryButton->setEnabledState(false);
		masterClock->resetRecordTime();
		return;

	}

	if (playButton->getToggleState())
	{

		if (!audio->callbacksAreActive()) {
			
			if (graph->enableProcessors()) 
			{
				if (recordButton->getToggleState())
					graph->getRecordNode()->setParameter(1,10.0f);
				
				stopTimer();
				startTimer(250); // refresh every 250 ms
				audio->beginCallbacks();
				masterClock->start();
			}
			
		} else {

			if (recordButton->getToggleState())
				graph->getRecordNode()->setParameter(1,10.0f);

		}

	} else {

		if (audio->callbacksAreActive()) {
			audio->endCallbacks();
			graph->disableProcessors();
			refreshMeters();
			masterClock->stop();
			stopTimer();
			startTimer(10000); // back to refresh every 10 seconds

		}

	}

}

void ControlPanel::disableCallbacks()
{

	std::cout << "Control panel received signal to disable callbacks." << std::endl;

	if (audio->callbacksAreActive())
	{
		std::cout << "Stopping audio." << std::endl;
		audio->endCallbacks();
		std::cout << "Disabling processors." << std::endl;
		graph->disableProcessors();
		std::cout << "Updating control panel." << std::endl;
		refreshMeters();
		stopTimer();
		startTimer(10000); // back to refresh every 10 seconds
		
	}

	playButton->setToggleState(false,false);
	recordButton->setToggleState(false,false);
	masterClock->stopRecording();
	masterClock->stop();


}

// void ControlPanel::actionListenerCallback(const String & msg)
// {
// 	//std::cout << "Message Received." << std::endl;
// 	if (playButton->getToggleState()) {
// 		cpuMeter->updateCPU(audio->deviceManager.getCpuUsage());
// 	}

// 	cpuMeter->repaint();

// 	diskMeter->updateDiskSpace(graph->getRecordNode()->getFreeSpace());
// 	diskMeter->repaint();
	
	
// }

void ControlPanel::timerCallback()
{
	//std::cout << "Message Received." << std::endl;
	
	refreshMeters();
	
}

void ControlPanel::refreshMeters()
{
	if (playButton->getToggleState()) {
		cpuMeter->updateCPU(audio->deviceManager.getCpuUsage());
	} else {
		cpuMeter->updateCPU(0.0f);
	}

	cpuMeter->repaint();

	masterClock->repaint();

	diskMeter->updateDiskSpace(graph->getRecordNode()->getFreeSpace());
	diskMeter->repaint();

	if (initialize)
	{
		stopTimer();
		startTimer(5000); // check for disk updates every 5 seconds
		initialize = false;
	}
}

bool ControlPanel::keyPressed(const KeyPress& key)
{
	 std::cout << "Control panel received" << key.getKeyCode() << std::endl;

	 return false;

}

void ControlPanel::toggleState()
{
	open = !open;

	cpb->toggleState();
	getUIComponent()->childComponentChanged();
}
