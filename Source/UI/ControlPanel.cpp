/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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
    : DrawableButton("PlayButton", DrawableButton::ImageFitted)
{

    DrawablePath normal, over, down;

    Path p;
    p.addTriangle(0.0f, 0.0f, 0.0f, 20.0f, 18.0f, 10.0f);
    normal.setPath(p);
    normal.setFill(Colours::lightgrey);
    normal.setStrokeThickness(0.0f);

    over.setPath(p);
    over.setFill(Colours::black);
    over.setStrokeFill(Colours::black);
    over.setStrokeThickness(5.0f);

    down.setPath(p);
    down.setFill(Colours::pink);
    down.setStrokeFill(Colours::pink);
    down.setStrokeThickness(5.0f);

    setImages(&normal, &over, &over);
    // setBackgroundColours(Colours::darkgrey, Colours::yellow);
    setClickingTogglesState(true);
    setTooltip("Start/stop acquisition");


}

PlayButton::~PlayButton()
{
}

RecordButton::RecordButton()
    : DrawableButton("RecordButton", DrawableButton::ImageFitted)
{

    DrawablePath normal, over, down;

    Path p;
    p.addEllipse(0.0,0.0,20.0,20.0);
    normal.setPath(p);
    normal.setFill(Colours::lightgrey);
    normal.setStrokeThickness(0.0f);

    over.setPath(p);
    over.setFill(Colours::black);
    over.setStrokeFill(Colours::black);
    over.setStrokeThickness(5.0f);

    setImages(&normal, &over, &over);
    //setBackgroundColours(Colours::darkgrey, Colours::red);
    setClickingTogglesState(true);
    setTooltip("Start/stop writing to disk");
}

RecordButton::~RecordButton()
{
}


CPUMeter::CPUMeter() : Label("CPU Meter","0.0"), cpu(0.0f), lastCpu(0.0f)
{

    font = Font("Small Text", 12, Font::plain);

    // MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    // Typeface::Ptr typeface = new CustomTypeface(mis);
    // font = Font(typeface);
    // font.setHeight(12);
    
    setTooltip("CPU usage");
}

CPUMeter::~CPUMeter()
{
}

void CPUMeter::updateCPU(float usage)
{
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

    font = Font("Small Text", 12, Font::plain);

    // MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    // Typeface::Ptr typeface = new CustomTypeface(mis);
    // font = Font(typeface);
    // font.setHeight(12);
    
    setTooltip("Disk space available");
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

    clockFont = Font("Default Light", 30, Font::plain);

    // MemoryInputStream mis(BinaryData::cpmonolightserialized, BinaryData::cpmonolightserializedSize, false);
    // Typeface::Ptr typeface = new CustomTypeface(mis);
    // clockFont = Font(typeface);
    // clockFont.setHeight(30);

    totalTime = 0;
    totalRecordTime = 0;

}

Clock::~Clock()
{
}


void Clock::paint(Graphics& g)
{
    if (isRecording)
    {
        g.fillAll(Colour(255,0,0));
    } else {
        g.fillAll(Colour(58,58,58));
    }
    
    drawTime(g);
}

void Clock::drawTime(Graphics& g)
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
        g.setColour(Colours::black);
        m = floor(totalRecordTime/60000.0);
        s = floor((totalRecordTime - m*60000.0)/1000.0);

    }
    else
    {

        if (isRunning)
            g.setColour(Colours::yellow);
        else
            g.setColour(Colours::white);

        m = floor(totalTime/60000.0);
        s = floor((totalTime - m*60000.0)/1000.0);
    }

    String timeString = "";

    timeString += m;
    timeString += " min ";
    timeString += s;
    timeString += " s";

    g.setFont(clockFont);
    //g.setFont(30);
    g.drawText(timeString, 0, 0, getWidth(), getHeight(), Justification::left, false);

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

    setTooltip("Show/hide recording options");
}

ControlPanelButton::~ControlPanelButton()
{

}

