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
#include "FilenameConfigWindow.h"


const int SIZE_AUDIO_EDITOR_MAX_WIDTH = 500;
//const int SIZE_AUDIO_EDITOR_MIN_WIDTH = 250;

#define defaultButtonColour Colour(180,180,180)


FilenameEditorButton::FilenameEditorButton()
    : TextButton("Filename Editor")
{
    setTooltip("Edit the recording filename");
}

PlayButton::PlayButton()
    : DrawableButton("Play Button", DrawableButton::ImageFitted)
{

    DrawablePath normal, over, down;

    Path p;
    p.addTriangle(0.0f, 0.0f, 0.0f, 20.0f, 18.0f, 10.0f);
    normal.setPath(p);
    normal.setFill(defaultButtonColour);
    normal.setStrokeThickness(0.0f);

    over.setPath(p);
    over.setFill(Colours::black);
    over.setStrokeThickness(2.0f);
    over.setStrokeFill(Colours::black);

    down.setPath(p);
    down.setFill(Colours::pink);
    down.setStrokeFill(Colours::pink);
    down.setStrokeThickness(5.0f);

    setImages(&normal, &over, &over);
    setColour(DrawableButton::backgroundColourId, Colours::darkgrey.withAlpha(0.0f));
    setColour(DrawableButton::backgroundOnColourId, Colours::darkgrey.withAlpha(0.0f));
    setClickingTogglesState(true);
    setTooltip("Start/stop acquisition");
}

RecordButton::RecordButton()
    : DrawableButton("Record Button", DrawableButton::ImageFitted)
{

    DrawablePath normal, over, down;

    Path p;
    p.addEllipse(0.0,0.0,20.0,20.0);
    normal.setPath(p);
    normal.setFill(defaultButtonColour);
    normal.setStrokeThickness(0.0f);

    over.setPath(p);
    over.setFill(Colours::black);
    over.setStrokeFill(Colours::black);
    over.setStrokeThickness(5.0f);

    setImages(&normal, &over, &over);
    setColour(DrawableButton::backgroundColourId, Colours::darkgrey.withAlpha(0.0f));
    setColour(DrawableButton::backgroundOnColourId, Colours::darkgrey.withAlpha(0.0f));
    setClickingTogglesState(true);
    setTooltip("Start/stop writing to disk");
}


CPUMeter::CPUMeter() : Label("CPU Meter","0.0"), cpu(0.0f)
{

    font = Font("Silkscreen", "Regular", 12);
    
    setTooltip("CPU usage");
}

void CPUMeter::updateCPU(float usage)
{
    cpu = usage;

    repaint();
}

void CPUMeter::paint(Graphics& g)
{
    g.fillAll(Colours::grey);

    g.setColour(Colours::yellow);
    g.fillRect(0.0f, 0.0f, getWidth() * cpu, float(getHeight()));

    g.setColour(Colours::black);
    g.drawRect(0,0,getWidth(),getHeight(),1);

    g.setFont(font);
    g.drawSingleLineText("CPU",65,12);

}


DiskSpaceMeter::DiskSpaceMeter()

{

    font = Font("Silkscreen", "Regular", 12);
    
    setTooltip("Disk space available");
}

void DiskSpaceMeter::updateDiskSpace(float percent)
{
    diskFree = percent;

    repaint();
}

void DiskSpaceMeter::paint(Graphics& g)
{

    g.fillAll(Colours::grey);

    g.setColour(Colours::lightgrey);
    if (diskFree > 0)
    {
        if (diskFree > 1.0)
            diskFree = 1.0; 
        g.fillRect(0.0f, 0.0f, getWidth() * diskFree, float(getHeight()));
    }

    g.setColour(Colours::black);
    g.drawRect(0, 0, getWidth(), getHeight(), 1);

    g.setFont(font);
    g.drawSingleLineText("DF",75,12);

}

