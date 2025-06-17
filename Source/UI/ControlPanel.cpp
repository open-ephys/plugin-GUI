/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2024 Open Ephys

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
#include "../AccessClass.h"
#include "../Processors/PluginManager/PluginManager.h"
#include "../Processors/RecordNode/RecordEngine.h"
#include "FilenameConfigWindow.h"
#include "UIComponent.h"
#include <math.h>
#include <stdio.h>

#include "LookAndFeel/CustomLookAndFeel.h"

const int SIZE_AUDIO_EDITOR_MAX_WIDTH = 500;

NewDirectoryButton::NewDirectoryButton() : Button ("NewDirectory")
{
    XmlDocument xmlDoc (R"(
        <svg  xmlns="http://www.w3.org/2000/svg"  width="24"  height="24"  viewBox="0 0 24 24"  fill="currentColor"  class="icon icon-tabler icons-tabler-filled icon-tabler-folder"><path stroke="none" d="M0 0h24v24H0z" fill="none"/><path d="M9 3a1 1 0 0 1 .608 .206l.1 .087l2.706 2.707h6.586a3 3 0 0 1 2.995 2.824l.005 .176v8a3 3 0 0 1 -2.824 2.995l-.176 .005h-14a3 3 0 0 1 -2.995 -2.824l-.005 -.176v-11a3 3 0 0 1 2.824 -2.995l.176 -.005h4z" /></svg>)");

    newDirectoryIcon = Drawable::createFromSVG (*xmlDoc.getDocumentElement().get());

    newDirectoryIcon->replaceColour (Colours::black, Colours::black);

    setClickingTogglesState (true);
}

void NewDirectoryButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    Colour backgroundColour;
    Colour iconColour;

    if (getToggleState())
    {
        backgroundColour = findColour (ThemeColours::highlightedFill);
        iconColour = Colours::black;
    }
    else
    {
        backgroundColour = findColour (ThemeColours::widgetBackground);
        iconColour = findColour (ThemeColours::defaultText);
    }

    if (isMouseOver)
    {
        backgroundColour = backgroundColour.contrasting (0.1f);
        iconColour = iconColour.contrasting (0.1f);
    }

    g.setColour (findColour (ThemeColours::outline));
    g.fillRoundedRectangle (0, 0, getWidth(), getHeight(), 5);
    g.setColour (backgroundColour);
    g.fillRoundedRectangle (1, 1, getWidth() - 2, getHeight() - 2, 3);

    g.setColour (Colours::black);
    newDirectoryIcon->replaceColour (Colours::black, iconColour);
    newDirectoryIcon->drawWithin (g, juce::Rectangle<float> (2, 2.5, 18, 18), RectanglePlacement::centred, 1.0f);
    newDirectoryIcon->replaceColour (iconColour, Colours::black);
    g.setColour (backgroundColour);
    g.drawRect (10, 9, 2, 6);
    g.drawRect (8, 11, 6, 2);
}

ForceNewDirectoryButton::ForceNewDirectoryButton() : Button ("ForceNewDirectory")
{
    XmlDocument xmlDoc (R"(
       <svg  xmlns="http://www.w3.org/2000/svg"  width="24"  height="24"  viewBox="0 0 24 24"  fill="currentColor"  class="icon icon-tabler icons-tabler-filled icon-tabler-lock"><path stroke="none" d="M0 0h24v24H0z" fill="none"/><path d="M12 2a5 5 0 0 1 5 5v3a3 3 0 0 1 3 3v6a3 3 0 0 1 -3 3h-10a3 3 0 0 1 -3 -3v-6a3 3 0 0 1 3 -3v-3a5 5 0 0 1 5 -5m0 12a2 2 0 0 0 -1.995 1.85l-.005 .15a2 2 0 1 0 2 -2m0 -10a3 3 0 0 0 -3 3v3h6v-3a3 3 0 0 0 -3 -3" /></svg>)");

    forceNewDirectoryIcon = Drawable::createFromSVG (*xmlDoc.getDocumentElement().get());

    setClickingTogglesState (true);
}

void ForceNewDirectoryButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    Colour buttonColour;

    if (getToggleState())
        buttonColour = findColour (ThemeColours::highlightedFill);
    else
        buttonColour = findColour (ThemeColours::controlPanelText);

    if (isMouseOver)
        buttonColour = buttonColour.brighter (0.2f);

    forceNewDirectoryIcon->replaceColour (Colours::black, buttonColour);

    forceNewDirectoryIcon->drawWithin (g, getLocalBounds().toFloat(), RectanglePlacement::centred, 1.0f);

    forceNewDirectoryIcon->replaceColour (buttonColour, Colours::black);
}

FilenameEditorButton::FilenameEditorButton()
    : TextButton ("Filename Editor")
{
    setTooltip ("Edit the recording filename");
}

PlayButton::PlayButton()
    : DrawableButton ("Play Button", DrawableButton::ImageRaw)
{
    setColour (DrawableButton::backgroundColourId, Colours::darkgrey.withAlpha (0.0f));
    setColour (DrawableButton::backgroundOnColourId, Colours::darkgrey.withAlpha (0.0f));
    setClickingTogglesState (true);
    setTooltip ("Start/stop acquisition");

    updateImages (false);
}

void PlayButton::updateImages (bool acquisitionIsActive)
{
    DrawablePath normal, over, down;

    Colour buttonColour = acquisitionIsActive ? Colours::yellow : findColour (ThemeColours::controlPanelText);

    Path p;
    p.addTriangle (0.0f, 0.0f, 0.0f, 20.0f, 18.0f, 10.0f);
    p.applyTransform (AffineTransform::scale (0.8f));
    p.applyTransform (AffineTransform::translation (2, 2));
    normal.setPath (p);
    normal.setFill (buttonColour);
    normal.setStrokeThickness (0.0f);

    over.setPath (p);
    over.setFill (buttonColour);
    over.setStrokeThickness (2.0f);
    over.setStrokeFill (buttonColour);

    down.setPath (p);
    down.setFill (Colours::white);
    down.setStrokeThickness (2.0f);
    down.setStrokeFill (Colours::white);

    setImages (&normal, &over, &down);
}

