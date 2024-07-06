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

#include "LfpDisplayEditor.h"

#define MS_FROM_START Time::highResolutionTicksToSeconds (Time::getHighResolutionTicks() - start) * 1000

using namespace LfpViewer;

LayoutButton::LayoutButton (const String& buttonName)
    : Button (buttonName)
{
    setClickingTogglesState (true);
}

LayoutButton::~LayoutButton()
{
}

void LayoutButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState())
        g.setColour (findColour (ThemeColours::highlightedFill));
    else
        g.setColour (findColour (ThemeColours::widgetBackground));

    g.fillRoundedRectangle (0, 0, getWidth(), getHeight(), 3);

    if (getToggleState())
        g.setColour (Colours::black.withAlpha (isMouseOver ? 0.6f : 1.0f));
    else
        g.setColour (findColour (ThemeColours::defaultText).withAlpha (isMouseOver ? 0.6f : 1.0f));

    juce::Path path;

    if (getName().equalsIgnoreCase ("single"))
    {
        g.drawRoundedRectangle (3, 3, getWidth() - 6, getHeight() - 6, 2, 1);
    }
    else if (getName().equalsIgnoreCase ("two-vertical"))
    {
        g.drawRoundedRectangle (3, 3, getWidth() - 6, getHeight() - 6, 2, 1);
        g.fillRect ((float) (getWidth() / 2) - 0.5f, 3.0f, 1.0f, (float) (getHeight() - 6));
    }
    else if (getName().equalsIgnoreCase ("three-vertical"))
    {
        g.drawRoundedRectangle (3, 3, getWidth() - 6, getHeight() - 6, 2, 1);
        g.fillRect ((float) (getWidth() / 3) + 1.0f, 3.0f, 1.0f, (float) (getHeight() - 6));
        g.fillRect ((float) (2 * getWidth() / 3) - 1.0f, 3.0f, 1.0f, (float) (getHeight() - 6));
    }
    else if (getName().equalsIgnoreCase ("two-horizontal"))
    {
        g.drawRoundedRectangle (3, 3, getWidth() - 6, getHeight() - 6, 2, 1);
        g.fillRect (3.0f, (float) (getHeight() / 2) - 0.5f, (float) (getWidth() - 6), 1.0f);
    }
    else
    {
        g.drawRoundedRectangle (3, 3, getWidth() - 6, getHeight() - 6, 2, 1);
        g.fillRect (3.0f, (float) (getHeight() / 3) + 1.0f, (float) (getWidth() - 6), 1.0f);
        g.fillRect (3.0f, (float) (2 * getHeight() / 3) - 1.0f, (float) (getWidth() - 6), 1.0f);
    }
}

LfpDisplayEditor::LfpDisplayEditor (GenericProcessor* parentNode)
    : VisualizerEditor (parentNode, "LFP", 195),
      hasNoInputs (true),
      signalChainIsLoading (true)
{
    lfpProcessor = (LfpDisplayNode*) parentNode;

    defaultSubprocessor = 0;

    layoutLabel = std::make_unique<Label> ("layout", "Layout:");
    //addAndMakeVisible(layoutLabel.get());

    singleDisplay = std::make_unique<LayoutButton> ("single");
    singleDisplay->setToggleState (true, dontSendNotification);
    singleDisplay->setRadioGroupId (201, dontSendNotification);
    singleDisplay->addListener (this);
    addAndMakeVisible (singleDisplay.get());
    selectedLayout = SplitLayouts::SINGLE;

    twoVertDisplay = std::make_unique<LayoutButton> ("two-vertical");
    twoVertDisplay->setToggleState (false, dontSendNotification);
    twoVertDisplay->setRadioGroupId (201, dontSendNotification);
    twoVertDisplay->addListener (this);
    addAndMakeVisible (twoVertDisplay.get());

    threeVertDisplay = std::make_unique<LayoutButton> ("three-vertical");
    threeVertDisplay->setToggleState (false, dontSendNotification);
    threeVertDisplay->setRadioGroupId (201, dontSendNotification);
    threeVertDisplay->addListener (this);
    addAndMakeVisible (threeVertDisplay.get());

    twoHoriDisplay = std::make_unique<LayoutButton> ("two-horizontal");
    twoHoriDisplay->setToggleState (false, dontSendNotification);
    twoHoriDisplay->setRadioGroupId (201, dontSendNotification);
    twoHoriDisplay->addListener (this);
    addAndMakeVisible (twoHoriDisplay.get());

    threeHoriDisplay = std::make_unique<LayoutButton> ("three-horizontal");
    threeHoriDisplay->setToggleState (false, dontSendNotification);
    threeHoriDisplay->setRadioGroupId (201, dontSendNotification);
    threeHoriDisplay->addListener (this);
    addAndMakeVisible (threeHoriDisplay.get());

    syncButton = std::make_unique<UtilityButton> ("SYNC DISPLAYS");
    syncButton->addListener (this);
    addAndMakeVisible (syncButton.get());
}

