/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

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
#include "../AccessClass.h"
#include "../Processors/RecordNode/RecordEngine.h"
#include "../Processors/PluginManager/PluginManager.h"


const int SIZE_AUDIO_EDITOR_MAX_WIDTH = 500;
//const int SIZE_AUDIO_EDITOR_MIN_WIDTH = 250;


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
    clockFont.setHorizontalScale(0.95f);

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
    }
    else
    {
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
    : graph(graph_), audio(audio_), initialize(true), open(false), lastEngineIndex(-1)
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

    recordSelector = new ComboBox();
    recordSelector->addListener(this);
    
    addChildComponent(recordSelector);

    recordOptionsButton = new UtilityButton("R",Font("Small Text", 15, Font::plain));
    recordOptionsButton->setEnabledState(true);
    recordOptionsButton->addListener(this);
    recordOptionsButton->setTooltip("Configure options for selected record engine");
    addChildComponent(recordOptionsButton);

    newDirectoryButton = new UtilityButton("+", Font("Small Text", 15, Font::plain));
    newDirectoryButton->setEnabledState(false);
    newDirectoryButton->addListener(this);
    newDirectoryButton->setTooltip("Start a new data directory");
    addChildComponent(newDirectoryButton);


    const File dataDirectory = CoreServices::getDefaultUserSaveDirectory();

    filenameComponent = new FilenameComponent("folder selector",
                                              dataDirectory.getFullPathName(),
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
    dateText->setColour(Label::textColourId, Colours::grey);
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

    recordButton->setToggleState(t, sendNotification);

}

bool ControlPanel::getRecordingState()
{
	return recordButton->getToggleState();

}

void ControlPanel::setRecordingDirectory(String path)
{
    File newFile(path);
    filenameComponent->setCurrentFile(newFile, true, sendNotificationSync);

    for (auto* node : graph->getRecordNodes())
    {
        node->newDirectoryNeeded = true;
    }
    masterClock->resetRecordTime();
}

File ControlPanel::getRecordingDirectory()
{
    return filenameComponent->getCurrentFile();
}

bool ControlPanel::getAcquisitionState()
{
	return playButton->getToggleState();
}

void ControlPanel::setAcquisitionState(bool state)
{
	playButton->setToggleState(state, sendNotification);
}


void ControlPanel::updateChildComponents()
{
    /*
    filenameComponent->addListener(AccessClass::getProcessorGraph()->getRecordNode());
    AccessClass::getProcessorGraph()->getRecordNode()->filenameComponentChanged(filenameComponent);
    */
	updateRecordEngineList();

}

void ControlPanel::updateRecordEngineList()
{


	int selectedEngine = recordSelector->getSelectedId();
	recordSelector->clear(dontSendNotification);
	recordEngines.clear();
	int id = 1;

    LOGD("Num built in engines: ", RecordEngineManager::getNumOfBuiltInEngines());
	for (int i = 0; i < RecordEngineManager::getNumOfBuiltInEngines(); i++)
	{
		RecordEngineManager* rem = RecordEngineManager::createBuiltInEngineManager(i);
		recordSelector->addItem(rem->getName(), id++);
        LOGD("Adding engine: ", rem->getName());
		recordEngines.add(rem);
	}
    LOGD("Num plugin engines: ", AccessClass::getPluginManager()->getNumRecordEngines());
	for (int i = 0; i < AccessClass::getPluginManager()->getNumRecordEngines(); i++)
	{
		Plugin::RecordEngineInfo info;
		info = AccessClass::getPluginManager()->getRecordEngineInfo(i);
		recordSelector->addItem(info.name, id++);
        LOGD("Adding engine: ", info.name);
		recordEngines.add(info.creator());
	}

	if (selectedEngine < 1)
		recordSelector->setSelectedId(1, sendNotification);
	else
		recordSelector->setSelectedId(selectedEngine, sendNotification);
    
}

std::vector<RecordEngineManager*> ControlPanel::getAvailableRecordEngines()
{
    std::vector<RecordEngineManager*> engines;

    for (auto engine : recordEngines)
    {
        engines.push_back(engine);
    }

    return engines;
}

String ControlPanel::getSelectedRecordEngineId()
{
	return recordEngines[recordSelector->getSelectedId() - 1]->getID();
}

bool ControlPanel::setSelectedRecordEngineId(String id)
{
	if (getAcquisitionState())
	{
		return false;
	}

	int nEngines = recordEngines.size();
	for (int i = 0; i < nEngines; ++i)
	{
		if (recordEngines[i]->getID() == id)
		{
			recordSelector->setSelectedId(i + 1, sendNotificationSync);
			return true;
		}
	}
	return false;
}

void ControlPanel::createPaths()
{
    /*  int w = getWidth() - 325;
    if (w > 150)
    w = 150;*/

    int w = getWidth() - 435;
    if (w > 22)
        w = 22;

    int h1 = getHeight()-32;
    int h2 = getHeight();
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
    g.setColour (backgroundColour);
    g.fillRect (0, 0, getWidth(), getHeight());

    if (open)
    {
        createPaths();
        g.setColour(Colours::black);
        g.fillPath(p1);
        g.fillPath(p2);
    }
}

void ControlPanel::resized()
{
    const int w = getWidth();
    const int h = 32; //getHeight();

    // We have 3 possible layout schemes:
    // when there are 1, 2 or 3 rows within which our elements are placed.
    const int twoRowsWidth   = 750;
    const int threeRowsWidth = 570;
    int offset1 = twoRowsWidth - getWidth();
    if (offset1 > h)
        offset1 = h;

    int offset2 = threeRowsWidth - getWidth();
    if (offset2 > h)
        offset2 = h;

    const int currentNumRows = (w < twoRowsWidth && w >= threeRowsWidth - 23)
                                ? 2
                                : (w < threeRowsWidth - 23)
                                    ? 3 : 1;

    // Set positions for CPU and Disk meter components
    // ====================================================================
    int meterComponentsY            = h / 4;
    int meterComponentsWidth        = h * 3;
    const int meterComponentsHeight = h / 2;
    const int meterComponentsMargin = 8;
    switch (currentNumRows)
    {
        case 2:
            meterComponentsY += offset1;
            //meterComponentsWidth = w / 2 - meterComponentsMargin * 2 - 12;
            break;

        case 3:
            meterComponentsY += offset1 + offset2;
            //meterComponentsWidth = w / 2 - meterComponentsMargin * 2 - 12;
            break;

        default:
            break;
    }

    juce::Rectangle<int> meterBounds (meterComponentsMargin, meterComponentsY, meterComponentsWidth, meterComponentsHeight);
    cpuMeter->setBounds  (meterBounds);
    diskMeter->setBounds (meterBounds.translated (meterComponentsWidth + meterComponentsMargin, 0));
    // ====================================================================

    // Set positions for controls and clock
    // ====================================================================
    const int controlButtonWidth    = h - 5;
    const int controlButtonHeight   = h - 10;
    const int masterClockWidth      = h * 6 - 10;
    const int controlsMargin        = 10;
    const int totalControlsWidth = controlButtonWidth * 2 + controlsMargin + masterClockWidth;
    if (currentNumRows != 3)
    {
        playButton->setBounds   (w - h * 8, 5, controlButtonWidth, controlButtonHeight);
        recordButton->setBounds (w - h * 7, 5, controlButtonWidth, controlButtonHeight);
        masterClock->setBounds  (w - masterClockWidth, 0, masterClockWidth,  h);
    }
    else
    {
        const int startX = (w - totalControlsWidth) / 2;
        playButton->setBounds   (startX,     5, controlButtonWidth, controlButtonHeight);
        recordButton->setBounds (startX + h, 5, controlButtonWidth, controlButtonHeight);
        masterClock->setBounds  (startX + h * 2 + controlsMargin * 2, 0, masterClockWidth, h);
    }
    // ====================================================================


    if (audioEditor) //if (audioEditor)
    {
        const bool isThereElementOnLeft = diskMeter->getBounds().getY() <= h;
        const bool isSecondRowAvailable = diskMeter->getBounds().getY() >= 2 * h;
        const int leftElementWidth  = diskMeter->getBounds().getRight();
        const int rightElementWidth = w - playButton->getBounds().getX();

        int maxAvailableWidthForEditor = w;
        if (isThereElementOnLeft)
            maxAvailableWidthForEditor -= leftElementWidth + rightElementWidth;
        else if (! isSecondRowAvailable)
            maxAvailableWidthForEditor -= rightElementWidth;

        const bool isEnoughSpaceForFullSize = maxAvailableWidthForEditor >= SIZE_AUDIO_EDITOR_MAX_WIDTH;

        const int rowIndex    = (isSecondRowAvailable) ? 1 : 0;
        const int editorWidth = isEnoughSpaceForFullSize
                                 ? SIZE_AUDIO_EDITOR_MAX_WIDTH
                                 : maxAvailableWidthForEditor * 0.95;
        const int editorX     = (rowIndex != 0)
                                    ? (w - editorWidth) / 2
                                    : isThereElementOnLeft
                                        ? leftElementWidth + (maxAvailableWidthForEditor - editorWidth) / 2
                                        : (maxAvailableWidthForEditor - editorWidth) / 2;
        const int editorY     = (rowIndex == 0 ) ? 0 : offset1;

        audioEditor->setBounds (editorX, editorY, editorWidth, h);
    }


    if (open)
        cpb->setBounds (w - 28, getHeight() - 5 - h * 2 + 10, h - 10, h - 10);
    else
        cpb->setBounds (w - 28, getHeight() - 5 - h + 10, h - 10, h - 10);

    createPaths();

    if (open)
    {
        int topBound = getHeight() - h + 10 - 5;

        recordSelector->setBounds ( (w - 435) > 40 ? 35 : w - 450, topBound, 100, h - 10);
        recordSelector->setVisible (true);

        recordOptionsButton->setBounds ( (w - 435) > 40 ? 140 : w - 350, topBound, h - 10, h - 10);
        recordOptionsButton->setVisible (true);

        filenameComponent->setBounds (165, topBound, w - 500, h - 10);
        filenameComponent->setVisible (true);

        newDirectoryButton->setBounds (w - h + 4, topBound, h - 10, h - 10);
        newDirectoryButton->setVisible (true);

        prependText->setBounds (165 + w - 490, topBound, 50, h - 10);
        prependText->setVisible (true);

        dateText->setBounds (165 + w - 435, topBound, 175, h - 10);
        dateText->setVisible (true);

        appendText->setBounds (165 + w - 255, topBound, 50, h - 10);
        appendText->setVisible (true);

    }
    else
    {
        filenameComponent->setVisible   (false);
        newDirectoryButton->setVisible  (false);
        prependText->setVisible         (false);
        dateText->setVisible            (false);
        appendText->setVisible          (false);
        recordSelector->setVisible      (false);
        recordOptionsButton->setVisible (false);
    }

    repaint();
}

void ControlPanel::openState(bool os)
{
    open = os;

    cpb->setState(os);

    AccessClass::getUIComponent()->childComponentChanged();
}

void ControlPanel::labelTextChanged(Label* label)
{
    for (auto* node : AccessClass::getProcessorGraph()->getRecordNodes())
    {   
        node->newDirectoryNeeded = true;
    }
    newDirectoryButton->setEnabledState(false);
    masterClock->resetRecordTime();

    dateText->setColour(Label::textColourId, Colours::grey);
}

void ControlPanel::startRecording()
{

    masterClock->startRecording(); // turn on recording
    backgroundColour = Colour(255,0,0);
    prependText->setEditable(false);
    appendText->setEditable(false);
    dateText->setColour(Label::textColourId, Colours::black);

    graph->setRecordState(true);

    repaint();
}

void ControlPanel::stopRecording()
{
    graph->setRecordState(false); // turn off recording in processor graph

    masterClock->stopRecording();
    newDirectoryButton->setEnabledState(true);
    backgroundColour = Colour (51, 51, 51);

    prependText->setEditable(true);
    appendText->setEditable(true);

    recordButton->setToggleState(false, dontSendNotification);

    repaint();
}

void ControlPanel::buttonClicked(Button* button)

{
    if (button == newDirectoryButton && newDirectoryButton->getEnabledState())
    {
        for (auto* node : AccessClass::getProcessorGraph()->getRecordNodes())
        {   
            node->newDirectoryNeeded = true;
        }
        newDirectoryButton->setEnabledState(false);
        masterClock->resetRecordTime();

        dateText->setColour(Label::textColourId, Colours::grey);

        return;
    }

    if (button == playButton)
    {
        if (playButton->getToggleState())
        {

            if (graph->enableProcessors()) // start the processor graph
            {
                if (recordEngines[recordSelector->getSelectedId()-1]->isWindowOpen())
                    recordEngines[recordSelector->getSelectedId()-1]->toggleConfigWindow();

                audio->beginCallbacks(); // launches acquisition
                masterClock->start(); // starts the clock
                //audioEditor->disable();

                stopTimer();
                startTimer(250); // refresh every 250 ms

            }
            recordSelector->setEnabled(false); // why is this outside the "if" statement?
            recordOptionsButton->setEnabled(false);
        }
        else
        {

            if (recordButton->getToggleState())
            {
                stopRecording();
            }

            audio->endCallbacks();
            graph->disableProcessors();
            refreshMeters();
            masterClock->stop();
            stopTimer();
            startTimer(60000); // back to refresh every minute
            audioEditor->enable();
            recordSelector->setEnabled(true);
            recordOptionsButton->setEnabled(true);

        }

        return;
    }

    if (button == recordButton)
    {
        if (recordButton->getToggleState())
        {
            
            if (!graph->hasRecordNode())
            {
                CoreServices::sendStatusMessage("Please insert at least one Record Node to start recording!");
                recordButton->setToggleState(false, dontSendNotification);
                return;
            }
            
            if (playButton->getToggleState())
            {
                startRecording();
            }
            else
            {
                if (graph->enableProcessors()) // start the processor graph
                {
                    if (recordEngines[recordSelector->getSelectedId()-1]->isWindowOpen())
                        recordEngines[recordSelector->getSelectedId()-1]->toggleConfigWindow();
					
					startRecording();
                    masterClock->start();
					audio->beginCallbacks();
                    //audioEditor->disable();

                    stopTimer();
                    startTimer(250); // refresh every 250 ms

                    

                    playButton->setToggleState(true, dontSendNotification);
                    recordSelector->setEnabled(false);
                    recordOptionsButton->setEnabled(false);

                }
            }
        }
        else
        {
            stopRecording();
        }
    }

    if (button == recordOptionsButton)
    {
        int id = recordSelector->getSelectedId()-1;
        if (id < 0) return;

        recordEngines[id]->toggleConfigWindow();
    }

}

void ControlPanel::comboBoxChanged(ComboBox* combo)
{

    if (lastEngineIndex >= 0)
    {
        if (recordEngines[lastEngineIndex]->isWindowOpen())
            recordEngines[lastEngineIndex]->toggleConfigWindow();
    }
    ScopedPointer<RecordEngine> re;
    //AccessClass::getProcessorGraph()->getRecordNode()->clearRecordEngines();
    if (combo->getSelectedId() > 0)
    {
        LOGD("Num engines: ", recordEngines.size());
        re = recordEngines[combo->getSelectedId()-1]->instantiateEngine();
    }
    else
    {
        LOGD("Engine ComboBox: Bad ID");
        combo->setSelectedId(1,dontSendNotification);
        re = recordEngines[0]->instantiateEngine();
    }
    //re->setUIComponent(getUIComponent());
    re->registerManager(recordEngines[combo->getSelectedId()-1]);
    //AccessClass::getProcessorGraph()->getRecordNode()->registerRecordEngine(re);

    //graph->getRecordNode()->newDirectoryNeeded = true;
    newDirectoryButton->setEnabledState(false);
    masterClock->resetRecordTime();

    dateText->setColour(Label::textColourId, Colours::grey);
    lastEngineIndex=combo->getSelectedId()-1;
}

void ControlPanel::disableCallbacks()
{

    LOGD("Control panel received signal to disable callbacks.");

    if (audio->callbacksAreActive())
    {
        LOGD("Stopping audio.");
        audio->endCallbacks();
        LOGD("Disabling processors.");
        graph->disableProcessors();
        LOGD("Updating control panel.");
        refreshMeters();
        stopTimer();
        startTimer(60000); // back to refresh every 10 seconds

    }

    playButton->setToggleState(false, dontSendNotification);
    recordButton->setToggleState(false, dontSendNotification);
    recordSelector->setEnabled(true);
    masterClock->stopRecording();
    masterClock->stop();

}

// void ControlPanel::actionListenerCallback(const String & msg)
// {
// 	LOGDD("Message Received");
// 	if (playButton->getToggleState()) {
// 		cpuMeter->updateCPU(audio->deviceManager.getCpuUsage());
// 	}

// 	cpuMeter->repaint();

// 	diskMeter->updateDiskSpace(graph->getRecordNode()->getFreeSpace());
// 	diskMeter->repaint();


// }

void ControlPanel::timerCallback()
{
    LOGDD("Message Received.");
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

    //diskMeter->updateDiskSpace(graph->getRecordNode()->getFreeSpace());
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
    LOGD("Control panel received", key.getKeyCode());

    return false;

}

void ControlPanel::toggleState()
{
    open = !open;

    cpb->toggleState();
    AccessClass::getUIComponent()->childComponentChanged();
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

void ControlPanel::setPrependText(String t)
{
    prependText->setText(t, sendNotificationSync);
}

void ControlPanel::setAppendText(String t)
{
    appendText->setText(t, sendNotificationSync);
}

void ControlPanel::setDateText(String t)
{
    dateText->setText(t, dontSendNotification);
}


void ControlPanel::saveStateToXml(XmlElement* xml)
{

    XmlElement* controlPanelState = xml->createNewChildElement("CONTROLPANEL");
    controlPanelState->setAttribute("isOpen",open);
	controlPanelState->setAttribute("recordPath", filenameComponent->getCurrentFile().getFullPathName());
    controlPanelState->setAttribute("prependText",prependText->getText());
    controlPanelState->setAttribute("appendText",appendText->getText());
    controlPanelState->setAttribute("recordEngine",recordEngines[recordSelector->getSelectedId()-1]->getID());

    audioEditor->saveStateToXml(xml);

    /*
    XmlElement* recordEnginesState = xml->createNewChildElement("RECORDENGINES");
    for (int i=0; i < recordEngines.size(); i++)
    {
        XmlElement* reState = recordEnginesState->createNewChildElement("ENGINE");
        reState->setAttribute("id",recordEngines[i]->getID());
        reState->setAttribute("name",recordEngines[i]->getName());
        recordEngines[i]->saveParametersToXml(reState);
    }
    */

}

void ControlPanel::loadStateFromXml(XmlElement* xml)
{

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("CONTROLPANEL"))
        {
			String recordPath = xmlNode->getStringAttribute("recordPath", String::empty);
			if (!recordPath.isEmpty())
			{
				filenameComponent->setCurrentFile(File(recordPath), true, sendNotificationAsync);
			}
            appendText->setText(xmlNode->getStringAttribute("appendText", ""), dontSendNotification);
            prependText->setText(xmlNode->getStringAttribute("prependText", ""), dontSendNotification);
			String selectedEngine = xmlNode->getStringAttribute("recordEngine");
			for (int i = 0; i < recordEngines.size(); i++)
			{
				if (recordEngines[i]->getID() == selectedEngine)
				{
					recordSelector->setSelectedId(i + 1, sendNotification);
				}
			}

            bool isOpen = xmlNode->getBoolAttribute("isOpen");
            openState(isOpen);

        }
        else if (xmlNode->hasTagName("RECORDENGINES"))
        {
            for (int i = 0; i < recordEngines.size(); i++)
            {
                forEachXmlChildElementWithTagName(*xmlNode,xmlEngine,"ENGINE")
                {
                    if (xmlEngine->getStringAttribute("id") == recordEngines[i]->getID())
                        recordEngines[i]->loadParametersFromXml(xmlEngine);
                }
            }
        }
    }

    audioEditor->loadStateFromXml(xml);

}


StringArray ControlPanel::getRecentlyUsedFilenames()
{
    return filenameComponent->getRecentlyUsedFilenames();
}


void ControlPanel::setRecentlyUsedFilenames(const StringArray& filenames)
{
    filenameComponent->setRecentlyUsedFilenames(filenames);
}