RecordButton::RecordButton()
    : DrawableButton ("Record Button", DrawableButton::ImageRaw)
{
    setColour (DrawableButton::backgroundColourId, Colours::darkgrey.withAlpha (0.0f));
    setColour (DrawableButton::backgroundOnColourId, Colours::darkgrey.withAlpha (0.0f));
    setClickingTogglesState (true);
    setTooltip ("Start/stop writing to disk");

    updateImages (false);
}

void RecordButton::updateImages (bool recordingIsActive)
{
    DrawablePath normal, over, down;

    Colour buttonColour = recordingIsActive ? Colours::yellow : findColour (ThemeColours::controlPanelText);

    Path p;
    p.addEllipse (0.0, 0.0, 20.0, 20.0);
    p.applyTransform (AffineTransform::scale (0.8f));
    p.applyTransform (AffineTransform::translation (2, 2));
    normal.setPath (p);
    normal.setFill (buttonColour);
    normal.setStrokeThickness (0.0f);

    over.setPath (p);
    over.setFill (buttonColour);
    over.setStrokeThickness (2.0f);
    over.setStrokeFill (buttonColour);

    down.setPath (p);
    down.setFill (Colours::white);
    down.setStrokeThickness (2.0f);
    down.setStrokeFill (Colours::white);

    setImages (&normal, &over, &down);
}

CPUMeter::CPUMeter() : Component ("CPU Meter"),
                       cpu (0.0f)
{
    font = FontOptions ("Silkscreen", "Regular", 14);

    setTooltip ("CPU usage");
}

void CPUMeter::updateCPU (float usage)
{
    cpu = usage;

    repaint();
}

void CPUMeter::paint (Graphics& g)
{
    g.fillAll (findColour (ThemeColours::defaultFill));

    g.setColour (Colours::yellow);
    g.fillRect (0.0f, 0.0f, getWidth() * cpu, float (getHeight()));

    g.setColour (findColour (ThemeColours::outline));
    g.drawRect (0, 0, getWidth(), getHeight(), 1);

    g.setColour (findColour (ThemeColours::defaultText));
    g.setFont (font);
    g.drawSingleLineText ("CPU", 65, 12);
}

void CPUMeter::mouseUp (const MouseEvent& e)
{
    if (e.mods.isLeftButtonDown())
    {
        AccessClass::getUIComponent()->showBubbleMessage (this,
                                                          "Displays the fraction of available time that the"
                                                          " signal chain takes to complete one processing cycle");
    }
}

DiskSpaceMeter::DiskSpaceMeter() : Component ("Disk Space Meter"),
                                   diskFree (0.0f)

{
    font = FontOptions ("Silkscreen", "Regular", 14);

    setTooltip ("Disk space available");
}

void DiskSpaceMeter::updateDiskSpace (float percent)
{
    diskFree = percent;

    repaint();
}

void DiskSpaceMeter::paint (Graphics& g)
{
    g.fillAll (findColour (ThemeColours::defaultFill));

    g.setColour (findColour (ThemeColours::widgetBackground));
    if (diskFree > 0)
    {
        if (diskFree > 1.0)
            diskFree = 1.0;
        g.fillRect (0.0f, 0.0f, getWidth() * diskFree, float (getHeight()));
    }

    g.setColour (findColour (ThemeColours::outline));
    g.drawRect (0, 0, getWidth(), getHeight(), 1);

    g.setColour (findColour (ThemeColours::defaultText));
    g.setFont (font);
    g.drawSingleLineText ("DF", 75, 12);
}

void DiskSpaceMeter::mouseUp (const MouseEvent& e)
{
    if (e.mods.isLeftButtonDown())
    {
        AccessClass::getUIComponent()->showBubbleMessage (this,
                                                          "Displays the fraction of available disk space"
                                                          " used by the default recording directory");
    }
}

Clock::Clock()
{
    clockFont = FontOptions ("CP Mono", "Light", 30.0f);
}

void Clock::paint (Graphics& g)
{
    drawTime (g);
}

void Clock::drawTime (Graphics& g)
{
    if (isRunning)
    {
        int64 now = Time::currentTimeMillis();
        int64 diff = now - lastTime;
        totalTime += diff;
        latestAcquisitionTime += diff;

        if (isRecording)
        {
            totalRecordingTime += diff;
        }

        lastTime = Time::currentTimeMillis();
    }

    int m;
    int s;
    int h;

    int64 timeToDraw;

    if (referenceTime == ACQUISITION_START)
    {
        timeToDraw = latestAcquisitionTime;
    }
    else
    {
        if (isRecording)
            timeToDraw = totalRecordingTime;
        else
            timeToDraw = totalTime;
    }

    h = floor (timeToDraw / 3600000.0f);
    m = floor (timeToDraw / 60000.0);
    s = floor ((timeToDraw - m * 60000.0) / 1000.0);

    if (isRecording)
    {
        g.setColour (Colours::black);
    }
    else
    {
        if (isRunning)
            g.setColour (findColour (ThemeColours::controlPanelText));
        else
            g.setColour (findColour (ThemeColours::controlPanelText).withAlpha (0.8f));
    }

    String timeString = "";

    if (mode == DEFAULT)
    {
        timeString += m;
        timeString += " min ";
        timeString += s;
        timeString += " s";
    }
    else
    {
        if (h < 10)
            timeString += "0";
        timeString += h;
        timeString += ":";

        int minutes = m - h * 60;

        if (minutes < 10)
            timeString += "0";
        timeString += minutes;
        timeString += ":";

        if (s < 10)
            timeString += "0";
        timeString += s;
    }

    g.setFont (clockFont);
    g.drawText (timeString, 0, 0, getWidth(), getHeight(), Justification::centred, false);
}

