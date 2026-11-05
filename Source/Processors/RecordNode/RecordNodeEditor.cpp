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

#include "RecordNodeEditor.h"
#include "../../CoreServices.h"
#include "RecordNode.h"
#include "../Parameter/ParameterEditorOwner.h"
#include <stdio.h>

namespace
{
class EngineOptionsButton : public Button
{
public:
    EngineOptionsButton()
        : Button ("Engine options"),
          icon (Drawable::parseSVGPath (
              "M17.2994 10.4527L19.2267 10.7677C19.3846 10.7935 19.5003 10.9298 19.5 11.0896V12.883C19.5 13.0412 19.3865 13.1768 19.2303 13.2042L17.3004 13.543"
              "C17.1885 13.9298 17.0349 14.3022 16.8415 14.6543L17.9823 16.2382C18.0759 16.3679 18.0612 16.5463 17.9483 16.6595L16.6804 17.9283"
              "C16.5682 18.0401 16.3921 18.0561 16.2623 17.9645L14.6627 16.8424C14.3099 17.0387 13.9352 17.1952 13.5442 17.3103L13.2034 19.231"
              "C13.176 19.3865 13.0406 19.5 12.8825 19.5H11.0888C10.9294 19.5 10.7934 19.3849 10.7676 19.228L10.4493 17.3168"
              "C10.059 17.204 9.6823 17.0485 9.32585 16.8525L7.73767 17.9648C7.60821 18.0558 7.43178 18.0401 7.31992 17.9283L6.05198 16.6595"
              "C5.93947 16.5463 5.9248 16.3686 6.01741 16.2391L7.13958 14.6697C6.94163 14.3116 6.78444 13.9337 6.67062 13.5414L4.76905 13.2042"
              "C4.61349 13.1765 4.5 13.0412 4.5 12.883V11.0896C4.5 10.9304 4.61544 10.7941 4.77263 10.768L6.67421 10.4514"
              "C6.78868 10.0582 6.94586 9.68022 7.14316 9.32315L6.0347 7.73739C5.94371 7.60793 5.95937 7.43185 6.07122 7.32L7.33883 6.0525"
              "C7.452 5.94 7.62908 5.925 7.7592 6.01793L9.33433 7.14293C9.68817 6.94924 10.0639 6.795 10.4552 6.6825L10.767 4.77359"
              "C10.7927 4.61576 10.929 4.5 11.0888 4.5H12.8825C13.041 4.5 13.1763 4.61413 13.2037 4.77L13.5399 6.68935"
              "C13.929 6.8025 14.304 6.95837 14.6591 7.15467L16.2385 6.01957C16.3683 5.92598 16.5464 5.94065 16.6595 6.05348L17.9278 7.32098"
              "C18.0397 7.43315 18.0553 7.60957 17.9643 7.73902L16.8392 9.34239C17.0323 9.69424 17.1865 10.066 17.2994 10.4527Z"
              "M9.71725 12C9.71725 13.2607 10.7393 14.2826 12.0001 14.2826C13.2608 14.2826 14.2829 13.2607 14.2829 12"
              "C14.2829 10.7394 13.2608 9.71742 12.0001 9.71742C10.7393 9.71742 9.71725 10.7394 9.71725 12Z"))
    {
    }

    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        const auto colour = findColour (isMouseOverButton ? ThemeColours::highlightedFill : ThemeColours::defaultText);
        g.setColour (colour.withMultipliedAlpha (! isEnabled() ? 0.4f : isButtonDown ? 0.7f : 1.0f));
        g.fillPath (icon, icon.getTransformToScaleToFit (getLocalBounds().toFloat().reduced (2.0f), true));
    }

private:
    Path icon;
};

void commitEngineEditors (Component& component)
{
    if (auto* label = dynamic_cast<Label*> (&component))
        label->hideEditor (false);
    else
        for (auto* child : component.getChildren())
            commitEngineEditors (*child);
}