Clock::Clock() : isRunning(false),
                 isRecording(false),
                 mode(DEFAULT)
{

    clockFont = Font("CP Mono", "Light", 30);
    clockFont.setHorizontalScale(0.95f);

    totalTime = 0;
    totalRecordTime = 0;

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
    int h;

    if (isRecording)
    {
        g.setColour(Colours::black);
		h = floor(totalRecordTime / 3600000.0f);
        m = floor(totalRecordTime / 60000.0);
        s = floor((totalRecordTime - m * 60000.0) / 1000.0);

    }
    else
    {

        if (isRunning)
            g.setColour(Colours::yellow);
        else
            g.setColour(Colours::white);

        h = floor(totalTime / 3600000.0f);
        m = floor(totalTime / 60000.0);
        s = floor((totalTime - m * 60000.0) / 1000.0);
    }

    String timeString = "";

    if (mode == DEFAULT)
    {
        timeString += m;
        timeString += " min ";
        timeString += s;
        timeString += " s";
    }
    else {
        if (h < 10) timeString += "0";
        timeString += h;
        timeString += ":";
        
        if (m < 10) timeString += "0";
        timeString += m;
        timeString += ":";

        if (s < 10) timeString += "0";
        timeString += s;
    }
    

    g.setFont(clockFont);
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

void Clock::setMode(Mode m)
{
	mode = m;

    repaint();
}

void Clock::mouseDown(const MouseEvent& e)
{
	if (e.mods.isRightButtonDown())
	{
		PopupMenu m;
        
        m.addItem(1, "Clock mode", false);
        m.addSeparator();
		m.addItem(2, "Default", true, mode == DEFAULT);
		m.addItem(3, "HH:MM:SS", true, mode == HHMMSS);

		int result = m.show();

		if (result == 2)
		{
			setMode(DEFAULT);
		}
		else if (result == 3)
		{
			setMode(HHMMSS);
		}
	}
}

ControlPanelButton::ControlPanelButton(ControlPanel* cp_)
    : cp(cp_),
      open(false)
{
    openPath.addTriangle(10.f, 14.398f,
                         4.f, 4.f,
                         16.f, 4.f);
    
    closedPath = Path(openPath);
    openPath.applyTransform(AffineTransform::translation(4,4));
    closedPath.applyTransform(AffineTransform::rotation(MathConstants<float>::pi/2, 10.f, 10.f));

    setTooltip("Show/hide recording options");
}

void ControlPanelButton::paint(Graphics& g)
{
    
    g.setColour(defaultButtonColour);

    PathStrokeType pst = PathStrokeType(1.0f, PathStrokeType::curved, PathStrokeType::rounded);

    if (open)
        g.strokePath(openPath, pst);
    else
        g.strokePath(closedPath, pst);
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
    : graph(graph_), audio(audio_), initialize(true), open(false), lastEngineIndex(-1), forceRecording(false)
{

    font = Font("Miso", "Regular", 13);

    audioEditor = (AudioEditor*) graph->getAudioNode()->createEditor();
    addAndMakeVisible(audioEditor);

    playButton = std::make_unique<PlayButton>();
    playButton->addListener(this);
    addAndMakeVisible(playButton.get());

    recordButton = std::make_unique<RecordButton>();
    recordButton->addListener(this);
    addAndMakeVisible(recordButton.get());

    clock = std::make_unique<Clock>();
    addAndMakeVisible(clock.get());

    cpuMeter = std::make_unique<CPUMeter>();
    addAndMakeVisible(cpuMeter.get());

    diskMeter = std::make_unique<DiskSpaceMeter>();
    addAndMakeVisible(diskMeter.get());

    cpb = std::make_unique<ControlPanelButton>(this);
    addAndMakeVisible(cpb.get());

    recordSelector = std::make_unique<ComboBox>("Control Panel Record Engine Selector");
    recordSelector->addListener(this);
    addChildComponent(recordSelector.get());

    recordOptionsButton = std::make_unique<UtilityButton>("R", Font("Silkscreen", "Regular", 15));
    recordOptionsButton->setEnabledState(true);
    recordOptionsButton->addListener(this);
    recordOptionsButton->setTooltip("Configure options for selected record engine");
    addChildComponent(recordOptionsButton.get());

    newDirectoryButton = std::make_unique<UtilityButton>("+", Font("Silkscreen", "Regular", 15));
    newDirectoryButton->setEnabledState(false);
    newDirectoryButton->addListener(this);
    newDirectoryButton->setTooltip("Start a new data directory");
    addChildComponent(newDirectoryButton.get());

    const File dataDirectory = CoreServices::getDefaultUserSaveDirectory();

    filenameComponent = std::make_unique<FilenameComponent>("folder selector",
                                              dataDirectory.getFullPathName(),
                                              true,
                                              true,
                                              true,
                                              "*",
                                              "",
                                              "");
    addChildComponent(filenameComponent.get());

    filenameFields.add(std::make_shared<FilenameFieldComponent>(
        FilenameFieldComponent::Type::PREPEND, FilenameFieldComponent::State::NONE, ""));
    filenameFields.add(std::make_shared<FilenameFieldComponent>(
        FilenameFieldComponent::Type::MAIN, FilenameFieldComponent::State::AUTO,"MM-DD-YYYY_HH-MM-SS"));
    filenameFields.add(std::make_shared<FilenameFieldComponent>(
        FilenameFieldComponent::Type::APPEND, FilenameFieldComponent::State::NONE,""));

    filenameText = std::make_unique<FilenameEditorButton>();
    generateFilenameFromFields(true);
    filenameText->addListener(this);
    addAndMakeVisible(filenameText.get());

    filenameConfigWindow = std::make_unique<FilenameConfigWindow>(filenameFields);

    refreshMeters();

    startTimer(60000); // update disk space every minute

    setWantsKeyboardFocus(true);

    backgroundColour = Colour(58,58,58);

}

ControlPanel::~ControlPanel()
{

}

void ControlPanel::setRecordingState(bool t, bool force)
{

    forceRecording = force;
    
    recordButton->setToggleState(t, sendNotification);

}

bool ControlPanel::getRecordingState()
{
	return recordButton->getToggleState();

}

void ControlPanel::setRecordingParentDirectory(String path)
{
    File newFile(path);
    filenameComponent->setCurrentFile(newFile, true, sendNotificationSync);
}

File ControlPanel::getRecordingParentDirectory()
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

void ControlPanel::startAcquisition(bool recordingShouldAlsoStart)
{
    
    if (!audio->checkForDevice())
    {
        String titleMessage = String("No audio device found");
        String contentMessage = String("An active audio device is required to process data. ") + 
                                String("Try restarting the GUI to regain control of the system audio.");
        AlertWindow::showMessageBox(AlertWindow::InfoIcon,
                                    titleMessage,
                                    contentMessage);
        
        playButton->setToggleState(false, dontSendNotification);
        
        return;
    }
    
    if (graph->isReady()) // check that all processors are enabled
    {
        if (recordEngines[recordSelector->getSelectedId() - 1]->isWindowOpen())
            recordEngines[recordSelector->getSelectedId() - 1]->toggleConfigWindow();

        graph->updateConnections();
        
        if (audio->beginCallbacks()) // starts acquisition callbacks
        {
            if (recordingShouldAlsoStart)
            {
                startRecording();
                playButton->setToggleState(true, dontSendNotification);
            }

            playButton->getNormalImage()->replaceColour(defaultButtonColour, Colours::yellow);
            
            clock->start(); // starts the clock
            audioEditor->disable();

            stopTimer();
            startTimer(250); // refresh every 250 ms

            recordSelector->setEnabled(false); // why is this outside the "if" statement?
            recordOptionsButton->setEnabled(false);
            
            graph->startAcquisition(); // start data flow
        }
    }
}

void ControlPanel::stopAcquisition()
{
    if (recordButton->getToggleState())
    {
        stopRecording();
    }

    graph->stopAcquisition();

    audio->endCallbacks();
    
    playButton->getNormalImage()->replaceColour(Colours::yellow, defaultButtonColour);

    refreshMeters();

    clock->stop();
    audioEditor->enable();

    stopTimer();
    startTimer(60000); // back to refresh every minute
    
    recordSelector->setEnabled(true);
    recordOptionsButton->setEnabled(true);
}

void ControlPanel::updateRecordEngineList()
{

	int selectedEngine = recordSelector->getSelectedId();
	recordSelector->clear(dontSendNotification);
	recordEngines.clear();
	int id = 1;

    LOGD("Built-in Record Engine count: ", RecordEngineManager::getNumOfBuiltInEngines());

	for (int i = 0; i < RecordEngineManager::getNumOfBuiltInEngines(); i++)
	{
		RecordEngineManager* rem = RecordEngineManager::createBuiltInEngineManager(i);
		recordSelector->addItem(rem->getName(), id++);
        LOGD("Adding Record Engine: ", rem->getName());
		recordEngines.add(rem);
	}
    LOGD("Plugin Record Engine count: ", AccessClass::getPluginManager()->getNumRecordEngines());
	for (int i = 0; i < AccessClass::getPluginManager()->getNumRecordEngines(); i++)
	{
		Plugin::RecordEngineInfo info;
		info = AccessClass::getPluginManager()->getRecordEngineInfo(i);
		recordSelector->addItem(info.name, id++);
        LOGD("Adding Record Engine: ", info.name);
		recordEngines.add(info.creator());
	}

    if (selectedEngine < 1)
    {
        setSelectedRecordEngine(0);
        recordSelector->setSelectedId(1, dontSendNotification);
    }
		
    else
    {
        setSelectedRecordEngine(selectedEngine - 1);
        recordSelector->setSelectedId(selectedEngine, dontSendNotification);
    }
		
    
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
    const int clockWidth      = h * 6 - 10;
    const int controlsMargin        = 10;
    const int totalControlsWidth = controlButtonWidth * 2 + controlsMargin + clockWidth;
    if (currentNumRows != 3)
    {
        playButton->setBounds   (w - h * 8, 5, controlButtonWidth, controlButtonHeight);
        recordButton->setBounds (w - h * 7, 5, controlButtonWidth, controlButtonHeight);
        clock->setBounds  (w - clockWidth, 0, clockWidth,  h);
    }
    else
    {
        const int startX = (w - totalControlsWidth) / 2;
        playButton->setBounds   (startX,     5, controlButtonWidth, controlButtonHeight);
        recordButton->setBounds (startX + h, 5, controlButtonWidth, controlButtonHeight);
        clock->setBounds  (startX + h * 2 + controlsMargin * 2, 0, clockWidth, h);
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

        recordSelector->setBounds ( (w - 435) > 40 ? 35 : w - 450, topBound, 125, h - 10);
        recordSelector->setVisible (true);

        recordOptionsButton->setBounds ( (w - 435) > 40 ? 140 : w - 350, topBound, h - 10, h - 10);
        recordOptionsButton->setVisible (false);

        filenameComponent->setBounds (165, topBound, w - 500, h - 10);
        filenameComponent->setVisible (true);

        newDirectoryButton->setBounds (w - h + 4, topBound, h - 10, h - 10);
        newDirectoryButton->setVisible (true);

        filenameText->setBounds (165 + w - 490, topBound, 280, h - 10);
        filenameText->setVisible (true);

    }
    else
    {
        filenameComponent->setVisible   (false);
        newDirectoryButton->setVisible  (false);
        filenameText->setVisible            (false);
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
    clock->resetRecordTime();

    filenameText->setColour(Label::textColourId, Colours::grey);
}

void ControlPanel::startRecording()
{

    clock->startRecording(); // turn on recording
    backgroundColour = Colour(255,0,0);

    filenameText->setColour(Label::textColourId, Colours::black);
    
    recordButton->getNormalImage()->replaceColour(defaultButtonColour, Colours::yellow);

    if (!newDirectoryButton->getEnabledState()) // new directory is required
    {

        for (auto& field : filenameFields)
        {
            field->incrementDirectoryIndex();
        }

        recordingDirectoryName = generateFilenameFromFields(false); // generate new name without placeholders

        for (int recordNodeId : CoreServices::getAvailableRecordNodeIds())
        {
            CoreServices::RecordNode::createNewRecordingDirectory(recordNodeId);
        }

        //std::cout << "Recording directory name: " << recordingDirectoryName << std::endl;
    }
        

    graph->setRecordState(true);

    repaint();
}

void ControlPanel::stopRecording()
{
    graph->setRecordState(false); // turn off recording in processor graph

    clock->stopRecording();
    newDirectoryButton->setEnabledState(true);
    backgroundColour = Colour (51, 51, 51);
    
    recordButton->getNormalImage()->replaceColour(Colours::yellow, defaultButtonColour);

    recordButton->setToggleState(false, dontSendNotification);

    repaint();
}

void ControlPanel::componentBeingDeleted(Component &component)
{
	/*Update filename fields as configured in the popup box upon exit. */
    filenameConfigWindow = std::make_unique<FilenameConfigWindow>(filenameFields);
    filenameText->setButtonText(generateFilenameFromFields(true));

    //TODO: Assumes any change in filename settings should start a new directory next recording
    if (newDirectoryButton->getEnabledState())
        buttonClicked(newDirectoryButton.get());

    CoreServices::saveRecoveryConfig();

	component.removeComponentListener(this);
}

void ControlPanel::buttonClicked(Button* button)
{

    if (button == filenameText.get() && !getRecordingState())
    {

        filenameConfigWindow.reset();
        filenameConfigWindow = std::make_unique<FilenameConfigWindow>(filenameFields);

        CallOutBox& myBox
            = CallOutBox::launchAsynchronously(std::move(filenameConfigWindow), 
                button->getScreenBounds(),
                nullptr);
        myBox.addComponentListener(this);
        myBox.setDismissalMouseClicksAreAlwaysConsumed(true);
        
        return;
    }


    if (button == newDirectoryButton.get()
        && newDirectoryButton->getEnabledState())
    {

        newDirectoryButton->setEnabledState(false);
        clock->resetRecordTime();

        filenameText->setColour(Label::textColourId, Colours::grey);

        return;
    }

    if (button == playButton.get())
    {
        if (playButton->getToggleState())
        {
            startAcquisition();
        }
        else
        {
            stopAcquisition();
        }

        return;
    }

    if (button == recordButton.get())
    {
        if (recordButton->getToggleState())
        {
            
            if (!graph->hasRecordNode())
            {
                CoreServices::sendStatusMessage("Insert at least one Record Node to start recording.");
                recordButton->setToggleState(false, dontSendNotification);
                return;
            } else {
                if (!graph->allRecordNodesAreSynchronized() && !forceRecording)
                {
                    int response = AlertWindow::showOkCancelBox(AlertWindow::WarningIcon,
                                                 "Data streams not synchronized",
                                                 "One or more data streams are not yet synchronized within "
                                                 "a Record Node. Are you sure want to start recording?",
                                                 "Yes", "No");
                    
                    if (!response)
                    {
                        CoreServices::sendStatusMessage("Recording was cancelled.");
                        recordButton->setToggleState(false, dontSendNotification);
                        return;
                    }
                    
                    forceRecording = false;
                    
                }
            }
            
            if (playButton->getToggleState())
            {
                startRecording();
            }
            else
            {
                startAcquisition(true);
            }
        }
        else
        {
            stopRecording();
        }
    }

    if (button == recordOptionsButton.get())
    {
        int id = recordSelector->getSelectedId()-1;
        if (id < 0) return;

        recordEngines[id]->toggleConfigWindow();
    }

}

void ControlPanel::comboBoxChanged(ComboBox* combo)
{

   
    if (combo->getSelectedId() > 0)
    {
        setSelectedRecordEngine(combo->getSelectedId() - 1);
    }
    else
    {
        setSelectedRecordEngine(0);
        combo->setSelectedId(1,dontSendNotification);
    }
    
}

void ControlPanel::setSelectedRecordEngine(int index)
{

    ScopedPointer<RecordEngine> re;

    re = recordEngines[index]->instantiateEngine();
    re->registerManager(recordEngines[index]);

    newDirectoryButton->setEnabledState(false);
    clock->resetRecordTime();

    filenameText->setColour(Label::textColourId, Colours::grey);
    lastEngineIndex = index;
}

void ControlPanel::disableCallbacks()
{

    LOGD("Control panel received signal to disable callbacks.");

    if (audio->callbacksAreActive())
    {
        graph->stopAcquisition();

        LOGD("Stopping audio.");
        audio->endCallbacks();
        LOGD("Disabling processors.");
        
        LOGD("Updating control panel.");
        refreshMeters();
        stopTimer();
        startTimer(60000); // back to refresh every 10 seconds

    }

    playButton->setToggleState(false, dontSendNotification);
    recordButton->setToggleState(false, dontSendNotification);
    recordSelector->setEnabled(true);
    clock->stopRecording();
    clock->stop();

}

void ControlPanel::timerCallback()
{
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

    clock->repaint();

    File currentDirectory = filenameComponent->getCurrentFile();

    diskMeter->updateDiskSpace(1.0f - float(currentDirectory.getBytesFreeOnVolume()) / float(currentDirectory.getVolumeTotalSize()));

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

void ControlPanel::saveStateToXml(XmlElement* xml)
{

    XmlElement* controlPanelState = xml->createNewChildElement("CONTROLPANEL");
    controlPanelState->setAttribute("isOpen",open);
	controlPanelState->setAttribute("recordPath", filenameComponent->getCurrentFile().getFullPathName());
    controlPanelState->setAttribute("recordEngine", recordEngines[recordSelector->getSelectedId()-1]->getID());
    controlPanelState->setAttribute("clockMode", (int) clock->getMode());

    audioEditor->saveStateToXml(xml);

    filenameConfigWindow->saveStateToXml(xml);

}

void ControlPanel::loadStateFromXml(XmlElement* xml)
{

    for (auto* xmlNode : xml->getChildIterator())
    {
        if (xmlNode->hasTagName("CONTROLPANEL"))
        {
			String recordPath = xmlNode->getStringAttribute("recordPath", String());
			if (!recordPath.isEmpty() && !recordPath.equalsIgnoreCase("default"))
			{
                if (!File(recordPath).exists())
                    recordPath = CoreServices::getRecordingParentDirectory().getFullPathName();
				filenameComponent->setCurrentFile(File(recordPath), true, sendNotificationAsync);
			}

			String selectedEngine = xmlNode->getStringAttribute("recordEngine");
			for (int i = 0; i < recordEngines.size(); i++)
			{
				if (recordEngines[i]->getID() == selectedEngine)
				{
					recordSelector->setSelectedId(i + 1, sendNotification);
				}
			}

            clock->setMode((Clock::Mode) xmlNode->getIntAttribute("clockMode", Clock::Mode::DEFAULT));

            bool isOpen = xmlNode->getBoolAttribute("isOpen");
            openState(isOpen);

        }
        else if (xmlNode->hasTagName("RECORDENGINES"))
        {
            for (int i = 0; i < recordEngines.size(); i++)
            {
                for (auto* xmlEngine : xmlNode->getChildWithTagNameIterator("ENGINE"))
                {
                    if (xmlEngine->getStringAttribute("id") == recordEngines[i]->getID())
                        recordEngines[i]->loadParametersFromXml(xmlEngine);
                }
            }
        }
    }

    audioEditor->loadStateFromXml(xml);

    filenameConfigWindow->loadStateFromXml(xml);
    generateFilenameFromFields(true);

}


StringArray ControlPanel::getRecentlyUsedFilenames()
{
    return filenameComponent->getRecentlyUsedFilenames();
}


void ControlPanel::setRecentlyUsedFilenames(const StringArray& filenames)
{
    filenameComponent->setRecentlyUsedFilenames(filenames);
}

static void forceFilenameEditor (int result, ControlPanel* panel)
{
    CallOutBox& myBox
        = CallOutBox::launchAsynchronously(std::move(panel->filenameConfigWindow), 
            panel->filenameText->getScreenBounds(),
            nullptr);
    myBox.addComponentListener(panel);
    myBox.setDismissalMouseClicksAreAlwaysConsumed(true);

    return;
}

String ControlPanel::getRecordingDirectoryName()
{
    return recordingDirectoryName;
}

void ControlPanel::createNewRecordingDirectory()
{
    buttonClicked(newDirectoryButton.get());
}

String ControlPanel::getRecordingDirectoryPrependText()
{
    for (auto& field : filenameFields) //loops in order through prepend, main, append 
    {
        if (field->type == FilenameFieldComponent::Type::PREPEND)
        {
            return field->value;
        }
    }
    return "";
}

void ControlPanel::setRecordingDirectoryPrependText(String text)
{
    for (auto& field : filenameFields) //loops in order through prepend, main, append 
    {
        if (field->type == FilenameFieldComponent::Type::PREPEND)
        {
            if (field->value != text)
            {

                field->newDirectoryNeeded = true;

                if (text.length() == 0)
                    field->state = FilenameFieldComponent::State::NONE;
                else if (text == "auto")
                    field->state = FilenameFieldComponent::State::AUTO;
                else
                {
                    String errString = field->validate(text);
                    if (errString.length())
                        return; //TODO: Notify user of error via HTTPServer
                    field->state = FilenameFieldComponent::State::CUSTOM;
                    field->value = text;
                }
                createNewRecordingDirectory();

                generateFilenameFromFields(true);
            }
        }
    }
}

String ControlPanel::getRecordingDirectoryAppendText()
{
    for (auto& field : filenameFields) //loops in order through prepend, main, append 
    {
        if (field->type == FilenameFieldComponent::Type::APPEND)
        {
            return field->value;
        }
    }
    return "";
}

void ControlPanel::setRecordingDirectoryAppendText(String text)
{
    for (auto& field : filenameFields) //loops in order through prepend, main, append 
    {
        if (field->type == FilenameFieldComponent::Type::APPEND)
        {
            if (field->value != text)
            {

                field->newDirectoryNeeded = true;

                if (text.length() == 0)
                    field->state = FilenameFieldComponent::State::NONE;
                else if (text == "auto")
                    field->state = FilenameFieldComponent::State::AUTO;
                else
                {
                    String errString = field->validate(text);
                    if (errString.length())
                        return; //TODO: Notify user of error via HTTPServer
                    field->state = FilenameFieldComponent::State::CUSTOM;
                    field->value = text;
                }
                createNewRecordingDirectory();

                generateFilenameFromFields(true);
            }
        }
    }
}

String ControlPanel::getRecordingDirectoryBaseText()
{
    for (auto& field : filenameFields) //loops in order through prepend, main, append 
    {
        if (field->type == FilenameFieldComponent::Type::MAIN)
        {
            return field->value;
        }
    }
    return "";
}

void ControlPanel::setRecordingDirectoryBaseText(String text)
{
    for (auto& field : filenameFields) //loops in order through prepend, main, append 
    {
        if (field->type == FilenameFieldComponent::Type::MAIN)
        {
            if (field->value != text)
            {

                field->newDirectoryNeeded = true;

                if (text == "auto")
                {
                    field->state = FilenameFieldComponent::State::AUTO;
                }
                    
                else if ( text.length() > 0 )
                {
                    String errString = field->validate(text);
                    if (errString.length())
                        return; //TODO: Notify user of error via HTTPServer
                    field->state = FilenameFieldComponent::State::CUSTOM;
                    field->value = text;
                }

                createNewRecordingDirectory();

                generateFilenameFromFields(true);
            }
        }
    }
}

String ControlPanel::generateFilenameFromFields(bool usePlaceholderText)
{

    //bool checkForExistingFilename = false;

    String filename = "";

    for (auto& field : filenameFields) //loops in order through prepend, main, append 
    {

        filename += field->getNextValue(usePlaceholderText);

    }

    filenameText->setButtonText(filename);

    return filename;


        /*if (field->state == FilenameFieldComponent::State::NONE)

            continue; //don't add to the filename

        else if (field->state == FilenameFieldComponent::State::CUSTOM)
        {
            filename += field->value; //Add filename field exactly as entered in popup window
            //checkForExistingFilename = true; 
        }
        else //FilenameFieldComponent::State::AUTO
        {

            if (usePlaceholderText)
            {
                filename += field->get
                continue;
            }

            switch (field->type)
            {

                case FilenameFieldComponent::Type::PREPEND:

                    filename += generatePrepend(field->value);
                    break;

                case FilenameFieldComponent::Type::MAIN:

                    filename += generateDatetimeFromFormat(field->value);
                    break;

                case FilenameFieldComponent::Type::APPEND:

                    filename += generateAppend(field->value);
                    break;       
                
                default:
                    break;

            }

        }

    }*/

    // Disallow overwrite of an existing data directory
    /*if (!usePlaceholderText && checkForExistingFilename && getRecordingParentDirectory().getChildFile(filename).exists())
    {

        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
            TRANS("Recording Directory Name Conflict"),
            TRANS("The current custom recording directory name already exists and "
                    "would overwrite existing data. ")
                + newLine
                + TRANS ("Please change the directory name: \"XYZ\"")
                .replace ("XYZ", filename),
            TRANS ("OK"),
            filenameText.get(),
            ModalCallbackFunction::create (forceFilenameEditor, this));

        return filename;
    }

    // Disallow both Prepend and Append fields to have state AUTO
    if (filenameFields[0]->state == FilenameFieldComponent::State::AUTO &&  filenameFields[2]->state == FilenameFieldComponent::State::AUTO)
    {

        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
            TRANS("Recording Directory Name Conflict"),
            TRANS("Auto mode cannot be enabled for both Prepend and Append fields simultaneously")
                + newLine
                + TRANS ("Please fix to continue"),
            TRANS ("OK"),
            filenameText.get(),
            ModalCallbackFunction::create (forceFilenameEditor, this));

        return filename;
    }*/

    //if (updateControlPanel)
    
}

String ControlPanel::generateDatetimeFromFormat(String format)
{

    //TODO: Parse format and generate the proper date string
    //For now use default format: "YYYY-MM-DD_HH-MM-SS"

    //Generate current datetime in default format
    Time calendar = Time::getCurrentTime();

    Array<int> t;
    t.add(calendar.getYear());
    t.add(calendar.getMonth() + 1); // January = 0 
    t.add(calendar.getDayOfMonth());
    t.add(calendar.getHours());
    t.add(calendar.getMinutes());
    t.add(calendar.getSeconds());

    String datestring = "";

    for (int n = 0; n < t.size(); n++)
    {
        if (t[n] < 10)
            datestring += "0";

        datestring += t[n];

        if (n == 2)
            datestring += "_";
        else if (n < 5)
            datestring += "-";
    }

    return datestring;

}

String ControlPanel::generatePrepend(String format)
{

    if (filenameFields[1]->state == FilenameFieldComponent::State::CUSTOM)
    {
        int maxIdx = 0;

        for (DirectoryEntry entry : RangedDirectoryIterator (getRecordingDirectoryName(), false, "*", 1))
        {
            if (entry.getFile().getFileName().contains(filenameFields[1]->value) > 0)
            {
                int idx;
                try
                {
                    idx = std::stoi(entry.getFile().getFileName().substring(0,3).toStdString());
                    if (idx > maxIdx)
                        maxIdx = idx;
                }
                catch(const std::exception& e)
                {
                    idx = 999;
                }

            }
        }

        if (!maxIdx) return format;

        String prependText = String(maxIdx + 1);
        for (int i = 0; i < 4 - prependText.length(); i++)
            prependText = "0" + prependText;

        return prependText + "_";
        
    }

    return format;

}

String ControlPanel::generateAppend(String format)
{
    
    if (filenameFields[1]->state == FilenameFieldComponent::State::CUSTOM)
    {
        int maxIdx = 0;
        for (DirectoryEntry entry : RangedDirectoryIterator (getRecordingDirectoryName(), false, "*", 1))
        {
            if (entry.getFile().getFileName().indexOfWholeWordIgnoreCase(filenameFields[1]->value) == 0)
            {
                int idx;
                try
                {
                    String fn = entry.getFile().getFileName();
                    idx = std::stoi(entry.getFile().getFileName().substring(fn.length()-3,fn.length()).toStdString());
                    if (idx > maxIdx)
                        maxIdx = idx;
                }
                catch(const std::exception& e)
                {
                    idx = 999;
                }

            }
        }

        if (!maxIdx) return format;

        String appendText = String(maxIdx + 1);
        for (int i = 0; i < 4 - appendText.length(); i++)
            appendText = "0" + appendText;

        return "_" + appendText;
        
    }

    return format;

}