void Clock::start()
{
    if (! isRunning)
    {
        isRunning = true;
        lastTime = Time::currentTimeMillis();
    }
}

void Clock::resetAcquisitionTime()
{
    latestAcquisitionTime = 0;
}

void Clock::resetRecordingTime()
{
    totalRecordingTime = 0;
}

int64 Clock::getRecordingTime() const
{
    return totalRecordingTime;
}

void Clock::startRecording()
{
    if (! isRecording)
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

void Clock::setMode (Mode m)
{
    mode = m;

    repaint();
}

void Clock::setReferenceTime (ReferenceTime t)
{
    referenceTime = t;

    repaint();
}

void Clock::mouseDown (const MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        PopupMenu m;
        m.setLookAndFeel (&getLookAndFeel());

        m.addItem (1, "Display mode", false);
        m.addItem (2, "Default", true, mode == DEFAULT);
        m.addItem (3, "HH:MM:SS", true, mode == HHMMSS);
        m.addSeparator();
        m.addItem (4, "Reference time", false);
        m.addItem (5, "Cumulative", true, referenceTime == CUMULATIVE);
        m.addItem (6, "Acquisition start", true, referenceTime == ACQUISITION_START);

        int result = m.showMenu (PopupMenu::Options {}.withStandardItemHeight (20));

        if (result == 2)
        {
            setMode (DEFAULT);
        }
        else if (result == 3)
        {
            setMode (HHMMSS);
        }
        else if (result == 5)
        {
            setReferenceTime (CUMULATIVE);
        }
        else if (result == 6)
        {
            setReferenceTime (ACQUISITION_START);
        }
    }
}

ControlPanel::ControlPanel (ProcessorGraph* graph_, AudioComponent* audio_, bool isConsoleApp_)
    : graph (graph_),
      audio (audio_),
      isConsoleApp (isConsoleApp_)
{
    AccessClass::setControlPanel (this);

    recordButton = std::make_unique<RecordButton>();
    recordButton->addListener (this);

    playButton = std::make_unique<PlayButton>();
    playButton->addListener (this);

    const File dataDirectory = CoreServices::getDefaultUserSaveDirectory();

    filenameComponent = std::make_unique<FilenameComponent> ("folder selector",
                                                             dataDirectory.getFullPathName(),
                                                             true,
                                                             true,
                                                             true,
                                                             "*",
                                                             "",
                                                             "");
    filenameComponent->addListener (this);
    addChildComponent (filenameComponent.get());

    filenameFields.add (std::make_shared<FilenameFieldComponent> (
        FilenameFieldComponent::Type::PREPEND, FilenameFieldComponent::State::NONE, ""));
    filenameFields.add (std::make_shared<FilenameFieldComponent> (
        FilenameFieldComponent::Type::MAIN, FilenameFieldComponent::State::AUTO, "MM-DD-YYYY_HH-MM-SS"));
    filenameFields.add (std::make_shared<FilenameFieldComponent> (
        FilenameFieldComponent::Type::APPEND, FilenameFieldComponent::State::NONE, ""));

    filenameText = std::make_unique<FilenameEditorButton>();
    generateFilenameFromFields (true);
    filenameText->addListener (this);

    recordSelector = std::make_unique<ComboBox> ("Control Panel Record Engine Selector");
    recordSelector->addListener (this);
    addChildComponent (recordSelector.get());

    recordOptionsButton = std::make_unique<UtilityButton> ("R");
    recordOptionsButton->setEnabledState (true);
    recordOptionsButton->addListener (this);
    recordOptionsButton->setTooltip ("Configure options for selected record engine");
    addChildComponent (recordOptionsButton.get());

    newDirectoryButton = std::make_unique<NewDirectoryButton>();
    newDirectoryButton->setEnabled (false);
    newDirectoryButton->addListener (this);
    newDirectoryButton->setTooltip ("Start a new data directory for next recording");
    newDirectoryButton->setToggleState (newDirectoryNeeded, sendNotification);
    newDirectoryButton->setClickingTogglesState (true);
    addChildComponent (newDirectoryButton.get());

    forceNewDirectoryButton = std::make_unique<ForceNewDirectoryButton>();
    forceNewDirectoryButton->setEnabled (true);
    forceNewDirectoryButton->addListener (this);
    forceNewDirectoryButton->setTooltip ("Force a new data directory for each recording");
    addChildComponent (forceNewDirectoryButton.get());

    clock = std::make_unique<Clock>();
    cpuMeter = std::make_unique<CPUMeter>();
    diskMeter = std::make_unique<DiskSpaceMeter>();
    showHideRecordingOptionsButton = std::make_unique<CustomArrowButton>();
    showHideRecordingOptionsButton->addListener (this);
    showHideRecordingOptionsButton->setTooltip ("Show/hide recording options");
    filenameConfigWindow = std::make_unique<FilenameConfigWindow> (filenameFields);

    if (! isConsoleApp)
    {
        audioEditor = (AudioEditor*) graph->getAudioNode()->createEditor();
        addAndMakeVisible (audioEditor);

        addAndMakeVisible (playButton.get());
        addAndMakeVisible (recordButton.get());
        addAndMakeVisible (clock.get());
        addAndMakeVisible (cpuMeter.get());
        addAndMakeVisible (diskMeter.get());
        addAndMakeVisible (showHideRecordingOptionsButton.get());
        addAndMakeVisible (filenameText.get());

        refreshMeters();

        startTimer (60000); // update disk space every minute

        setWantsKeyboardFocus (true);
    }
}