class EngineOptionsContent : public Component, public ParameterEditorOwner
{
public:
    explicit EngineOptionsContent (const Array<Parameter*>& parameters)
        : ParameterEditorOwner (this)
    {
        for (auto* parameter : parameters)
        {
            ParameterEditor* editor;
            if (parameter->getType() == Parameter::BOOLEAN_PARAM)
                editor = new ToggleParameterEditor (parameter);
            else
                editor = new TextBoxParameterEditor (parameter);
            editor->setLayout (ParameterEditor::Layout::nameOnLeft);
            editor->getLabel()->setFont (FontOptions ("Inter", "Regular", 14.0f));
            editor->getLabel()->setBorderSize (BorderSize<int>());
            editor->getLabel()->setTooltip (parameter->getDescription());
            if (parameter->getType() == Parameter::STRING_PARAM)
                static_cast<Label*> (editor->getEditor())->setJustificationType (Justification::centredLeft);
            addParameterEditor (editor, 0, 0);
        }
        setSize (360, padding * 2 + parameters.size() * rowHeight + jmax (0, parameters.size() - 1) * rowGap);
    }

    void resized() override
    {
        int y = padding;
        const int width = jmax (1, getWidth() - padding * 2);
        const int labelWidth = jmax (1, (width - columnGap) / 2);
        const int controlX = labelWidth + columnGap;
        const int availableControlWidth = jmax (1, width - controlX);
        for (auto* editor : getParameterEditors())
        {
            editor->setBounds (padding, y, width, rowHeight);
            editor->getLabel()->setBounds (0, 0, labelWidth, rowHeight);
            editor->getLabel()->setJustificationType (Justification::centredLeft);
            const int controlWidth = dynamic_cast<ToggleParameterEditor*> (editor) != nullptr ? jmin (64, availableControlWidth) : availableControlWidth;
            editor->getEditor()->setBounds (controlX, 0, controlWidth, rowHeight);
            y += rowHeight + rowGap;
        }
    }

    void paint (Graphics& g) override
    {
        // g.fillAll (findColour (ThemeColours::componentBackground));
        g.setColour (findColour (ThemeColours::componentBackground));
        const auto editors = getParameterEditors();
        for (int i = 0; i < editors.size(); ++i)
            g.fillRect (editors[i]->getBounds().expanded (padding / 2));
    }

private:
    static constexpr int padding = 12;
    static constexpr int rowHeight = 24;
    static constexpr int rowGap = 20;
    static constexpr int columnGap = 12;
};

class EngineOptionsViewport : public Viewport
{
public:
    explicit EngineOptionsViewport (const Array<Parameter*>& parameters)
    {
        auto content = std::make_unique<EngineOptionsContent> (parameters);
        setScrollBarsShown (true, false);
        setSize (content->getWidth(), content->getHeight());
        setViewedComponent (content.release(), true);
    }

    ~EngineOptionsViewport() override
    {
        commitEngineEditors (*this);
    }

    void visibleAreaChanged (const Rectangle<int>&) override
    {
        if (auto* content = getViewedComponent())
            content->setSize (getMaximumVisibleWidth(), content->getHeight());
    }
};
}

void RecordNode::showEngineOptions (Component* anchor)
{
    if (CoreServices::getAcquisitionStatus() || recordThread->isThreadRunning())
    {
        CoreServices::sendStatusMessage ("Cannot configure recording engine options during acquisition.");
        return;
    }
    if (getEngineParameters().isEmpty())
    {
        CoreServices::sendStatusMessage ("The selected recording engine has no options.");
        return;
    }
    closeEngineOptions();
    if (anchor == nullptr)
        anchor = editor.get();
    const auto area = anchor != nullptr
                          ? anchor->getScreenBounds()
                          : Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.withSizeKeepingCentre (1, 1);
    auto content = std::make_unique<EngineOptionsViewport> (getEngineParameters());
    engineOptionsViewport = content.get();
    auto& popup = CallOutBox::launchAsynchronously (std::move (content), area, nullptr);
    const auto screen = Desktop::getInstance().getDisplays().getDisplayForRect (area)->userArea;
    const int border = popup.getBorderSize() * 2;
    engineOptionsViewport->setSize (jmin (engineOptionsViewport->getWidth(), jmax (1, screen.getWidth() - border)),
                                   jmin (engineOptionsViewport->getHeight(), jmax (1, screen.getHeight() - border)));
    popup.setName ("Record Node " + String (getNodeId()) + " - " + getEngineId() + " options");
    popup.setDismissalMouseClicksAreAlwaysConsumed (true);
    engineOptionsPopup = &popup;
}

void RecordNode::commitEngineOptions()
{
    if (engineOptionsViewport != nullptr)
        commitEngineEditors (*engineOptionsViewport);
}