void ControlPanelButton::paint(Graphics& g)
{
    //g.fillAll(Colour(58,58,58));

    g.setColour(Colours::white);

    Path p;

    float h = getHeight();
    float w = getWidth();

    if (open)
    {
        p.addTriangle(0.5f*w, 0.8f*h,
                      0.2f*w, 0.2f*h,
                      0.8f*w, 0.2f*h);
    }
    else
    {
        p.addTriangle(0.8f*w, 0.8f*h,
                      0.2f*w, 0.5f*h,
                      0.8f*w, 0.2f*h);
    }

    PathStrokeType pst = PathStrokeType(1.0f, PathStrokeType::curved, PathStrokeType::rounded);

    g.strokePath(p, pst);

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

void ControlPanelButton::setState(bool b)
{
    open = b;
    repaint();
}




ControlPanel::ControlPanel(ProcessorGraph* graph_, AudioComponent* audio_)
    : graph(graph_), audio(audio_), initialize(true), open(false)
{

    if (1)
    {

        font = Font("Paragraph", 13, Font::plain);

        // MemoryInputStream mis(BinaryData::misoserialized, BinaryData::misoserializedSize, false);
        // Typeface::Ptr typeface = new CustomTypeface(mis);
        // font = Font(typeface);
        // font.setHeight(15);
    }

    audioEditor = (AudioEditor*) graph->getAudioNode()->createEditor();
    addAndMakeVisible(audioEditor);

    playButton = new PlayButton();
    playButton->addListener(this);
    addAndMakeVisible(playButton);

    recordButton = new RecordButton();
    recordButton->addListener(this);
    addAndMakeVisible(recordButton);

    masterClock = new Clock();
    addAndMakeVisible(masterClock);

    cpuMeter = new CPUMeter();
    addAndMakeVisible(cpuMeter);

    diskMeter = new DiskSpaceMeter();
    addAndMakeVisible(diskMeter);

    cpb = new ControlPanelButton(this);
    addAndMakeVisible(cpb);

    newDirectoryButton = new UtilityButton("+", Font("Small Text", 15, Font::plain));
    newDirectoryButton->setEnabledState(false);
    newDirectoryButton->addListener(this);
    newDirectoryButton->setTooltip("Start a new data directory");
    addChildComponent(newDirectoryButton);


    File executable = File::getSpecialLocation(File::currentExecutableFile);
    
#if defined(__APPLE__)
    const String executableDirectory =
    executable.getParentDirectory().getParentDirectory().getParentDirectory().getParentDirectory().getFullPathName();
#else
    const String executableDirectory = executable.getParentDirectory().getFullPathName();
#endif

    filenameComponent = new FilenameComponent("folder selector",
                                              executableDirectory,
                                              true,
                                              true,
                                              true,
                                              "*",
                                              "",
                                              "");
    addChildComponent(filenameComponent);

    prependText = new Label("Prepend","");
    prependText->setEditable(true);
    prependText->addListener(this);
    prependText->setColour(Label::backgroundColourId, Colours::lightgrey);
    prependText->setTooltip("Prepend to name of data directory");

    addChildComponent(prependText);

    dateText = new Label("Date","YYYY-MM-DD_HH-MM-SS");
    dateText->setColour(Label::backgroundColourId, Colours::lightgrey);
    addChildComponent(dateText);

    appendText = new Label("Append","");
    appendText->setEditable(true);
    appendText->addListener(this);
    appendText->setColour(Label::backgroundColourId, Colours::lightgrey);
    addChildComponent(appendText);
    appendText->setTooltip("Append to name of data directory");

    //diskMeter->updateDiskSpace(graph->getRecordNode()->getFreeSpace());
    //diskMeter->repaint();
    //refreshMeters();
    startTimer(10);

    setWantsKeyboardFocus(true);
    
    backgroundColour = Colour(58,58,58);

}

ControlPanel::~ControlPanel()
{

}

void ControlPanel::setRecordState(bool t)
{

    //MessageManager* mm = MessageManager::getInstance();

    recordButton->setToggleState(t, true);

}

void ControlPanel::updateChildComponents()
{

    filenameComponent->addListener(getProcessorGraph()->getRecordNode());
    getProcessorGraph()->getRecordNode()->filenameComponentChanged(filenameComponent);

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
    g.setColour(backgroundColour);
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
        playButton->setBounds(w-h*10,5,h-5,h-10);

    if (recordButton != 0)
        recordButton->setBounds(w-h*9,5,h-5,h-10);

    if (masterClock != 0)
        masterClock->setBounds(w-h*7-15,0,h*7-15,h);

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
        filenameComponent->setBounds(165, h+5, w-500, h-10);
        filenameComponent->setVisible(true);

        newDirectoryButton->setBounds(w-h+4, h+5, h-10, h-10);
        newDirectoryButton->setVisible(true);

        prependText->setBounds(165+w-490, h+5, 50, h-10);
        prependText->setVisible(true);

        dateText->setBounds(165+w-435, h+5, 175, h-10);
        dateText->setVisible(true);

        appendText->setBounds(165+w-255, h+5, 50, h-10);
        appendText->setVisible(true);

    }
    else
    {
        filenameComponent->setVisible(false);
        newDirectoryButton->setVisible(false);
        prependText->setVisible(false);
        dateText->setVisible(false);
        appendText->setVisible(false);
    }

    repaint();
}