ControlPanel::~ControlPanel()
{
}

void ControlPanel::setRecordingState (bool t, bool force)
{
    forceRecording = force;

    recordButton->setToggleState (t, sendNotification);
}

bool ControlPanel::getRecordingState()
{
    return recordButton->getToggleState();
}

int64 ControlPanel::getRecordingTime() const
{
    return clock->getRecordingTime();
}

void ControlPanel::setRecordingParentDirectory (String path)
{
    if (getRecordingState())
    {
        LOGE ("Cannot change recording parent directory while recording is active. "
              "Please stop recording before changing the directory.");
        return;
    }

    File newFile (path);
    filenameComponent->setCurrentFile (newFile, true, sendNotificationSync);

    for (auto recNode : AccessClass::getProcessorGraph()->getRecordNodes())
    {
        recNode->setDefaultRecordingDirectory (newFile);
    }
}

File ControlPanel::getRecordingParentDirectory()
{
    return filenameComponent->getCurrentFile();
}

bool ControlPanel::getAcquisitionState()
{
    return playButton->getToggleState();
}

void ControlPanel::setAcquisitionState (bool state)
{
    playButton->setToggleState (state, sendNotification);
}

void ControlPanel::startAcquisition (bool recordingShouldAlsoStart)
{
    if (! audio->checkForDevice())
    {
        playButton->setToggleState (false, dontSendNotification);
        recordButton->setToggleState (false, dontSendNotification);

        String errorMsg = "No output device found. Unable to start acquisition.";
        LOGE (errorMsg);

        if (! isConsoleApp)
        {
            errorMsg += "\n\nAn active audio output device is required to process data. "
                        "Try changing the audio device type in the audio configuration window (click on the \"Latency\" button in the Control Panel) "
                        "and ensure there is an output device selected.";

            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         "Acquisition Error",
                                         errorMsg);
        }

        return;
    }

    if (audio->getSampleRate() < 44100)
    {
        playButton->setToggleState (false, dontSendNotification);
        recordButton->setToggleState (false, dontSendNotification);

        String errorMsg = "Sample rate too low. Unable to start acquisition.";
        LOGE (errorMsg);

        if (! isConsoleApp)
        {
            errorMsg += "\n\nThe sample rate must be at least 44.1 kHz to process data. "
                        "Try changing the sample rate in the audio configuration window (click on the \"Latency\" button in the Control Panel) "
                        "to a value of 44.1 kHz or higher. If no such option is available, try changing the output device type.";

            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         "Acquisition Error",
                                         errorMsg);
        }

        return;
    }

    if (! isConsoleApp && audioEditor->isAudioConfigurationWindowVisible())
    {
        audioEditor->disable();
        audioEditor->enable();
    }

    if (graph->isReady()) // check that all processors are enabled
    {
        graph->updateConnections();

        LOGC ("Starting acquisition");

        graph->startAcquisition(); // inform processors that acquisition will start

        if (recordingShouldAlsoStart)
        {
            startRecording();
            playButton->setToggleState (true, dontSendNotification);
        }

        if (! isConsoleApp)
        {
            playButton->updateImages (true);

            audioEditor->disable();

            clock->start(); // starts the clock

            stopTimer();
            startTimer (250); // refresh every 250 ms

            recordSelector->setEnabled (false); // why is this outside the "if" statement?
            recordOptionsButton->setEnabled (false);
        }

        clock->resetAcquisitionTime();
        audio->beginCallbacks(); // starts acquisition callbacks
    }
}

void ControlPanel::stopAcquisition()
{
    if (recordButton->getToggleState())
    {
        stopRecording();
    }

    graph->stopAcquisition();

    LOGC ("Stopping acquisition");

    audio->endCallbacks();

    if (! isConsoleApp)
    {
        playButton->updateImages (false);

        refreshMeters();

        clock->stop();
        audioEditor->enable();

        stopTimer();
        startTimer (60000); // back to refresh every minute

        recordSelector->setEnabled (true);
        recordOptionsButton->setEnabled (true);
    }
}

void ControlPanel::updateRecordEngineList()
{
    int selectedEngine = recordSelector->getSelectedId();
    recordSelector->clear (dontSendNotification);
    recordEngines.clear();
    int id = 1;

    LOGD ("Built-in Record Engine count: ", RecordEngineManager::getNumOfBuiltInEngines());

    for (int i = 0; i < RecordEngineManager::getNumOfBuiltInEngines(); i++)
    {
        RecordEngineManager* rem = RecordEngineManager::createBuiltInEngineManager (i);
        recordSelector->addItem (rem->getName(), id++);
        LOGD ("Adding Record Engine: ", rem->getName());
        recordEngines.add (rem);
    }
    LOGD ("Plugin Record Engine count: ", AccessClass::getPluginManager()->getNumRecordEngines());
    for (int i = 0; i < AccessClass::getPluginManager()->getNumRecordEngines(); i++)
    {
        Plugin::RecordEngineInfo info;
        info = AccessClass::getPluginManager()->getRecordEngineInfo (i);
        recordSelector->addItem (info.name, id++);
        LOGD ("Adding Record Engine: ", info.name);
        recordEngines.add (info.creator());
    }

    if (selectedEngine < 1)
    {
        setSelectedRecordEngine (0);
        recordSelector->setSelectedId (1, dontSendNotification);
    }

    else
    {
        setSelectedRecordEngine (selectedEngine - 1);
        recordSelector->setSelectedId (selectedEngine, dontSendNotification);
    }
}