void RecordNode::closeEngineOptions()
{
    commitEngineOptions();
    dismissEngineOptions();
}

void RecordNode::dismissEngineOptions()
{
    // Release parameter editors before a node is deleted or its settings are restored.
    if (engineOptionsViewport != nullptr)
        engineOptionsViewport->setViewedComponent (nullptr);
    if (engineOptionsPopup != nullptr)
    {
        engineOptionsPopup->exitModalState (0);
        engineOptionsPopup->setVisible (false);
    }
    engineOptionsViewport = nullptr;
    engineOptionsPopup = nullptr;
}

SyncMonitor::SyncMonitor()
{
    setInterceptsMouseClicks (false, false);
}

SyncMonitor::~SyncMonitor()
{
}

void SyncMonitor::setSyncMetric (SyncStatus syncStatus_, float syncMetric_)
{
    syncStatus = syncStatus_;
    metric = syncMetric_;

    if(syncStatus == SyncStatus::SYNCED || syncStatus == SyncStatus::HARDWARE_SYNCED || syncStatus == SyncStatus::HARP_CLOCK)
        isSynchronized = true;
    else
        isSynchronized = false;

    if(syncStatus == SyncStatus::HARP_CLOCK)
        timeUnits = " s";
    else
        timeUnits = " ms";
}

void SyncMonitor::setEnabled (bool state)
{
    isEnabled = state;

    repaint();
}

void LastSyncEventMonitor::paint (Graphics& g)
{
    g.setFont (FontOptions ("Fira Sans", "SemiBold", 12));

    if (isSynchronized && metric != -1.0)
    {
        if (metric <= 60)
        {
            g.setColour (Colours::green);
            g.drawText ("<1 min", 0, 0, 55, 20, Justification::centred);
        }

        else if (metric > 60 && metric < 60 * 5)
        {
            g.setColour (Colours::orange);
            g.drawText (">1 min", 0, 0, 55, 20, Justification::centred);
        }

        else
        {
            g.setColour (Colours::red);
            g.drawText (">5 min", 0, 0, 55, 20, Justification::centred);
        }
    }
    else
    {
        g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.5f));
        g.drawText ("--", 0, 0, 55, 20, Justification::centred);
    }
}

void SyncStartTimeMonitor::paint (Graphics& g)
{
    g.setFont (FontOptions ("Fira Sans", "SemiBold", 12));

    if (isSynchronized)
    {
        g.setColour (findColour (ThemeColours::defaultText));
        g.drawText (String (metric, 2) + timeUnits, 0, 0, 50, 20, Justification::centred);
    }

    else
    {
        g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.5f));
        g.drawText ("--", 0, 0, 50, 20, Justification::centred);
    }
}

void SyncAccuracyMonitor::paint (Graphics& g)
{
    g.setFont (FontOptions ("Fira Sans", "SemiBold", 12));

    if (isSynchronized)
    {
        g.setColour (findColour (ThemeColours::defaultText));
        g.drawText (String (std::abs (metric), 3) + " ms", 0, 0, 55, 20, Justification::centred);
    }

    else
    {
        g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.5f));
        g.drawText ("--", 0, 0, 55, 20, Justification::centred);
    }
}

StreamMonitor::StreamMonitor (RecordNode* rn, uint64 id)
    : LevelMonitor (rn),
      streamId (id)
{
    selectedChannels = 0;
    totalChannels = rn->getDataStream (streamId)->getChannelCount();

    startTimerHz (5);
}

StreamMonitor::~StreamMonitor() {}

void StreamMonitor::timerCallback()
{
    if (((RecordNode*) processor)->recordThread->isThreadRunning())
    {
        float fifoUsage = ((RecordNode*) processor)->fifoUsage[streamId];

        // Scale the fill percentage to reduce noise from expected low values:
        // - Values <= 5% are displayed as 0%
        // - Values between 5% and 10% are scaled to 0-10%
        // - Values above 10% are displayed linearly
        float scaledUsage;
        if (fifoUsage <= 0.05f)
            scaledUsage = 0.0f;
        else if (fifoUsage <= 0.10f)
            scaledUsage = (fifoUsage - 0.05f) * 2.0f;  // Maps 0.05-0.10 to 0.0-0.10
        else
            scaledUsage = fifoUsage;

        setFillPercentage (scaledUsage);
    }
    else
        setFillPercentage (0.0);
}