void ControlPanel::openState(bool os)
{
    open = os;

    cpb->setState(os);

    getUIComponent()->childComponentChanged();
}

void ControlPanel::labelTextChanged(Label* label)
{

}

void ControlPanel::startRecording()
{
    playButton->setToggleState(true,false);
    masterClock->startRecording(); // turn on recording
    backgroundColour = Colour(255,0,0);
    repaint();
}

void ControlPanel::stopRecording()
{
    graph->setRecordState(false); // turn off recording in processor graph
    masterClock->stopRecording();
    newDirectoryButton->setEnabledState(true);
    backgroundColour = Colour(58,58,58);
    repaint();
}

void ControlPanel::buttonClicked(Button* button)

{
    
    if (button == recordButton)
    {
        std::cout << "Record button pressed." << std::endl;
        if (recordButton->getToggleState())
        {

            startRecording();

        }
        else
        {
            stopRecording();
        }

        dateText->setColour(Label::textColourId, Colours::black);

    }
    else if (button == playButton)
    {
        std::cout << "Play button pressed." << std::endl;
        if (!playButton->getToggleState())
        {
            if (recordButton->getToggleState())
            {
                recordButton->setToggleState(false,false);
                stopRecording();
                //newDirectoryButton->setEnabledState(true);
            }

        }

    }
    else if (button == newDirectoryButton && newDirectoryButton->getEnabledState())
    {
        graph->getRecordNode()->newDirectoryNeeded = true;
        newDirectoryButton->setEnabledState(false);
        masterClock->resetRecordTime();

        dateText->setColour(Label::textColourId, Colours::grey);

        return;

    }

    if (playButton->getToggleState())
    {

        if (!audio->callbacksAreActive())
        {

            if (graph->enableProcessors())
            {
                
                //std::cout << "Enabling processors from " << getThreadName() << " thread." << std::endl;
                
                if (recordButton->getToggleState())
                    graph->setRecordState(true);

                stopTimer();
                
                audio->beginCallbacks();
                masterClock->start();
                
                startTimer(250); // refresh every 250 ms

            }

        }
        else
        {

            if (recordButton->getToggleState())
            {
                graph->setRecordState(true); //getRecordNode()->setParameter(1,10.0f);
            }

        }

    }
    else
    {

        if (audio->callbacksAreActive())
        {
            
            std::cout << "Control panel requesting to end callbacks." << std::endl;
            
            audio->endCallbacks();
            
            std::cout << "Control panel requesting to disable processors." << std::endl;
            graph->disableProcessors();
            
            refreshMeters();
            masterClock->stop();
            stopTimer();
            startTimer(60000); // back to refresh every minute

        }

    }

    if (playButton->getToggleState())
        audioEditor->disable();
    else
        audioEditor->enable();

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
        startTimer(60000); // back to refresh every 10 seconds

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
    if (playButton->getToggleState())
    {
        cpuMeter->updateCPU(audio->deviceManager.getCpuUsage());
    }
    else
    {
        cpuMeter->updateCPU(0.0f);
    }

    cpuMeter->repaint();

    masterClock->repaint();

    diskMeter->updateDiskSpace(graph->getRecordNode()->getFreeSpace());
    diskMeter->repaint();

    if (initialize)
    {
        stopTimer();
        startTimer(60000); // check for disk updates every minute
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

String ControlPanel::getTextToAppend()
{
    String t = appendText->getText();

    if (t.length() > 0)
    {
        return "_" + t;
    }
    else
    {
        return t;
    }
}

String ControlPanel::getTextToPrepend()
{
    String t = prependText->getText();

    if (t.length() > 0)
    {
        return t + "_";
    }
    else
    {
        return t;
    }
}

void ControlPanel::setDateText(String t)
{
    dateText->setText(t, dontSendNotification);
}


void ControlPanel::saveStateToXml(XmlElement* xml)
{

    XmlElement* controlPanelState = xml->createNewChildElement("CONTROLPANEL");
    controlPanelState->setAttribute("isOpen",open);
    controlPanelState->setAttribute("prependText",prependText->getText());
    controlPanelState->setAttribute("appendText",appendText->getText());

}

void ControlPanel::loadStateFromXml(XmlElement* xml)
{

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("CONTROLPANEL"))
        {

            appendText->setText(xmlNode->getStringAttribute("appendText", ""), dontSendNotification);
            prependText->setText(xmlNode->getStringAttribute("prependText", ""), dontSendNotification);

            bool isOpen = xmlNode->getBoolAttribute("isOpen");
            openState(isOpen);

        }
    }
    
    getProcessorGraph()->getAudioNode()->updateBufferSize();

}