std::vector<RecordEngineManager*> ControlPanel::getAvailableRecordEngines()
{
    std::vector<RecordEngineManager*> engines;

    for (auto engine : recordEngines)
    {
        engines.push_back (engine);
    }

    return engines;
}

String ControlPanel::getSelectedRecordEngineId()
{
    return recordEngines[recordSelector->getSelectedId() - 1]->getID();
}

bool ControlPanel::setSelectedRecordEngineId (String id)
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
            recordSelector->setSelectedId (i + 1, sendNotificationSync);
            return true;
        }
    }
    return false;
}

void ControlPanel::createPaths()
{
    int w = getWidth() - 435;
    if (w > 22)
        w = 22;

    int h1 = getHeight() - 32;
    int h2 = getHeight();
    int indent = 5;

    p1.clear();
    p1.startNewSubPath (0, h1);
    p1.lineTo (w, h1);
    p1.lineTo (w + indent, h1 + indent);
    p1.lineTo (w + indent, h2 - indent);
    p1.lineTo (w + indent * 2, h2);
    p1.lineTo (0, h2);
    p1.closeSubPath();

    p2.clear();
    p2.startNewSubPath (getWidth(), h2 - indent);
    p2.lineTo (getWidth(), h2);
    p2.lineTo (getWidth() - indent, h2);
    p2.closeSubPath();
}

void ControlPanel::paint (Graphics& g)
{
    if (! getRecordingState())
        g.setColour (findColour (ThemeColours::controlPanelBackground));
    else
        g.setColour (Colour (255, 0, 0));

    g.fillRect (0, 0, getWidth(), getHeight());

    if (open)
    {
        g.setColour (findColour (ThemeColours::windowBackground));
        g.fillPath (p1);
        g.fillPath (p2);

        // Fill record engine selector background area with control panel background colour
        g.setColour (findColour (ThemeColours::controlPanelBackground));
        g.fillRoundedRectangle (recordSelector->getBounds().toFloat(), 3.0f);
        g.fillRoundedRectangle (filenameComponent->getBounds().toFloat(), 3.0f);
    }
}

void ControlPanel::resized()
{
    const int w = getWidth();
    const int h = 32;

    // We have 3 possible layout schemes:
    // when there are 1, 2 or 3 rows within which our elements are placed.
    const int twoRowsWidth = 750;
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
                                   ? 3
                                   : 1;

    // Set positions for CPU and Disk meter components
    // ====================================================================
    int meterComponentsY = h / 4;
    int meterComponentsWidth = h * 3;
    const int meterComponentsHeight = h / 2;
    const int meterComponentsMargin = 8;
    switch (currentNumRows)
    {
        case 2:
            meterComponentsY += offset1;
            break;

        case 3:
            meterComponentsY += offset1 + offset2;
            break;

        default:
            break;
    }

    juce::Rectangle<int> meterBounds (meterComponentsMargin, meterComponentsY, meterComponentsWidth, meterComponentsHeight);
    cpuMeter->setBounds (meterBounds);
    diskMeter->setBounds (meterBounds.translated (meterComponentsWidth + meterComponentsMargin, 0));
    // ====================================================================

    // Set positions for controls and clock
    // ====================================================================
    const int controlButtonWidth = 22;
    const int controlButtonHeight = 22;
    const int clockWidth = 195;
    const int controlsMargin = 10;
    const int totalControlsWidth = controlButtonWidth * 2 + controlsMargin + clockWidth;
    if (currentNumRows != 3)
    {
        playButton->setBounds (w - clockWidth - 2 * (controlButtonHeight + controlsMargin) - 22, 5, controlButtonWidth, controlButtonHeight);
        recordButton->setBounds (w - clockWidth - controlButtonHeight - controlsMargin - 22, 5, controlButtonWidth, controlButtonHeight);
        clock->setBounds (w - clockWidth - 32, 0, clockWidth, h);
    }
    else
    {
        const int startX = (w - totalControlsWidth) / 2;
        playButton->setBounds (startX, 5, controlButtonWidth, controlButtonHeight);
        recordButton->setBounds (startX + h, 5, controlButtonWidth, controlButtonHeight);
        clock->setBounds (startX + h * 2 + controlsMargin * 2, 0, clockWidth, h);
    }
    // ====================================================================

    if (audioEditor) //if (audioEditor)
    {
        const bool isThereElementOnLeft = diskMeter->getBounds().getY() <= h;
        const bool isSecondRowAvailable = diskMeter->getBounds().getY() >= 2 * h;
        const int leftElementWidth = diskMeter->getBounds().getRight();
        const int rightElementWidth = w - playButton->getBounds().getX();

        int maxAvailableWidthForEditor = w;
        if (isThereElementOnLeft)
            maxAvailableWidthForEditor -= leftElementWidth + rightElementWidth;
        else if (! isSecondRowAvailable)
            maxAvailableWidthForEditor -= rightElementWidth;

        const bool isEnoughSpaceForFullSize = maxAvailableWidthForEditor >= SIZE_AUDIO_EDITOR_MAX_WIDTH;

        const int rowIndex = (isSecondRowAvailable) ? 1 : 0;
        const int editorWidth = isEnoughSpaceForFullSize
                                    ? SIZE_AUDIO_EDITOR_MAX_WIDTH
                                    : maxAvailableWidthForEditor * 0.95;
        const int editorX = (rowIndex != 0)
                                ? (w - editorWidth) / 2
                            : isThereElementOnLeft
                                ? leftElementWidth + (maxAvailableWidthForEditor - editorWidth) / 2
                                : (maxAvailableWidthForEditor - editorWidth) / 2;
        const int editorY = (rowIndex == 0) ? 0 : offset1;

        audioEditor->setBounds (editorX, editorY, editorWidth, h);
    }

    if (open)
        showHideRecordingOptionsButton->setBounds (w - 28, getHeight() - 5 - h * 2 + 10, h - 10, h - 10);
    else
        showHideRecordingOptionsButton->setBounds (w - 28, getHeight() - 5 - h + 10, h - 10, h - 10);

    createPaths();

    if (open)
    {
        int topBound = getHeight() - h + 10 - 5;

        /* Record engine selector */
        recordSelector->setBounds ((w - 435) > 40 ? 35 : w - 450, topBound, 125, h - 10);
        recordSelector->setVisible (true);

        /* Shows the default root recording directory for new RecordNodes */
        filenameComponent->setBounds (165, topBound, w - 510, h - 10);
        filenameComponent->setVisible (true);

        /* Shows the file name / format to be used for the next recording */
        filenameText->setBounds (165 + w - 504, topBound, 280, h - 10);
        filenameText->setVisible (true);

        /* Toggle state determines whether a new root directory will be created for the next recording */
        newDirectoryButton->setBounds (w - h - 22, topBound, h - 10, h - 10);
        newDirectoryButton->setVisible (true);

        /* Toggle state determines whether new root directories will automatically be created for each recording */
        forceNewDirectoryButton->setBounds (w - h + 4, topBound, h - 10, h - 10);
        forceNewDirectoryButton->setVisible (true);
    }
    else
    {
        filenameComponent->setVisible (false);
        newDirectoryButton->setVisible (false);
        forceNewDirectoryButton->setVisible (false);
        filenameText->setVisible (false);
        recordSelector->setVisible (false);
        recordOptionsButton->setVisible (false);
    }

    repaint();
}