void StreamMonitor::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    // Draw the monitor
    LevelMonitor::paintButton (g, isMouseOver, isButtonDown);

    // Draw the channel count text
    auto textArea = getLocalBounds().reduced (2);

    auto t = AffineTransform::rotation (-MathConstants<float>::halfPi,
                                        textArea.toFloat().getCentreX(),
                                        textArea.toFloat().getCentreY());

    auto newTextArea = textArea.toFloat().transformedBy (t);

    g.addTransform (t);

    if (selectedChannels == 0)
        g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.5f));
    else
        g.setColour (findColour (ThemeColours::defaultText));

    g.setFont (FontOptions ("Inter", "Regular", 12.0f));
    g.drawText (String (selectedChannels) + "/" + String (totalChannels), newTextArea, Justification::centred);
}

void StreamMonitor::updateChannelCount (int selected)
{
    selectedChannels = selected;
    repaint();
}

DiskMonitor::DiskMonitor (RecordNode* rn)
    : LevelMonitor (rn),
      lastFreeSpace (0.0),
      recordingTimeLeftInSeconds (0),
      dataRate (0.0)
{
    rn->getDiskSpaceChecker()->addListener (this);
    startTimerHz (1);
}

DiskMonitor::~DiskMonitor()
{
    if (((RecordNode*) processor)->getDiskSpaceChecker() != nullptr)
        ((RecordNode*) processor)->getDiskSpaceChecker()->removeListener (this);
}

void DiskMonitor::update (float dataRate, int64 bytesFree, float timeLeft)
{
    if (((RecordNode*) processor)->recordThread->isThreadRunning())
    {
        String msg = String (bytesFree / pow (2, 30)) + " GB available\n";
        msg += String (int (timeLeft / 60.0f)) + " minutes remaining\n";
        msg += "Data rate: " + String (dataRate * 1000 / pow (2, 20), 2) + " MB/s";
        setTooltip (msg);
    }
    else
    {
        setTooltip (String (bytesFree / pow (2, 30)) + " GB available");
    }
}

void DiskMonitor::updateDiskSpace (float percentage)
{
    setFillPercentage (1.0f - percentage);
}

void DiskMonitor::directoryInvalid (bool recordingStopped)
{
    setFillPercentage (1.0);
    setTooltip ("Invalid directory");

    if (recordingStopped)
    {
        String msg = "Record Node (" + String (processor->getNodeId()) + ") - The selected recording path doesn't exist anymore.";
        msg += "\n\nPlease select a valid directory and restart the recording.";
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Recording Stopped", msg);
    }
}

void DiskMonitor::lowDiskSpace()
{
    String msg = "Recording stopped. Less than 5 minutes of disk space remaining.";
    AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "WARNING", msg);
}

void DiskMonitor::timerCallback()
{
    repaint();
}

RecordChannelsParameterEditor::RecordChannelsParameterEditor (RecordNode* rn, Parameter* param, int rowHeightPixels, int rowWidthPixels)
    : ParameterEditor (param),
      recordNode (rn)
{
    int numChannels = int (((MaskChannelsParameter*) param)->getChannelStates().size());
    int selected = 0;
    for (auto chan : ((MaskChannelsParameter*) param)->getChannelStates())
        if (chan)
            selected++;

    uint64 streamId = ((DataStream*) param->getOwner())->getStreamId();
    String streamName = ((DataStream*) param->getOwner())->getName();
    String sourceNodeId = String (((DataStream*) param->getOwner())->getSourceNodeId());

    monitor = std::make_unique<StreamMonitor> (recordNode, streamId);
    monitor->setTooltip (sourceNodeId + " | " + streamName);
    monitor->setComponentID (sourceNodeId + " | " + streamName);
    monitor->addListener (this);
    monitor->setBounds (0, 0, 15, 73);
    addAndMakeVisible (monitor.get());

    setBounds (0, 0, 15, 73);

    editor = monitor.get();

    updateView();
};

void RecordChannelsParameterEditor::channelStateChanged (Array<int> newChannels)
{
    Array<var> newArray;

    for (int i = 0; i < newChannels.size(); i++)
        newArray.add (newChannels[i]);

    param->setNextValue (newArray);

    updateView();
}

int RecordChannelsParameterEditor::getChannelCount()
{
    return ((MaskChannelsParameter*) param)->getChannelCount();
}

