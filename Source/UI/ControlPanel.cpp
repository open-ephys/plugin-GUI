/*
  ==============================================================================

    ControlPanel.cpp
    Created: 1 May 2011 2:57:48pm
    Author:  jsiegle

  ==============================================================================
*/

#include "ControlPanel.h"
#include <stdio.h>
#include <math.h>

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
	g.fillRect(0.0f,0.0f,getWidth()*diskFree,float(getHeight()));

	g.setColour(Colours::black);
	g.drawRect(0,0,getWidth(),getHeight(),1);

	g.setFont(font);
	g.drawSingleLineText("DF",75,12);
	
}

Clock::Clock() : isRunning(false), isRecording(false)
{
	const unsigned char* buffer = reinterpret_cast<const unsigned char*>(BinaryData::cpmono_light_otf);
	size_t bufferSize = BinaryData::cpmono_light_otfSize;

	font = new FTPixmapFont(buffer, bufferSize);

	totalTime = 0;
	totalRecordTime = 0;
}

Clock::~Clock()
{
}

void Clock::newOpenGLContextCreated()
{
	glMatrixMode (GL_PROJECTION);

	glLoadIdentity();
	glOrtho (0, 1, 1, 0, 0, 1);
	glMatrixMode (GL_MODELVIEW);
	
	glEnable(GL_TEXTURE_2D);

	glClearColor(0.23f, 0.23f, 0.23f, 1.0f); 

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
		m = floor(totalRecordTime/60000);
		s = floor((totalRecordTime - m*60000)/1000);

	} else {
		if (isRunning)
			glColor4f(1.0, 1.0, 0.0, 1.0);
		else
			glColor4f(1.0, 1.0, 1.0, 1.0);
		m = floor(totalTime/60000);
		s = floor((totalTime - m*60000)/1000);
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

	font->FaceSize(23);
	font->Render(timeString);


} 

void Clock::start()
{
	if (!isRunning)
	{
		isRunning = true;
		lastTime = Time::currentTimeMillis();
	}
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
	g.setColour(Colour(58,58,58));
	g.fillRect(0,0,getWidth(),getHeight());

	//g.setFont(font);
	//g.setColour(Colours::white);
	//g.drawText("CONTROL PANEL",getWidth()/2,0,getWidth(),getHeight(),Justification::left,true);
}

void ControlPanel::resized()
{
	int w = getWidth();
	int h = getHeight();

	if (playButton != 0)
		playButton->setBounds(w-h*9,5,h-5,h-10);
	
	if (recordButton != 0)
		recordButton->setBounds(w-h*8,5,h-5,h-10);

	if (masterClock != 0)
		masterClock->setBounds(w-h*6,0,h*6,h);
	
	if (cpuMeter != 0)
		cpuMeter->setBounds(8,h/4,h*3,h/2);

	if (diskMeter != 0)
		diskMeter->setBounds(16+h*3,h/4,h*3,h/2);

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
			graph->getRecordNode()->setParameter(1,10.0f);
			masterClock->startRecording(); // turn on recording

		} else {
			graph->getRecordNode()->setParameter(0,10.0f); // turn off recording
			masterClock->stopRecording();
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
			{
				audio->beginCallbacks();
				masterClock->start();
			}
			
		} else {
			playButton->setToggleState(false, false);
		}

	} else {

		if (audio->callbacksAreActive()) {
			audio->endCallbacks();
			graph->disableProcessors();
			cpuMeter->updateCPU(0.0f);
			masterClock->stop();
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
		cpuMeter->updateCPU(0.0f);
		playButton->setToggleState(false,false);
		recordButton->setToggleState(false,false);
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

	masterClock->repaint();

	diskMeter->updateDiskSpace(graph->getRecordNode()->getFreeSpace());
	diskMeter->repaint();
	
	
}