void ControlPanel::openState (bool os)
{
    open = os;

    showHideRecordingOptionsButton->setToggleState (os, dontSendNotification);

    if (! isConsoleApp)
        AccessClass::getUIComponent()->childComponentChanged();
}

void ControlPanel::labelTextChanged (Label* label)
{
    for (auto* node : AccessClass::getProcessorGraph()->getRecordNodes())
    {
        node->newDirectoryNeeded = true;
    }

    clock->resetRecordingTime();
}

void ControlPanel::startRecording()
{
    if (newDirectoryButton->getToggleState())
        clock->resetRecordingTime();

    clock->startRecording(); // turn on recording

    // filenameText->setColour(Label::textColourId, Colours::black);

    recordButton->updateImages (true);

    showHideRecordingOptionsButton->setCustomBackground (true, Colour (255, 0, 0));

    if (newDirectoryButton->getToggleState()) // new directory is required
    {
        for (auto& field : filenameFields)
        {
            field->incrementDirectoryIndex();
        }

        recordingDirectoryName = generateFilenameFromFields (false); // generate new name without placeholders

        for (int recordNodeId : CoreServices::getAvailableRecordNodeIds())
        {
            CoreServices::RecordNode::createNewRecordingDirectory (recordNodeId);
        }
    }

    filenameComponent->setEnabled (false);

    graph->setRecordState (true);

    LOGC ("Starting recording");

    repaint();
}

void ControlPanel::stopRecording()
{
    hasRecorded = true;

    graph->setRecordState (false); // turn off recording in processor graph

    clock->stopRecording();

    if (forceNewDirectoryButton->getToggleState())
    {
        newDirectoryNeeded = true;
        newDirectoryButton->setEnabled (false);
    }
    else
    {
        newDirectoryNeeded = false;
        newDirectoryButton->setEnabled (true);
    }

    newDirectoryButton->setToggleState (newDirectoryNeeded, dontSendNotification);

    recordButton->updateImages (false);
    showHideRecordingOptionsButton->setCustomBackground (false, findColour (ThemeColours::windowBackground));

    recordButton->setToggleState (false, dontSendNotification);

    filenameComponent->setEnabled (true);

    LOGC ("Stopping recording");

    repaint();
}

void ControlPanel::componentBeingDeleted (Component& component)
{
    /*Update filename fields as configured in the popup box upon exit. */
    filenameConfigWindow = std::make_unique<FilenameConfigWindow> (filenameFields);
    filenameText->setButtonText (generateFilenameFromFields (true));

    //TODO: Assumes any change in filename settings should start a new directory next recording
    if (! newDirectoryButton->getToggleState())
    {
        newDirectoryNeeded = true;
        newDirectoryButton->setToggleState (newDirectoryNeeded, dontSendNotification);
    }

    CoreServices::saveRecoveryConfig();

    component.removeComponentListener (this);
}

void ControlPanel::colourChanged()
{
    playButton->updateImages (getAcquisitionState());
    recordButton->updateImages (getRecordingState());
}