Array<int> RecordChannelsParameterEditor::getSelectedChannels()
{
    MaskChannelsParameter* p = (MaskChannelsParameter*) param;
    std::vector<bool> channelStates = p->getChannelStates();

    Array<int> selectedChannels;

    for (int i = 0; i < channelStates.size(); i++)
        if (channelStates[i])
            selectedChannels.add (i);

    return selectedChannels;
}

void RecordChannelsParameterEditor::buttonClicked (Button* label)
{
    if (param == nullptr)
        return;

    MaskChannelsParameter* p = (MaskChannelsParameter*) param;

    Array<String> channelNames;
    String popupTitle;

    if (p->getOwner()->getType() == ParameterOwner::DATASTREAM)
    {
        DataStream* stream = (DataStream*) p->getOwner();
        popupTitle = String (stream->getSourceNodeId()) + " " + stream->getSourceNodeName() + " - " + stream->getName();

        for (auto channel : stream->getContinuousChannels())
            channelNames.add (channel->getName());
    }

    std::vector<bool> channelStates = p->getChannelStates();

    auto* channelSelector = new PopupChannelSelector (label, this, channelStates, channelNames, popupTitle);

    channelSelector->setChannelButtonColour (param->getColour());

    CoreServices::getPopupManager()->showPopup (std::unique_ptr<PopupComponent> (channelSelector), monitor.get());
}

void RecordChannelsParameterEditor::updateView()
{
    /* Update channel count in StreamMonitor */

    if (param != nullptr)
    {
        monitor->updateChannelCount (param->currentValue.getArray()->size());
    }
}

void RecordChannelsParameterEditor::resized()
{
    updateBounds();
}

RecordToggleButton::RecordToggleButton (const String& name) : CustomToggleButton() {}

RecordToggleButton::~RecordToggleButton() {}

void RecordToggleButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    float alpha = 1.0f;

    if (! isEnabled())
    {
        alpha = 0.5f;
    }

    g.setColour (Colour (0, 0, 0).withAlpha (alpha));
    g.fillRoundedRectangle (0, 0, getWidth(), getHeight(), 4);

    if (! getToggleState())
    {
        g.setColour (findColour (ThemeColours::widgetBackground).withAlpha (alpha));
    }
    else
    {
        g.setColour (Colour (255, 0, 0).withAlpha (alpha));
    }

    g.fillRoundedRectangle (1, 1, getWidth() - 2, getHeight() - 2, 3);

    g.setColour (Colour (0, 0, 0).withAlpha (alpha));
    g.fillEllipse (0.35 * getWidth(), 0.35 * getHeight(), 0.3 * getWidth(), 0.3 * getHeight());
}

RecordToggleParameterEditor::RecordToggleParameterEditor (Parameter* param) : ParameterEditor (param)
{
    label = std::make_unique<Label> ("Parameter name", param->getDisplayName());
    label->setFont (FontOptions ("Inter", "Regular", 13.0f));
    addAndMakeVisible (label.get());

    toggleButton = std::make_unique<RecordToggleButton> (param->getDisplayName()); //param->getDisplayName());
    toggleButton->setClickingTogglesState (true);
    toggleButton->setTooltip ("Toggle recording state on/off");
    toggleButton->addListener (this);
    toggleButton->setToggleState (true, dontSendNotification);
    toggleButton->setBounds (50, 0, 15, 15);
    addAndMakeVisible (toggleButton.get());

    label->setBounds (0, 0, 100, 15);
    toggleButton->setBounds (95, 0, 15, 15);

    setBounds (0, 0, 150, 20);

    editor = toggleButton.get();
}

void RecordToggleParameterEditor::buttonClicked (Button*)
{
    param->setNextValue (toggleButton->getToggleState());
}

void RecordToggleParameterEditor::updateView()
{
    if (param != nullptr)
        toggleButton->setToggleState (param->getValue(), dontSendNotification);
}

void RecordToggleParameterEditor::resized()
{
}

void ClearButton::paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    g.fillAll (findColour (ThemeColours::widgetBackground).contrasting (0.05f));

    int xoffset = (getWidth() - 14) / 2;
    int yoffset = (getHeight() - 14) / 2;

    if (isMouseOverButton)
    {
        g.setColour (findColour (ThemeColours::defaultFill).withAlpha (0.5f));
        g.fillRoundedRectangle (xoffset, yoffset, 14, 14, 4.0f);
    }

    g.setColour (findColour (ThemeColours::defaultText));

    Path path;
    path.addLineSegment (Line<float> (2, 2, 8, 8), 0.0f);
    path.addLineSegment (Line<float> (2, 8, 8, 2), 0.0f);

    path.applyTransform (AffineTransform::translation (xoffset + 2, yoffset + 2));

    g.strokePath (path, PathStrokeType (1.0f));
}