Visualizer* LfpDisplayEditor::createNewCanvas()
{
    return new LfpDisplayCanvas (lfpProcessor, selectedLayout, signalChainIsLoading);
}

void LfpDisplayEditor::initialize (bool signalChainIsLoading_)
{
    signalChainIsLoading = signalChainIsLoading_;
}

void LfpDisplayEditor::buttonClicked (Button* button)
{
    if (button == singleDisplay.get())
        selectedLayout = SplitLayouts::SINGLE;
    else if (button == twoVertDisplay.get())
        selectedLayout = SplitLayouts::TWO_VERT;
    else if (button == threeVertDisplay.get())
        selectedLayout = SplitLayouts::THREE_VERT;
    else if (button == twoHoriDisplay.get())
        selectedLayout = SplitLayouts::TWO_HORZ;
    else if (button == threeHoriDisplay.get())
        selectedLayout = SplitLayouts::THREE_HORZ;
    else if (button == syncButton.get())
    {
        if (canvas != nullptr)
        {
            LfpDisplayCanvas* c = (LfpDisplayCanvas*) canvas.get();
            c->syncDisplays();
        }
    }

    if (button->getRadioGroupId() == 201 && canvas != nullptr)
        static_cast<LfpDisplayCanvas*> (canvas.get())->setLayout (selectedLayout);
}

void LfpDisplayEditor::resized()
{
    VisualizerEditor::resized();

    int buttonSize = 25;

    //layoutLabel->setBounds(5, 40, 50, 20);
    singleDisplay->setBounds (22, 43, buttonSize, buttonSize);
    twoVertDisplay->setBounds (52, 43, buttonSize, buttonSize);
    threeVertDisplay->setBounds (82, 43, buttonSize, buttonSize);
    twoHoriDisplay->setBounds (112, 43, buttonSize, buttonSize);
    threeHoriDisplay->setBounds (142, 43, buttonSize, buttonSize);

    syncButton->setBounds (40, 84, 110, 30);
}

void LfpDisplayEditor::removeBufferForDisplay (int splitID)
{
    if (canvas != nullptr)
    {
        LfpDisplayCanvas* cv = (LfpDisplayCanvas*) canvas.get();

        cv->removeBufferForDisplay (splitID);
    }
}

void LfpDisplayEditor::saveVisualizerEditorParameters (XmlElement* xml)
{
    xml->setAttribute ("Type", "LfpDisplayEditor");

    XmlElement* values = xml->createNewChildElement ("VALUES");
    values->setAttribute ("SelectedLayout", static_cast<int> (selectedLayout));
}

void LfpDisplayEditor::loadVisualizerEditorParameters (XmlElement* xml)
{
    for (auto* xmlNode : xml->getChildIterator())
    {
        if (xmlNode->hasTagName ("VALUES"))
        {
            int64 start = Time::getHighResolutionTicks();

            selectedLayout = static_cast<SplitLayouts> (xmlNode->getIntAttribute ("SelectedLayout"));

            if (canvas != nullptr)
                static_cast<LfpDisplayCanvas*> (canvas.get())->setLayout (selectedLayout);

            if (selectedLayout == SplitLayouts::SINGLE)
                singleDisplay->setToggleState (true, dontSendNotification);
            else if (selectedLayout == SplitLayouts::TWO_VERT)
                twoVertDisplay->setToggleState (true, dontSendNotification);
            else if (selectedLayout == SplitLayouts::THREE_VERT)
                threeVertDisplay->setToggleState (true, dontSendNotification);
            else if (selectedLayout == SplitLayouts::TWO_HORZ)
                twoHoriDisplay->setToggleState (true, dontSendNotification);
            else
                threeHoriDisplay->setToggleState (true, dontSendNotification);

            LOGDD ("    Loaded layout in ", MS_FROM_START, " milliseconds");
        }
    }
}