void ControlPanel::buttonClicked (Button* button)
{
    if (button == showHideRecordingOptionsButton.get())
    {
        openState (button->getToggleState());
        AccessClass::getUIComponent()->resized();
        return;
    }

    if (button == filenameText.get() && ! getRecordingState())
    {
        filenameConfigWindow.reset();
        filenameConfigWindow = std::make_unique<FilenameConfigWindow> (filenameFields);
        filenameConfigWindow->setLookAndFeel (&getLookAndFeel());

        CallOutBox& myBox = CallOutBox::launchAsynchronously (std::move (filenameConfigWindow),
                                                              button->getScreenBounds(),
                                                              nullptr);
        myBox.addComponentListener (this);
        myBox.setDismissalMouseClicksAreAlwaysConsumed (true);

        return;
    }

    if (button == newDirectoryButton.get())
    {
        //Setting the button state only takes effect on the next recording

        return;
    }

    if (button == forceNewDirectoryButton.get())
    {
        if (button->getToggleState())
        {
            newDirectoryNeeded = true;
            newDirectoryButton->setToggleState (newDirectoryNeeded, dontSendNotification);
            newDirectoryButton->setEnabled (false);
        }
        else
        {
            if (hasRecorded)
                newDirectoryButton->setEnabled (true);
        }
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
            if (! graph->hasRecordNode())
            {
                if (! isConsoleApp)
                {
                    getLookAndFeel().playAlertSound();
                    AccessClass::getUIComponent()->showBubbleMessage (getRecordButton(),
                                                                      "Insert at least one Record Node to start recording");
                }
                CoreServices::sendStatusMessage ("Insert at least one Record Node to start recording.");
                recordButton->setToggleState (false, dontSendNotification);
                return;
            }
            else
            {
                if (! graph->allRecordNodeDirectoriesAreValid() && getAcquisitionState())
                {
                    recordButton->setToggleState (false, dontSendNotification);

                    if (! isConsoleApp)
                    {
                        getLookAndFeel().playAlertSound();
                        AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                                     "Recording could not start",
                                                     "One or more Record Nodes have an invalid recording path. "
                                                     "Please ensure all Record Nodes are configured with a valid path before starting the recording.");
                    }
                    CoreServices::sendStatusMessage ("One or more Record Nodes have invalid recording path");
                    return;
                }

                if (! graph->allRecordNodesAreSynchronized() && ! forceRecording)
                {
                    recordButton->setToggleState (false, dontSendNotification);

                    int response = AlertWindow::showOkCancelBox (AlertWindow::WarningIcon,
                                                                 "Data streams not synchronized",
                                                                 "One or more data streams are not yet synchronized within "
                                                                 "a Record Node. Are you sure want to start recording?",
                                                                 "Yes",
                                                                 "No");

                    if (! response)
                    {
                        CoreServices::sendStatusMessage ("Recording was cancelled.");
                        return;
                    }

                    recordButton->setToggleState (true, dontSendNotification);
                    forceRecording = false;
                }
            }

            if (playButton->getToggleState())
            {
                startRecording();
            }
            else
            {
                startAcquisition (true);
            }
        }
        else
        {
            stopRecording();
        }
    }

    if (button == recordOptionsButton.get())
    {
        int id = recordSelector->getSelectedId() - 1;
        if (id < 0)
            return;
    }
}

void ControlPanel::comboBoxChanged (ComboBox* combo)
{
    if (combo->getSelectedId() > 0)
    {
        setSelectedRecordEngine (combo->getSelectedId() - 1);
    }
    else
    {
        setSelectedRecordEngine (0);
        combo->setSelectedId (1, dontSendNotification);
    }
}

void ControlPanel::setSelectedRecordEngine (int index)
{
    ScopedPointer<RecordEngine> re;

    re = recordEngines[index]->instantiateEngine();
    re->registerManager (recordEngines[index]);

    newDirectoryButton->setEnabled (false);
    clock->resetRecordingTime();

    lastEngineIndex = index;
}

void ControlPanel::filenameComponentChanged (FilenameComponent* fnComponent)
{
    File currentFile = fnComponent->getCurrentFile();

    if (! currentFile.exists())
    {
        StringArray fileNames = fnComponent->getRecentlyUsedFilenames();
        fileNames.removeString (currentFile.getFullPathName(), true);
        fnComponent->setCurrentFile (CoreServices::getDefaultUserSaveDirectory(), true, dontSendNotification);
        fnComponent->setRecentlyUsedFilenames (fileNames);

        if (! isConsoleApp)
        {
            AccessClass::getUIComponent()->showBubbleMessage (fnComponent,
                                                              "The selected recording directory does not exist. "
                                                              "Setting the parent recording directory to the default user save directory.");
        }

        return;
    }
    else
    {
        for (auto recNode : AccessClass::getProcessorGraph()->getRecordNodes())
        {
            recNode->setDefaultRecordingDirectory (currentFile);
        }
    }
}

void ControlPanel::disableCallbacks()
{
    LOGD ("Control panel received signal to disable callbacks.");

    if (audio->callbacksAreActive())
    {
        graph->stopAcquisition();

        LOGD ("Stopping audio.");
        audio->endCallbacks();
        LOGD ("Disabling processors.");

        LOGD ("Updating control panel.");
        refreshMeters();
        stopTimer();
        startTimer (60000); // back to refresh every 10 seconds
    }

    playButton->setToggleState (false, dontSendNotification);
    recordButton->setToggleState (false, dontSendNotification);
    recordSelector->setEnabled (true);
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
        cpuMeter->updateCPU (audio->deviceManager.getCpuUsage());
    }
    else
    {
        cpuMeter->updateCPU (0.0f);
    }

    clock->repaint();

    File currentDirectory = filenameComponent->getCurrentFile();

    diskMeter->updateDiskSpace (1.0f - float (currentDirectory.getBytesFreeOnVolume()) / float (currentDirectory.getVolumeTotalSize()));

    if (initialize)
    {
        stopTimer();
        startTimer (60000); // check for disk updates every minute
        initialize = false;
    }
}

void ControlPanel::toggleState()
{
    open = ! open;

    showHideRecordingOptionsButton->setToggleState (open, dontSendNotification);
    AccessClass::getUIComponent()->childComponentChanged();
}

void ControlPanel::saveStateToXml (XmlElement* xml)
{
    XmlElement* controlPanelState = xml->createNewChildElement ("CONTROLPANEL");
    controlPanelState->setAttribute ("isOpen", open);
    controlPanelState->setAttribute ("recordPath", filenameComponent->getCurrentFile().getFullPathName());
    controlPanelState->setAttribute ("recordEngine", recordEngines[recordSelector->getSelectedId() - 1]->getID());
    controlPanelState->setAttribute ("clockMode", (int) clock->getMode());
    controlPanelState->setAttribute ("clockReferenceTime", (int) clock->getReferenceTime());
    controlPanelState->setAttribute ("forceNewDirectory", forceNewDirectoryButton->getToggleState());

    if (! isConsoleApp)
        audioEditor->saveStateToXml (xml);

    filenameConfigWindow->saveStateToXml (xml);
}