RecordPathParameterEditor::RecordPathParameterEditor (Parameter* param, int rowHeightPixels, int rowWidthPixels) : ParameterEditor (param)
{
    jassert (param->getType() == Parameter::PATH_PARAM);

    setBounds (0, 0, rowWidthPixels, rowHeightPixels);

    button = std::make_unique<TextButton> ("Browse");
    button->setName (param->getKey());
    button->addListener (this);
    button->setClickingTogglesState (false);
    button->setTooltip (param->getValueAsString());
    button->addMouseListener (this, true);
    addAndMakeVisible (button.get());

    label = std::make_unique<Label> ("Parameter name", param->getDisplayName()); // == "" ? param->getName().replace("_", " ") : param->getDisplayName());
    Font labelFont = FontOptions ("Inter", "Regular", int (0.75 * rowHeightPixels));
    label->setFont (labelFont);
    label->setJustificationType (Justification::left);
    addAndMakeVisible (label.get());

    clearButton = std::make_unique<ClearButton>();
    clearButton->addListener (this);
    clearButton->addMouseListener (this, true);
    clearButton->setTooltip ("Set to default");
    addChildComponent (clearButton.get());

    int width = rowWidthPixels;

    label->setBounds (width / 2 + 4, 0, width / 2, rowHeightPixels);
    button->setBounds (0, 0, width / 2, rowHeightPixels);
    clearButton->setBounds ((width) / 2 - rowHeightPixels - 1, 1, rowHeightPixels - 2, rowHeightPixels - 2);

    editor = (Component*) button.get();
}

void RecordPathParameterEditor::buttonClicked (Button* button_)
{
    if (button_ == button.get())
    {
        bool isDirectory = ((PathParameter*) param)->getIsDirectory();
        String dialogBoxTitle;
        String validFilePatterns;

        if (param->getDescription().isEmpty())
            dialogBoxTitle = "Select a " + String (isDirectory ? "directory" : "file");
        else
            dialogBoxTitle = param->getDescription();

        if (! isDirectory)
            validFilePatterns = "*." + ((PathParameter*) param)->getValidFilePatterns().joinIntoString (";*.");

        FileChooser chooser (dialogBoxTitle, File(), validFilePatterns);

        bool success = isDirectory ? chooser.browseForDirectory() : chooser.browseForFileToOpen();
        if (success)
        {
            File file = chooser.getResult();
            param->setNextValue (file.getFullPathName());
            updateView();
        }
    }
    else if (button_ == clearButton.get())
    {
        param->setNextValue ("None");
        clearButton->setVisible (false);
        updateView();
    }
}

void RecordPathParameterEditor::updateView()
{
    if (param == nullptr)
        button->setEnabled (false);
    else
        button->setEnabled (true);

    if (param)
    {
        String value = param->getValueAsString();
        button->setButtonText (value);
        if (! ((PathParameter*) param)->isValid())
        {
            button->setColour (TextButton::textColourOnId, Colours::red);
            button->setColour (TextButton::textColourOffId, Colours::red);
        }
        else
        {
            // Remove the red colour if it was previously set
            button->removeColour (TextButton::textColourOnId);
            button->removeColour (TextButton::textColourOffId);
        }
        //Alternatively:
        //button->setButtonText(File(param->getValueAsString()).getFileName());
        if (value == "None")
        {
            button->setButtonText ("default");
            button->setTooltip ("Override default path");
        }
        else
        {
            button->setTooltip (value);
        }
    }
}

void RecordPathParameterEditor::resized()
{
    updateBounds();

    if (button != nullptr)
    {
        clearButton->setBounds (button->getRight() - button->getHeight(),
                                button->getY() + 1,
                                button->getHeight() - 2,
                                button->getHeight() - 2);
    }
}

void RecordPathParameterEditor::mouseEnter (const MouseEvent& event)
{
    if (param->getValueAsString() != "None" && button->isEnabled())
    {
        clearButton->setVisible (true);
    }
}

void RecordPathParameterEditor::mouseExit (const MouseEvent& event)
{
    clearButton->setVisible (false);
}

RecordNodeEditor::RecordNodeEditor (RecordNode* parentNode)
    : GenericEditor (parentNode)
{
    desiredWidth = 165;

    recordNode = parentNode;

    fifoDrawerButton = std::make_unique<FifoDrawerButton> (getNameAndId() + " Fifo Drawer Button");
    fifoDrawerButton->setBounds (4, 40, 10, 78);
    fifoDrawerButton->addListener (this);
    addAndMakeVisible (fifoDrawerButton.get());

    diskMonitor = std::make_unique<DiskMonitor> (recordNode);
    diskMonitor->setBounds (18, 33, 15, 92);
    addAndMakeVisible (diskMonitor.get());

    addCustomParameterEditor (new RecordPathParameterEditor (parentNode->getParameter ("directory")), 42, 32);
    addComboBoxParameterEditor (Parameter::PROCESSOR_SCOPE, "engine", 42, 57);

    for (auto& p : { "directory", "engine" })
    {
        auto* ed = getParameterEditor (p);
        ed->setLayout (ParameterEditor::Layout::nameHidden);
        ed->setBounds (ed->getX(), ed->getY(), 110, ed->getHeight());
    }
    getParameterEditor ("engine")->setSize (90, getParameterEditor ("engine")->getHeight());
    engineOptionsButton = std::make_unique<EngineOptionsButton>();
    engineOptionsButton->setBounds (135, 57, 20, 18);
    engineOptionsButton->setTooltip ("Configure this Record Node's engine options");
    engineOptionsButton->addListener (this);
    addAndMakeVisible (engineOptionsButton.get());
    updateEngineOptions();

    addCustomParameterEditor (new RecordToggleParameterEditor (parentNode->getParameter ("events")), 40, 85);
    addCustomParameterEditor (new RecordToggleParameterEditor (parentNode->getParameter ("spikes")), 40, 107);

    // Show the fifo monitors by default
    fifoDrawerButton->triggerClick();
}

void RecordNodeEditor::timerCallback()
{
    fifoDrawerButton->triggerClick();
    stopTimer();
}

void RecordNodeEditor::updateSyncMonitors()
{
    for (auto stream : getProcessor()->getDataStreams())
    {
        syncStartTimeMonitors[stream->getStreamId()] = streamSelector->getSyncStartTimeMonitor (stream);
        syncAccuracyMonitors[stream->getStreamId()] = streamSelector->getSyncAccuracyMonitor (stream);
        lastSyncEventMonitors[stream->getStreamId()] = streamSelector->getlastSyncEventMonitor (stream);
    }

    if (recordNode != nullptr)
        recordNode->updateSyncMonitors();
}

void RecordNodeEditor::updateFifoMonitors()
{
    auto removeExistingMonitors = [this] (std::vector<ParameterEditor*>& monitors)
    {
        for (auto& monitor : monitors)
        {
            for (int i = 0; i < this->parameterEditors.size(); ++i)
            {
                if (this->parameterEditors[i] == monitor)
                {
                    removeChildComponent (monitor);
                    parameterEditors.removeObject (monitor);
                    break;
                }
            }
        }
        monitors.clear();
    };

    removeExistingMonitors (streamMonitors);
    removeExistingMonitors (syncMonitors);

    int streamCount = 0;
    for (auto& stream : recordNode->getDataStreams())
    {
        uint64 streamId = stream->getStreamId();

        // Add a recording channel selector for each stream
        Parameter* channels = recordNode->getDataStream (streamId)->getParameter ("channels");
        recordNode->getDataStream (streamId)->setColour ("channels", getLookAndFeel().findColour (ProcessorColour::IDs::RECORD_COLOUR));
        addCustomParameterEditor (new RecordChannelsParameterEditor (recordNode, channels), 18 + streamCount * 20, 32);
        parameterEditors.getLast()->setVisible (! getCollapsedState());
        parameterEditors.getLast()->disableUpdateOnSelectedStreamChanged();
        streamMonitors.push_back (parameterEditors.getLast());

        // Add a sync line selector for each stream
        Parameter* syncLineParam = recordNode->getDataStream (streamId)->getParameter ("sync_line");
        Parameter* mainSyncParam = recordNode->getParameter ("main_sync");
        addSyncLineParameterEditor ((TtlLineParameter*) syncLineParam, (SelectedStreamParameter*) mainSyncParam, 18 + streamCount * 20, 110);
        parameterEditors.getLast()->setVisible (! getCollapsedState());
        parameterEditors.getLast()->disableUpdateOnSelectedStreamChanged();
        syncMonitors.push_back (parameterEditors.getLast());

        streamCount++;
    }
}