void ControlPanel::loadStateFromXml (XmlElement* xml)
{
    for (auto* xmlNode : xml->getChildIterator())
    {
        if (xmlNode->hasTagName ("CONTROLPANEL"))
        {
            String recordPath = xmlNode->getStringAttribute ("recordPath", String());
            if (! recordPath.isEmpty() && ! recordPath.equalsIgnoreCase ("default"))
            {
                if (! File (recordPath).exists())
                    recordPath = CoreServices::getRecordingParentDirectory().getFullPathName();
                filenameComponent->setCurrentFile (File (recordPath), true, sendNotificationAsync);
            }

            String selectedEngine = xmlNode->getStringAttribute ("recordEngine");
            for (int i = 0; i < recordEngines.size(); i++)
            {
                if (recordEngines[i]->getID() == selectedEngine)
                {
                    recordSelector->setSelectedId (i + 1, sendNotification);
                }
            }

            clock->setMode ((Clock::Mode) xmlNode->getIntAttribute ("clockMode", Clock::Mode::DEFAULT));
            clock->setReferenceTime ((Clock::ReferenceTime) xmlNode->getIntAttribute ("clockReferenceTime", Clock::ReferenceTime::CUMULATIVE));

            bool isOpen = xmlNode->getBoolAttribute ("isOpen");
            openState (isOpen);

            if (xmlNode->getBoolAttribute ("forceNewDirectory"))
                forceNewDirectoryButton->setToggleState (true, dontSendNotification);
            else
                forceNewDirectoryButton->setToggleState (false, dontSendNotification);
        }
        else if (xmlNode->hasTagName ("RECORDENGINES"))
        {
            for (int i = 0; i < recordEngines.size(); i++)
            {
                for (auto* xmlEngine : xmlNode->getChildWithTagNameIterator ("ENGINE"))
                {
                    if (xmlEngine->getStringAttribute ("id") == recordEngines[i]->getID())
                        recordEngines[i]->loadParametersFromXml (xmlEngine);
                }
            }
        }
    }

    if (! isConsoleApp)
        audioEditor->loadStateFromXml (xml);

    filenameConfigWindow->loadStateFromXml (xml);
    generateFilenameFromFields (true);
}

Array<String> ControlPanel::getRecentlyUsedFilenames()
{
    return filenameComponent->getRecentlyUsedFilenames().strings;
}

void ControlPanel::setRecentlyUsedFilenames (const Array<String>& filenames)
{
    StringArray validFilenames;
    for (const auto& filename : filenames)
    {
        if (File (filename).exists())
        {
            validFilenames.addIfNotAlreadyThere (filename);
        }
    }

    filenameComponent->setRecentlyUsedFilenames (validFilenames);
}

static void forceFilenameEditor (int result, ControlPanel* panel)
{
    CallOutBox& myBox = CallOutBox::launchAsynchronously (std::move (panel->filenameConfigWindow),
                                                          panel->filenameText->getScreenBounds(),
                                                          nullptr);
    myBox.addComponentListener (panel);
    myBox.setDismissalMouseClicksAreAlwaysConsumed (true);

    return;
}

String ControlPanel::getRecordingDirectoryName()
{
    return recordingDirectoryName;
}

void ControlPanel::createNewRecordingDirectory()
{
    //TODO: Remove dependency on button states/callbacks
    newDirectoryNeeded = true;
    MessageManager::callAsync ([this]
                               { newDirectoryButton->setToggleState (true, dontSendNotification); });
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

void ControlPanel::setRecordingDirectoryPrependText (String text)
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
                    String errString = field->validate (text);
                    if (errString.length())
                        return; //TODO: Notify user of error via HTTPServer
                    field->state = FilenameFieldComponent::State::CUSTOM;
                    field->value = text;
                }
                createNewRecordingDirectory();

                generateFilenameFromFields (true);
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

void ControlPanel::setRecordingDirectoryAppendText (String text)
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
                    String errString = field->validate (text);
                    if (errString.length())
                        return; //TODO: Notify user of error via HTTPServer
                    field->state = FilenameFieldComponent::State::CUSTOM;
                    field->value = text;
                }
                createNewRecordingDirectory();

                generateFilenameFromFields (true);
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

void ControlPanel::setRecordingDirectoryBaseText (String text)
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

                else if (text.length() > 0)
                {
                    String errString = field->validate (text);
                    if (errString.length())
                        return; //TODO: Notify user of error via HTTPServer
                    field->state = FilenameFieldComponent::State::CUSTOM;
                    field->value = text;
                }

                createNewRecordingDirectory();

                generateFilenameFromFields (true);
            }
        }
    }
}

String ControlPanel::generateFilenameFromFields (bool usePlaceholderText)
{
    String filename = "";

    for (auto& field : filenameFields) //loops in order through prepend, main, append
    {
        filename += field->getNextValue (usePlaceholderText);
    }

    MessageManager::callAsync ([this, filename]
                               { filenameText->setButtonText (filename); });

    return filename;
}

String ControlPanel::generateDatetimeFromFormat (String format)
{
    //TODO: Parse format and generate the proper date string
    //For now use default format: "YYYY-MM-DD_HH-MM-SS"

    //Generate current datetime in default format
    Time calendar = Time::getCurrentTime();

    Array<int> t;
    t.add (calendar.getYear());
    t.add (calendar.getMonth() + 1); // January = 0
    t.add (calendar.getDayOfMonth());
    t.add (calendar.getHours());
    t.add (calendar.getMinutes());
    t.add (calendar.getSeconds());

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