void RecordNodeEditor::collapsedStateChanged()
{
    if (getCollapsedState())
    {
        for (auto monitor : streamMonitors)
            monitor->setVisible (false);
        for (auto monitor : syncMonitors)
            monitor->setVisible (false);
    }
    else
    {
        for (auto monitor : streamMonitors)
            monitor->setVisible (fifoDrawerButton.get()->getToggleState());
        for (auto monitor : syncMonitors)
            monitor->setVisible (fifoDrawerButton.get()->getToggleState());
    }
}

void RecordNodeEditor::updateSettings()
{
    updateEngineOptions();
    updateSyncMonitors();
    updateFifoMonitors();
    showFifoMonitors (fifoDrawerButton->getToggleState());
}

void RecordNodeEditor::updateEngineOptions()
{
    engineOptionsButton->setEnabled (! CoreServices::getAcquisitionStatus() && ! recordNode->getEngineParameters().isEmpty());
}

void RecordNodeEditor::startAcquisition()
{
    recordNode->closeEngineOptions();
    engineOptionsButton->setEnabled (false);
}

void RecordNodeEditor::stopAcquisition()
{
    engineOptionsButton->setEnabled (! recordNode->getEngineParameters().isEmpty());
}

void RecordNodeEditor::buttonClicked (Button* button)
{
    if (button == fifoDrawerButton.get())
    {
        showFifoMonitors (button->getToggleState());
    }
    else if (button == engineOptionsButton.get())
    {
        recordNode->showEngineOptions (button);
    }
}

void RecordNodeEditor::setStreamStartTime (uint16 streamId, SyncStatus status, float offsetMs)
{
    if (syncStartTimeMonitors.find (streamId) != syncStartTimeMonitors.end())
    {
        if (syncStartTimeMonitors[streamId] != nullptr)
            syncStartTimeMonitors[streamId]->setSyncMetric (status, offsetMs);
    }
}

void RecordNodeEditor::setLastSyncEvent (uint16 streamId, SyncStatus status, float syncTimeSeconds)
{
    if (lastSyncEventMonitors.find (streamId) != lastSyncEventMonitors.end())
    {
        if (lastSyncEventMonitors[streamId] != nullptr)
            lastSyncEventMonitors[streamId]->setSyncMetric (status, syncTimeSeconds);
    }
}

void RecordNodeEditor::setSyncAccuracy (uint16 streamId, SyncStatus status, float syncAccuracy)
{
    if (syncAccuracyMonitors.find (streamId) != syncAccuracyMonitors.end())
    {
        if (syncAccuracyMonitors[streamId] != nullptr)
            syncAccuracyMonitors[streamId]->setSyncMetric (status, syncAccuracy);
    }
}

void RecordNodeEditor::showFifoMonitors (bool show)
{
    int dX = 20 * (recordNode->getNumDataStreams() + 1);
    dX = show ? dX : 0;

    fifoDrawerButton->setBounds (
        4 + dX, fifoDrawerButton->getY(), fifoDrawerButton->getWidth(), fifoDrawerButton->getHeight());

    diskMonitor->setBounds (
        18 + dX, diskMonitor->getY(), diskMonitor->getWidth(), diskMonitor->getHeight());

    for (auto& p : { "directory", "engine", "events", "spikes" })
    {
        auto* ed = getParameterEditor (p);
        ed->setBounds (42 + dX, ed->getY(), ed->getWidth(), ed->getHeight());
    }

    desiredWidth = 165 + dX;
    engineOptionsButton->setTopLeftPosition (135 + dX, 57);

    if (getCollapsedState())
        return;

    for (auto& monitor : streamMonitors)
        monitor->setVisible (show);
    for (auto monitor : syncMonitors)
        monitor->setVisible (show);

    CoreServices::highlightEditor (this);
    deselect();
}

void FifoDrawerButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    g.setColour (Colour (110, 110, 110));
    if (isMouseOver)
        g.setColour (Colour (210, 210, 210));

    g.drawVerticalLine (5, 0.0f, getHeight());
}
