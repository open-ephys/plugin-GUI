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

#include "LfpDisplayCanvas.h"

#include "ColourSchemes/ChannelColourScheme.h"
#include "DisplayBuffer.h"
#include "LfpChannelDisplayInfo.h"
#include "LfpDisplayNode.h"
#include "ShowHideOptionsButton.h"

#include <math.h>

#define MS_FROM_START Time::highResolutionTicksToSeconds (Time::getHighResolutionTicks() - start) * 1000

using namespace LfpViewer;

LfpDisplayCanvas::LfpDisplayCanvas (LfpDisplayNode* processor_, SplitLayouts sl, bool isLoading_) : Visualizer (processor_),
                                                                                                    processor (processor_),
                                                                                                    selectedLayout (sl),
                                                                                                    isLoading (isLoading_)
{
    LOGD ("Creating LfpDisplayCanvas");

    int64 start = Time::getHighResolutionTicks();
    if (! processor->getHeadlessMode())
    {
        juce::TopLevelWindow::getTopLevelWindow (0)->addKeyListener (this);
    }
    optionsDrawerIsOpen = false;

    Array<DisplayBuffer*> displayBuffers = processor->getDisplayBuffers();
    Array<LfpDisplaySplitter*> splits;

    for (int i = 0; i < 3; i++) // create 3 split displays
    {
        displaySplits.add (new LfpDisplaySplitter (processor, this, displayBuffers[0], i));
        addChildComponent (displaySplits[i]);
        splits.add (displaySplits[i]);

        addChildComponent (displaySplits[i]->options.get());
        displaySplits[i]->options->setAlwaysOnTop (true);

        if (i == 0)
            displaySplits[i]->options->setVisible (true);
    }

    LOGD ("    Created split displays in ", MS_FROM_START, " milliseconds");

    processor->setSplitDisplays (splits);

    doubleVerticalSplitRatio = 0.5f;
    doubleHorizontalSplitRatio = 0.5f;

    tripleHorizontalSplitRatio.add (0.33f);
    tripleHorizontalSplitRatio.add (0.66f);

    tripleVerticalSplitRatio.add (0.33f);
    tripleVerticalSplitRatio.add (0.66f);

    addMouseListener (this, true);

    borderToDrag = -1;

    resized();

    //LOGD("    Finished in ", MS_FROM_START, " milliseconds");
}

LfpDisplayCanvas::~LfpDisplayCanvas()
{
    if (! processor->getHeadlessMode())
    {
        juce::TopLevelWindow::getTopLevelWindow (0)->removeKeyListener (this);
    }
}

void LfpDisplayCanvas::resized()
{
    if (isLoading)
        return;

    int borderSize = 5;

    if (selectedLayout == SINGLE)
    {
        displaySplits[0]->setVisible (true);

        displaySplits[0]->deselect(); // to remove boundary
        displaySplits[0]->options->setVisible (true);
        displaySplits[1]->options->setVisible (false);
        displaySplits[2]->options->setVisible (false);

        displaySplits[1]->setVisible (false);
        displaySplits[1]->setBounds (0, 0, 0, 0);
        displaySplits[2]->setVisible (false);
        displaySplits[2]->setBounds (0, 0, 0, 0);

        displaySplits[0]->setBounds (0, 0, getWidth(), getHeight());
    }
    else if (selectedLayout == TWO_VERT)
    {
        displaySplits[0]->setVisible (true);
        displaySplits[1]->setVisible (true);

        displaySplits[2]->setVisible (false);
        displaySplits[2]->setBounds (0, 0, 0, 0);

        displaySplits[0]->setBounds (0,
                                     0,
                                     getWidth() * doubleVerticalSplitRatio - borderSize,
                                     getHeight());

        displaySplits[0]->refreshScreenBuffer();

        displaySplits[1]->setBounds (getWidth() * doubleVerticalSplitRatio + borderSize,
                                     0,
                                     getWidth() * (1 - doubleVerticalSplitRatio) - borderSize,
                                     getHeight());

        displaySplits[1]->refreshScreenBuffer();
        //displaySplits[1]->resized();

        if (! displaySplits[1]->getSelectedState())
            displaySplits[0]->select();
        else
            displaySplits[1]->select();
    }
    else if (selectedLayout == THREE_VERT)
    {
        displaySplits[0]->setVisible (true);
        displaySplits[1]->setVisible (true);
        displaySplits[2]->setVisible (true);

        displaySplits[0]->setBounds (0,
                                     0,
                                     getWidth() * tripleVerticalSplitRatio[0] - borderSize,
                                     getHeight());

        displaySplits[0]->refreshScreenBuffer();

        displaySplits[1]->setBounds (getWidth() * tripleVerticalSplitRatio[0] + borderSize,
                                     0,
                                     getWidth() * (tripleVerticalSplitRatio[1] - tripleVerticalSplitRatio[0]) - borderSize,
                                     getHeight());

        displaySplits[1]->refreshScreenBuffer();
        // displaySplits[1]->resized();

        displaySplits[2]->setBounds (getWidth() * (tripleVerticalSplitRatio[1]) + borderSize,
                                     0,
                                     getWidth() * (1 - tripleVerticalSplitRatio[1]) - borderSize,
                                     getHeight());

        displaySplits[2]->refreshScreenBuffer();
        //displaySplits[2]->resized();

        if (! displaySplits[1]->getSelectedState() && ! displaySplits[2]->getSelectedState())
            displaySplits[0]->select();
    }
    else if (selectedLayout == TWO_HORZ)
    {
        displaySplits[0]->setVisible (true);
        displaySplits[1]->setVisible (true);

        displaySplits[2]->setVisible (false);
        displaySplits[2]->setBounds (0, 0, 0, 0);

        displaySplits[0]->setBounds (0,
                                     0,
                                     getWidth(),
                                     getHeight() * doubleHorizontalSplitRatio - borderSize);

        displaySplits[0]->refreshScreenBuffer();
        // displaySplits[0]->resized();

        displaySplits[1]->setBounds (0,
                                     getHeight() * doubleHorizontalSplitRatio + borderSize,
                                     getWidth(),
                                     getHeight() * (1 - doubleHorizontalSplitRatio) - borderSize);

        displaySplits[1]->refreshScreenBuffer();
        //displaySplits[1]->resized();

        if (! displaySplits[1]->getSelectedState())
            displaySplits[0]->select();
    }
    else
    {
        displaySplits[0]->setVisible (true);
        displaySplits[1]->setVisible (true);
        displaySplits[2]->setVisible (true);

        displaySplits[0]->setBounds (0,
                                     0,
                                     getWidth(),
                                     getHeight() * tripleHorizontalSplitRatio[0] - borderSize);

        displaySplits[0]->refreshScreenBuffer();
        // displaySplits[0]->resized();

        displaySplits[1]->setBounds (0,
                                     getHeight() * tripleHorizontalSplitRatio[0] + borderSize,
                                     getWidth(),
                                     getHeight() * (tripleHorizontalSplitRatio[1] - tripleHorizontalSplitRatio[0]) - borderSize);

        displaySplits[1]->refreshScreenBuffer();
        // displaySplits[1]->resized();

        displaySplits[2]->setBounds (0,
                                     getHeight() * (tripleHorizontalSplitRatio[1]) + borderSize,
                                     getWidth(),
                                     getHeight() * (1 - tripleHorizontalSplitRatio[1]) - borderSize);

        displaySplits[2]->refreshScreenBuffer();
        // displaySplits[2]->resized();

        if (! displaySplits[1]->getSelectedState() && ! displaySplits[2]->getSelectedState())
            displaySplits[0]->select();
    }

    syncDisplays();

    for (int i = 0; i < 3; i++)
    {
        if (optionsDrawerIsOpen)
            displaySplits[i]->options->setBounds (0, getHeight() - 210, getWidth(), 210);
        else
            displaySplits[i]->options->setBounds (0, getHeight() - 60, getWidth(), 60);
    }
}

void LfpDisplayCanvas::beginAnimation()
{
    for (auto split : displaySplits)
    {
        split->beginAnimation();
    }
}

void LfpDisplayCanvas::endAnimation()
{
    for (auto split : displaySplits)
    {
        split->endAnimation();
    }
}

void LfpDisplayCanvas::syncDisplays()
{
    for (auto split : displaySplits)
    {
        split->syncDisplay();
    }
}

void LfpDisplayCanvas::updateSettings()
{
    for (auto split : displaySplits)
    {
        split->updateSettings();
    }
}

void LfpDisplayCanvas::refreshState()
{
    for (auto split : displaySplits)
    {
        split->refresh();
    }
}

void LfpDisplayCanvas::select (LfpDisplaySplitter* splitter)
{
    for (auto split : displaySplits)
    {
        if (split != splitter)
        {
            split->deselect();
            split->options->setVisible (false);
        }
        else
        {
            split->options->setVisible (true);
        }
    }

    splitter->options->resized();
}

void LfpDisplayCanvas::setLayout (SplitLayouts sl)
{
    selectedLayout = sl;

    if (! isLoading)
        resized();
}

bool LfpDisplayCanvas::makeRoomForOptions (int splitID)
{
    if (selectedLayout == TWO_HORZ)
    {
        if (splitID < 1)
            return false;
    }
    else if (selectedLayout == THREE_HORZ)
    {
        if (splitID < 2)
            return false;
    }

    return true;
}

bool LfpDisplayCanvas::canSelect (int splitID)
{
    if (selectedLayout != SINGLE)
    {
        return true;
    }

    return false;
}

void LfpDisplayCanvas::mouseMove (const MouseEvent& e)
{
    MouseEvent event = e.getEventRelativeTo (this);

    int borderSize = 5;

    if (selectedLayout == TWO_VERT)
    {
        int relativeX = event.position.getX();
        //
        if ((relativeX > getWidth() * doubleVerticalSplitRatio - borderSize) && (relativeX < getWidth() * doubleVerticalSplitRatio + borderSize))
        {
            setMouseCursor (MouseCursor::LeftRightResizeCursor);
        }
        else
        {
            setMouseCursor (MouseCursor::NormalCursor);
        }
    }
    else if (selectedLayout == THREE_VERT)
    {
        int relativeX = event.position.getX();

        if ((relativeX > getWidth() * tripleVerticalSplitRatio[0] - borderSize) && (relativeX < getWidth() * tripleVerticalSplitRatio[0] + borderSize))
        {
            setMouseCursor (MouseCursor::LeftRightResizeCursor);
        }
        else if (
            (relativeX > getWidth() * tripleVerticalSplitRatio[1] - borderSize) && (relativeX < getWidth() * tripleVerticalSplitRatio[1] + borderSize))
        {
            setMouseCursor (MouseCursor::LeftRightResizeCursor);
        }
        else
        {
            setMouseCursor (MouseCursor::NormalCursor);
        }
    }
    else if (selectedLayout == TWO_HORZ)
    {
        int relativeY = event.position.getY();

        if ((relativeY > getHeight() * doubleHorizontalSplitRatio - borderSize) && (relativeY < getHeight() * doubleHorizontalSplitRatio + borderSize))
        {
            setMouseCursor (MouseCursor::UpDownResizeCursor);
        }
        else
        {
            setMouseCursor (MouseCursor::NormalCursor);
        }
    }
    else if (selectedLayout == THREE_HORZ)
    {
        int relativeY = event.position.getY();

        if ((relativeY > getHeight() * tripleHorizontalSplitRatio[0] - borderSize) && (relativeY < getHeight() * tripleHorizontalSplitRatio[0] + borderSize))
        {
            setMouseCursor (MouseCursor::UpDownResizeCursor);
        }
        else if (
            (relativeY > getHeight() * tripleHorizontalSplitRatio[1] - borderSize) && (relativeY < getHeight() * tripleHorizontalSplitRatio[1] + borderSize))
        {
            setMouseCursor (MouseCursor::UpDownResizeCursor);
        }
        else
        {
            setMouseCursor (MouseCursor::NormalCursor);
        }
    }
}

void LfpDisplayCanvas::mouseDrag (const MouseEvent& e)
{
    MouseEvent event = e.getEventRelativeTo (this);

    int borderSize = 5;

    if (selectedLayout == TWO_VERT)
    {
        int relativeX = event.getMouseDownX();

        if ((relativeX > getWidth() * doubleVerticalSplitRatio - borderSize) && (relativeX < getWidth() * doubleVerticalSplitRatio + borderSize))
        {
            borderToDrag = 0;
        }

        if (borderToDrag == 0)
        {
            doubleVerticalSplitRatio = float (event.position.getX()) / float (getWidth());

            if (doubleVerticalSplitRatio < 0.15)
                doubleVerticalSplitRatio = 0.15;
            else if (doubleVerticalSplitRatio > 0.85)
                doubleVerticalSplitRatio = 0.85;
        }
    }
    else if (selectedLayout == THREE_VERT)
    {
        int relativeX = event.getMouseDownX();

        if ((relativeX > getWidth() * tripleVerticalSplitRatio[0] - borderSize) && (relativeX < getWidth() * tripleVerticalSplitRatio[0] + borderSize))
        {
            borderToDrag = 0;
        }
        else if ((relativeX > getWidth() * tripleVerticalSplitRatio[1] - borderSize) && (relativeX < getWidth() * tripleVerticalSplitRatio[1] + borderSize))
        {
            borderToDrag = 1;
        }

        if (borderToDrag == 0)
        {
            tripleVerticalSplitRatio.set (0, float (event.position.getX()) / float (getWidth()));

            if (tripleVerticalSplitRatio[0] < 0.15)
                tripleVerticalSplitRatio.set (0, 0.15);
            else if (tripleVerticalSplitRatio[0] > tripleVerticalSplitRatio[1] - 0.15)
                tripleVerticalSplitRatio.set (0, tripleVerticalSplitRatio[1] - 0.15);
        }

        else if (borderToDrag == 1)
        {
            tripleVerticalSplitRatio.set (1, float (event.position.getX()) / float (getWidth()));

            if (tripleVerticalSplitRatio[1] < tripleVerticalSplitRatio[0] + 0.15)
                tripleVerticalSplitRatio.set (1, tripleVerticalSplitRatio[0] + 0.15);
            else if (tripleVerticalSplitRatio[1] > 0.85)
                tripleVerticalSplitRatio.set (1, 0.85);
        }
    }
    else if (selectedLayout == TWO_HORZ)
    {
        int relativeY = event.getMouseDownY();

        if ((relativeY > getHeight() * doubleHorizontalSplitRatio - borderSize) && (relativeY < getHeight() * doubleHorizontalSplitRatio + borderSize))

        {
            borderToDrag = 0;
        }

        if (borderToDrag == 0)
        {
            doubleHorizontalSplitRatio = float (event.position.getY()) / float (getHeight());

            if (doubleHorizontalSplitRatio < 0.15)
                doubleHorizontalSplitRatio = 0.15;
            else if (doubleHorizontalSplitRatio > 0.85)
                doubleHorizontalSplitRatio = 0.85;
        }
    }
    else if (selectedLayout == THREE_HORZ)
    {
        int relativeY = event.getMouseDownY();

        if ((relativeY > getHeight() * tripleHorizontalSplitRatio[0] - borderSize) && (relativeY < getHeight() * tripleHorizontalSplitRatio[0] + borderSize))
        {
            borderToDrag = 0;
        }
        else if ((relativeY > getHeight() * tripleHorizontalSplitRatio[1] - borderSize) && (relativeY < getHeight() * tripleHorizontalSplitRatio[1] + borderSize))
        {
            borderToDrag = 1;
        }

        if (borderToDrag == 0)
        {
            tripleHorizontalSplitRatio.set (0, float (event.position.getY()) / float (getHeight()));

            if (tripleHorizontalSplitRatio[0] < 0.15)
                tripleHorizontalSplitRatio.set (0, 0.15);
            else if (tripleHorizontalSplitRatio[0] > tripleHorizontalSplitRatio[1] - 0.15)
                tripleHorizontalSplitRatio.set (0, tripleHorizontalSplitRatio[1] - 0.15);
        }

        else if (borderToDrag == 1)
        {
            tripleHorizontalSplitRatio.set (1, float (event.position.getY()) / float (getHeight()));

            if (tripleHorizontalSplitRatio[1] < tripleHorizontalSplitRatio[0] + 0.15)
                tripleHorizontalSplitRatio.set (1, tripleHorizontalSplitRatio[0] + 0.15);
            else if (tripleHorizontalSplitRatio[1] > 0.85)
                tripleHorizontalSplitRatio.set (1, 0.85);
        }
    }
}

void LfpDisplayCanvas::mouseUp (const MouseEvent& e)
{
    if (borderToDrag >= 0)
    {
        resized();
        borderToDrag = -1;
    }
}

void LfpDisplayCanvas::toggleOptionsDrawer (bool isOpen)
{
    optionsDrawerIsOpen = isOpen;

    for (int i = 0; i < 3; i++)
    {
        if (optionsDrawerIsOpen)
            displaySplits[i]->options->setBounds (0, getHeight() - 210, getWidth(), 210);
        else
            displaySplits[i]->options->setBounds (0, getHeight() - 60, getWidth(), 60);

        displaySplits[i]->options->setShowHideOptionsButtonState (optionsDrawerIsOpen);
    }
}

int LfpDisplayCanvas::getTotalSplits()
{
    return displaySplits.size();
}

bool LfpDisplayCanvas::keyPressed (const KeyPress& key)
{
    if (key.getKeyCode() == key.spaceKey)
    {
        for (auto split : displaySplits)
        {
            if (split->getSelectedState() || selectedLayout == SINGLE)
                split->handleSpaceKeyPauseEvent();
        }

        return true;
    }

    return false;
}

bool LfpDisplayCanvas::keyPressed (const KeyPress& key, Component* orig)
{
    if (getTopLevelComponent() == orig && isVisible())
    {
        return keyPressed (key);
    }
    return false;
}

void LfpDisplayCanvas::removeBufferForDisplay (int splitID)
{
    displaySplits[splitID]->displayBuffer = nullptr;
}

#if BUILD_TESTS

bool LfpDisplayCanvas::getChannelBitmapBounds (int splitIndex, int& x, int& y, int& width, int& height)
{
    if (splitIndex >= displaySplits.size())
    {
        return false;
    }
    x = displaySplits[splitIndex]->leftmargin;
    y = displaySplits[splitIndex]->viewport->getY();
    width = displaySplits[splitIndex]->lfpDisplay->lfpChannelBitmap.getWidth();
    height = displaySplits[splitIndex]->lfpDisplay->lfpChannelBitmap.getHeight();
    return true;
}

bool LfpDisplayCanvas::getChannelColours (int splitIndex, Array<Colour>& channelColours, Colour& backgroundColour)
{
    if (splitIndex >= displaySplits.size())
    {
        return false;
    }
    channelColours = displaySplits[splitIndex]->lfpDisplay->channelColours;
    backgroundColour = displaySplits[splitIndex]->lfpDisplay->getColourSchemePtr()->getBackgroundColour();
    return true;
}

bool LfpDisplayCanvas::setChannelHeight (int splitIndex, int height)
{
    if (splitIndex >= displaySplits.size())
    {
        return false;
    }
    displaySplits[splitIndex]->lfpDisplay->setChannelHeight (height);
    return true;
}

bool LfpDisplayCanvas::setChannelRange (int splitIndex, int range, ContinuousChannel::Type type)
{
    if (splitIndex >= displaySplits.size())
    {
        return false;
    }
    displaySplits[splitIndex]->lfpDisplay->setRange (range, type);

    return true;
}
#endif

void LfpDisplayCanvas::saveCustomParametersToXml (XmlElement* xml)
{
    for (int i = 0; i < 3; i++)
    {
        displaySplits[i]->options->saveParameters (xml);
    }

    XmlElement* xmlNode = xml->createNewChildElement ("CANVAS");
    xmlNode->setAttribute ("doubleVerticalSplitRatio", doubleVerticalSplitRatio);
    xmlNode->setAttribute ("doubleHorizontalSplitRatio", doubleHorizontalSplitRatio);
    xmlNode->setAttribute ("tripleHorizontalSplitRatio", String (tripleHorizontalSplitRatio[0]) + "," + String (tripleHorizontalSplitRatio[1]));
    xmlNode->setAttribute ("tripleVerticalSplitRatio", String (tripleVerticalSplitRatio[0]) + "," + String (tripleVerticalSplitRatio[1]));

    xmlNode->setAttribute ("showAllOptions", optionsDrawerIsOpen);
}

void LfpDisplayCanvas::loadCustomParametersFromXml (XmlElement* xml)
{
    LOGD ("LfpDisplayCanvas loading custom parameters.");

    int64 start = Time::getHighResolutionTicks();

    for (auto* xmlNode : xml->getChildIterator())
    {
        if (xmlNode->hasTagName ("CANVAS"))
        {
            doubleHorizontalSplitRatio = xmlNode->getStringAttribute ("doubleHorizontalSplitRatio").getFloatValue();
            doubleVerticalSplitRatio = xmlNode->getStringAttribute ("doubleVerticalSplitRatio").getFloatValue();

            String splitString = xmlNode->getStringAttribute ("tripleHorizontalSplitRatio");
            int splitPoint = splitString.indexOf (",");
            tripleHorizontalSplitRatio.set (0, splitString.substring (0, splitPoint).getFloatValue());
            tripleHorizontalSplitRatio.set (1, splitString.substring (splitPoint + 1).getFloatValue());

            splitString = xmlNode->getStringAttribute ("tripleVerticalSplitRatio");
            splitPoint = splitString.indexOf (",");
            tripleVerticalSplitRatio.set (0, splitString.substring (0, splitPoint).getFloatValue());
            tripleVerticalSplitRatio.set (1, splitString.substring (splitPoint + 1).getFloatValue());

            toggleOptionsDrawer (xmlNode->getBoolAttribute ("showAllOptions", false));

            //LOGD("    Loaded canvas split settings in ", MS_FROM_START, " milliseconds");
        }
    }

    start = Time::getHighResolutionTicks();

    for (int i = 0; i < 3; i++)
    {
        displaySplits[i]->options->loadParameters (xml);
    }

    //LOGD("    Loaded split display parameters in ", MS_FROM_START, " milliseconds");

    start = Time::getHighResolutionTicks();

    isLoading = false;
    resized();

    //LOGD("    Resized in ", MS_FROM_START, " milliseconds");
}

LfpDisplaySplitter::LfpDisplaySplitter (LfpDisplayNode* node,
                                        LfpDisplayCanvas* canvas_,
                                        DisplayBuffer* db,
                                        int id) : timebase (1.0f), displayGain (1.0f), timeOffset (0.0f), triggerChannel (-1), trialAveraging (false), splitID (id), processor (node), displayBuffer (db), canvas (canvas_), reachedEnd (false), screenBufferWidth (0)
{
    isSelected = false;

    viewport = std::make_unique<LfpViewport> (this);
    lfpDisplay = std::make_unique<LfpDisplay> (this, viewport.get());
    timescale = std::make_unique<LfpTimescale> (this, lfpDisplay.get());
    options = std::make_unique<LfpDisplayOptions> (canvas, this, timescale.get(), lfpDisplay.get(), node);

    streamSelection = std::make_unique<ComboBox> ("Stream selection");
    streamSelection->addListener (this);

    lfpDisplay->options = options.get();

    viewport->setViewedComponent (lfpDisplay.get(), false);
    viewport->setScrollBarsShown (true, false);

    scrollBarThickness = 15;
    viewport->setScrollBarThickness (scrollBarThickness);

    addAndMakeVisible (timescale.get());
    addAndMakeVisible (viewport.get());
    addAndMakeVisible (streamSelection.get());

    nChans = 0;

    drawSaturationWarning = false;
    drawClipWarning = false;

    isLoading = true;
    isUpdating = false;

    displayBuffer = nullptr;
}

String LfpDisplaySplitter::getStreamKey()
{
    if (processor->getDataStreams().size() == 0 || selectedStreamId == 0)
        return "";

    DataStream* stream = processor->getDataStream (selectedStreamId);
    return stream->getKey();
}

void LfpDisplaySplitter::resized()
{
    const int timescaleHeight = 30;

    timescale->setBounds (leftmargin, 0, getWidth() - scrollBarThickness - leftmargin, timescaleHeight);

    if (canvas->makeRoomForOptions (splitID))
    {
        viewport->setBounds (0, timescaleHeight, getWidth(), getHeight() - 92);
    }
    else
    {
        viewport->setBounds (0, timescaleHeight, getWidth(), getHeight() - 32);
    }

    //if (screenBufferMean != nullptr)
    //{
    refreshScreenBuffer();
    //}

    if (nChans > 0)
    {
        //std::cout << "Changing view for display " << splitID << std::endl;

        if (lfpDisplay->getSingleChannelState())
            lfpDisplay->setChannelHeight (viewport->getHeight(), false);

        lfpDisplay->setBounds (0, 0, getWidth() - scrollBarThickness, lfpDisplay->getChannelHeight() * lfpDisplay->drawableChannels.size());

        lfpDisplay->restoreViewPosition();
    }
    else
    {
        lfpDisplay->setBounds (0, 0, getWidth(), getHeight());
    }

    streamSelection->setBounds (4, 4, 140, 22);
}

void LfpDisplaySplitter::resizeToChannels (bool respectViewportPosition)
{
    //std::cout << "RESIZE TO CHANNELS " << std::endl;

    lfpDisplay->setBounds (0, 0, getWidth() - scrollBarThickness, lfpDisplay->getChannelHeight() * lfpDisplay->drawableChannels.size());

    // if param is flagged, move the viewport scroll back to same relative position before
    // resize took place
    if (! respectViewportPosition)
        return;

    // get viewport scroll position as ratio against lfpDisplay's dims
    // so that we can set scrollbar back to rough position before resize
    // (else viewport scrolls back to top after resize)
    const double yPositionRatio = viewport->getViewPositionY() / (double) lfpDisplay->getHeight();
    const double xPositionRatio = viewport->getViewPositionX() / (double) lfpDisplay->getWidth();

    viewport->setViewPosition (lfpDisplay->getWidth() * xPositionRatio,
                               lfpDisplay->getHeight() * yPositionRatio);
}

void LfpDisplaySplitter::beginAnimation()
{
    if (displayBuffer != nullptr)
    {
        if (! processor->getHeadlessMode())
        {
            displayBuffer->resetIndices();
        }
        displayBufferSize = displayBuffer->getNumSamples();

        syncDisplay();

        numTrials = -1;

        eventState = 0;
    }

    startTimer (20);

    reachedEnd = true;
}

void LfpDisplaySplitter::endAnimation()
{
    stopTimer();
}

void LfpDisplaySplitter::timerCallback()
{
    refresh();
}

void LfpDisplaySplitter::monitorChannel (int chan)
{
    int globalIndex = processor->getDataStream (selectedStreamId)->getContinuousChannels()[chan]->getGlobalIndex();

    processor->setParameter (99, globalIndex);
}

void LfpDisplaySplitter::select()
{
    if (canvas->canSelect (splitID))
    {
        isSelected = true;

        canvas->select (this);
    }

    repaint();
}

void LfpDisplaySplitter::deselect()
{
    isSelected = false;

    repaint();
}

void LfpDisplaySplitter::recordingStarted()
{
    for (int i = 0; i < lfpDisplay->getNumChannels(); i++) // update channel metadata
    {
        lfpDisplay->channelInfo[i]->recordingStarted();
    }
}

void LfpDisplaySplitter::recordingStopped()
{
    for (int i = 0; i < lfpDisplay->getNumChannels(); i++) // update channel metadata
    {
        lfpDisplay->channelInfo[i]->recordingStopped();
    }
}

void LfpDisplaySplitter::updateSettings()
{
    if (displayBuffer != nullptr)
        displayBuffer->removeDisplay (splitID);

    isUpdating = true;

    Array<DisplayBuffer*> availableBuffers = processor->getDisplayBuffers();

    if (availableBuffers.size() == 0)
        displayBuffer = nullptr;

    streamSelection->clear (dontSendNotification);

    bool foundMatchingBuffer = false;

    for (auto buffer : availableBuffers)
    {
        streamSelection->addItem (buffer->name, buffer->id);

        if (displayBuffer != nullptr)
        {
            if (buffer->streamKey == displayBuffer->streamKey)
                foundMatchingBuffer = true;
        }
    }

    if (! foundMatchingBuffer)
        displayBuffer = nullptr;

    if (displayBuffer == nullptr) // displayBuffer was deleted
    {
        if (availableBuffers.size() > 0)
        {
            displayBuffer = availableBuffers[0];
        }
    }

    if (displayBuffer == nullptr) // no inputs to this processor
    {
        nChans = 0;
        sampleRate = 44100.0f;

        options->setEnabled (true);
    }
    else
    {
        displayBuffer->addDisplay (splitID);

        streamSelection->setSelectedId (displayBuffer->id, dontSendNotification);
        selectedStreamId = displayBuffer->id;

        displayBufferSize = displayBuffer->getNumSamples();
        nChans = displayBuffer->numChannels;
        sampleRate = displayBuffer->sampleRate;

        options->setEnabled (true);
        channelOverlapFactor = options->selectedOverlapValue.getFloatValue();
    }

    if (eventDisplayBuffer == nullptr) // not yet initialized
    {
        eventDisplayBuffer = std::make_unique<AudioBuffer<float>> (1, getWidth());
        screenBufferMin = std::make_unique<AudioBuffer<float>> (nChans, getWidth());
        screenBufferMean = std::make_unique<AudioBuffer<float>> (nChans, getWidth());
        screenBufferMax = std::make_unique<AudioBuffer<float>> (nChans, getWidth());
    }
    else
    {
        if (nChans != lfpDisplay->getNumChannels()) // new channel count
        {
            refreshScreenBuffer();
        }
    }

    lfpDisplay->setNumChannels (nChans);

    for (int i = 0; i < nChans; i++) // update channel metadata
    {
        lfpDisplay->channels[i]->setName (displayBuffer->channelMetadata[i].name);
        lfpDisplay->channels[i]->setGroup (displayBuffer->channelMetadata[i].group);
        lfpDisplay->channels[i]->setDepth (displayBuffer->channelMetadata[i].ypos);
        lfpDisplay->channels[i]->setRecorded (displayBuffer->channelMetadata[i].isRecorded);
        lfpDisplay->channels[i]->updateType (displayBuffer->channelMetadata[i].type);

        lfpDisplay->channelInfo[i]->setName (displayBuffer->channelMetadata[i].name);
        lfpDisplay->channelInfo[i]->setGroup (displayBuffer->channelMetadata[i].group);
        lfpDisplay->channelInfo[i]->setDepth (displayBuffer->channelMetadata[i].ypos);
        lfpDisplay->channelInfo[i]->setRecorded (displayBuffer->channelMetadata[i].isRecorded);
        lfpDisplay->channelInfo[i]->updateType (displayBuffer->channelMetadata[i].type);

        lfpDisplay->updateRange (i);

        if (i == 0)
        {
            options->setSelectedType (displayBuffer->channelMetadata[i].type);
        }
    }

    lfpDisplay->rebuildDrawableChannelsList(); // calls setColours(), which calls refresh

    isLoading = false;

    syncDisplay(); // sets lastBitmapIndex to 0

    isUpdating = false;

    //lfpDisplay->setColours(); // calls refresh

    resized();

    lfpDisplay->restoreViewPosition();

    //lfpDisplay->refresh(); // calls refresh
}

int LfpDisplaySplitter::getChannelHeight()
{
    return options->getChannelHeight();
}

void LfpDisplaySplitter::setTriggerChannel (int ch)
{
    triggerChannel = ch;

    if (triggerChannel == -1)
        timescale->setTimebase (timebase);

    syncDisplay();
}

void LfpDisplaySplitter::setAveraging (bool avg)
{
    if (trialAveraging == false)
    {
        numTrials = -1;
    }

    trialAveraging = avg;
}

void LfpDisplaySplitter::resetTrials()
{
    numTrials = -1;
}

void LfpDisplaySplitter::refreshSplitterState()
{
    syncDisplay();
}

void LfpDisplaySplitter::refreshScreenBuffer()
{
    const int extraWidth = 4;

    if (getWidth() == 0)
        return;

    screenBufferWidth = getWidth() * extraWidth;

    if (screenBufferMean->getNumSamples() < screenBufferWidth || screenBufferMean->getNumChannels() != nChans) // screenbuffer is too small
    {
        eventDisplayBuffer->setSize (1, screenBufferWidth);
        screenBufferMin->setSize (nChans, screenBufferWidth);
        screenBufferMean->setSize (nChans, screenBufferWidth);
        screenBufferMax->setSize (nChans, screenBufferWidth);
    }

    //std::cout << "Display " << splitID  << " setting screen buffer width to " << screenBufferWidth << std::endl;

    if (! lfpDisplay->isPaused())
    {
        //std::cout << "Display " << splitID << " clearing all buffers " << std::endl;

        updateScreenBuffer();

        for (int channel = 0; channel <= nChans; channel++)
        {
            screenBufferIndex.set (channel, 0);
            lastScreenBufferIndex.set (channel, 0);
        }

        eventDisplayBuffer->clear();
        screenBufferMin->clear();
        screenBufferMean->clear();
        screenBufferMax->clear();
    }
}

void LfpDisplaySplitter::syncDisplay()
{
    //for (int channel = 0; channel <= nChans; channel++)
    //{
    //    screenBufferIndex.set(channel, 0);
    //    lastScreenBufferIndex.set(channel, 0);

    //}

    lfpDisplay->sync();

    syncDisplayBuffer();
}

void LfpDisplaySplitter::syncDisplayBuffer()
{
    if (displayBuffer == nullptr)
        return;

    for (int channel = 0; channel <= nChans; channel++)
    {
        displayBufferIndex.set (channel, displayBuffer->displayBufferIndices[channel]);
        leftOverSamples.set (channel, 0.0f);
    }

    samplesPerBufferPass = 0;
}

void LfpDisplaySplitter::pause (bool shouldPause)
{
    if (shouldPause)
    {
        isUpdating = true;
    }
    else
    {
        isUpdating = false;
    }
}

void LfpDisplaySplitter::updateScreenBuffer()
{
    if (isVisible() && displayBuffer != nullptr && ! isUpdating)
    {
        // std::cout << "Update screen buffer" << std::endl;

        int triggerTime = triggerChannel >= 0
                              ? int (processor->getLatestTriggerTime (splitID))
                              : -1;

        int maxSamples = screenBufferWidth;
        int displayWidth = lfpDisplay->lfpChannelBitmap.getWidth();

        if (triggerChannel >= 0)
        {
            maxSamples = displayWidth;
        }

        if (triggerTime > 0)
        {
            processor->acknowledgeTrigger (splitID);
        }

        for (int channel = 0; channel <= nChans; channel++) // pull one extra channel for event display
        {
            int dbi = displayBufferIndex[channel]; // display buffer index from the last round of drawing

            int newDisplayBufferIndex = displayBuffer->displayBufferIndices[channel]; // get the latest value from the display buffer

            int newSamples = newDisplayBufferIndex - dbi; // N new samples (not pixels) to be drawn

            if (newSamples == 0)
            {
                //std::cout << "No new samples." << std::endl;
                lastScreenBufferIndex.set (channel, screenBufferIndex[channel]);
                continue;
            }

            if (newSamples < 0)
                newSamples += displayBufferSize;

            //if (channel == 0)
            //    std::cout << newSamples << " new samples." << std::endl;

            // this number is crucial -- converting from samples to values (in px) for the screen buffer:
            float ratio = sampleRate * timebase / float (displayWidth); // samples / pixel

            float pixelsToFill = float (newSamples) / ratio; // M pixels to update

            int sbi = screenBufferIndex[channel];

            // hold the last screen buffer index for comparison
            lastScreenBufferIndex.set (channel, sbi);
            //std::cout << "Setting channel " << channel << " lastScreenBufferIndex to " << lastScreenBufferIndex[channel] << std::endl;

            float subSampleOffset = leftOverSamples[channel];

            //if (ratio > 1)
            pixelsToFill += subSampleOffset; // keep track of fractional pixels left over from the last round

            if (triggerChannel >= 0)
            {
                // we may need to wait for a trigger
                if (triggerTime >= 0)
                {
                    if (sbi == 0 || reachedEnd)
                    {
                        const int screenThird = int (displayWidth * ratio / 3);
                        const int dispBufLim = displayBufferSize / 2;

                        int t0 = triggerTime - std::min (screenThird, dispBufLim); // rewind displayBufferIndex

                        if (t0 < 0)
                        {
                            t0 += displayBufferSize;
                        }

                        dbi = t0;
                        newSamples = newDisplayBufferIndex - dbi;

                        if (newSamples < 0)
                            newSamples += displayBufferSize;

                        pixelsToFill = newSamples / ratio;
                        subSampleOffset = 0;

                        // rewind screen buffer to the far left
                        screenBufferIndex.set (channel, 0);
                        sbi = 0;
                        lastScreenBufferIndex.set (channel, 0);

                        if (channel == 0)
                        {
                            numTrials += 1;

                            //std::cout << "Rewinding playhead" << std::endl;
                            lfpDisplay->lastBitmapIndex = 0;

                            /*std::cout << "Trial number: " << numTrials << std::endl;
                            std::cout << "maxSamples: " << maxSamples << std::endl;
                            std::cout << "ratio: " << ratio << std::endl;
                            std::cout << "dispBufLim: " << dispBufLim << std::endl;
                            std::cout << "screenThird: " << screenThird << std::endl;
                            std::cout << "triggerTime: " << triggerTime << std::endl;
                            std::cout << "t0: " << t0 << std::endl;
                            std::cout << "newSamples: " << newSamples << std::endl;
                            std::cout << "pixels to fill: " << pixelsToFill << std::endl;
                            std::cout << "sbi: " << sbi << std::endl;
                            std::cout << "playhead: " << lfpDisplay->lastBitmapIndex << std::endl;

                            std::cout << std::endl;*/
                        }

                        if (channel == nChans) // all channels have been reset
                        {
                            triggerTime = -1;
                            timescale->setTimebase (timebase, float (std::min (screenThird, dispBufLim)) / sampleRate);
                            reachedEnd = false;
                        }
                    }
                }
                else
                {
                    if (reachedEnd)
                    {
                        screenBufferIndex.set (channel, sbi); // don't update
                        return;
                    }
                }
            }

            // HELPFUL FOR DEBUGGING:

            /*if (channel == 0)
                std::cout << "Split "
                << splitID << " ch: "
                << channel << " sbi: "
                << sbi << " old_dbi: "
                << dbi << " new_dbi: "
                << newDisplayBufferIndex << " nSamp: "
                << newSamples << " pix: "
                << pixelsToFill << " ratio: "
                << ratio << " sso: "
                << subSampleOffset << " max: "
                << maxSamples << " playhead: "
                << lfpDisplay->lastBitmapIndex
                << std::endl;*/

            int sampleNumber = 0;

            if (pixelsToFill > 0 && pixelsToFill < 1000000)
            {
                float i;

                for (i = 0; i < pixelsToFill; i++)
                {
                    if (! lfpDisplay->isPaused())
                    {
                        if (channel == nChans)
                        {
                            eventDisplayBuffer->clear (0, sbi, 1);
                        }
                        else
                        {
                            if (triggerChannel < 0 || numTrials == 0 || trialAveraging == false)
                            {
                                screenBufferMean->clear (channel, sbi, 1);
                                screenBufferMin->clear (channel, sbi, 1);
                                screenBufferMax->clear (channel, sbi, 1);
                            }
                            else
                            {
                                screenBufferMean->applyGain (channel, sbi, 1, numTrials);
                                screenBufferMin->applyGain (channel, sbi, 1, numTrials);
                                screenBufferMax->applyGain (channel, sbi, 1, numTrials);
                            }
                        }

                        if (ratio < 1.0) // less than one sample per pixel
                        {
                            if (channel == nChans)
                            {
                                eventDisplayBuffer->setSample (0, sbi, displayBuffer->getSample (channel, dbi));
                            }
                            else
                            {
                                float alpha = subSampleOffset;
                                float invAlpha = 1.0f - alpha;

                                int lastIndex = dbi - 1;

                                if (lastIndex < 0)
                                {
                                    lastIndex = displayBufferSize;
                                    continue;
                                }

                                float val0 = displayBuffer->getSample (channel, lastIndex);
                                float val1 = displayBuffer->getSample (channel, dbi);

                                float val = invAlpha * val0 + alpha * val1;

                                screenBufferMean->addSample (channel, sbi, val);
                                screenBufferMin->addSample (channel, sbi, val);
                                screenBufferMax->addSample (channel, sbi, val);
                            }

                            subSampleOffset += ratio;

                            if (subSampleOffset > 1.0f) // go to next pixel
                            {
                                subSampleOffset -= 1.0f;
                                dbi += 1;
                                dbi %= displayBufferSize;
                            }
                        }
                        else
                        { // more than one sample per pixel

                            float sample_min = 10000000;
                            float sample_max = -10000000;
                            float sample_sum = 0;
                            float sampleCount = 0;

                            subSampleOffset += ratio;

                            if (subSampleOffset <= 1.0f)
                            {
                                sample_sum = displayBuffer->getSample (channel, dbi);
                                sample_min = sample_sum;
                                sample_max = sample_sum;
                                sampleCount = 1.0f;
                            }

                            bool foundIt = false;

                            while (subSampleOffset > 1.0f && sampleNumber < newSamples)
                            {
                                sampleNumber++;

                                float sample_current = displayBuffer->getSample (channel, dbi);

                                sample_sum = sample_sum + sample_current;

                                if (sample_min >= sample_current)
                                {
                                    sample_min = sample_current;
                                }

                                if (sample_max <= sample_current)
                                {
                                    sample_max = sample_current;
                                }

                                subSampleOffset -= 1.0f;

                                dbi += 1;
                                dbi %= displayBufferSize;

                                sampleCount += 1.0f;
                            }

                            float sample_mean = sample_sum / sampleCount;

                            // update event channel
                            if (channel == nChans)
                            {
                                eventDisplayBuffer->setSample (0, sbi, sample_max);
                            }
                            else
                            {
                                if (sbi > 0)
                                {
                                    if (sample_max < screenBufferMin->getSample (channel, sbi - 1))
                                        sample_max = screenBufferMin->getSample (channel, sbi - 1);

                                    if (sample_min > screenBufferMax->getSample (channel, sbi - 1))
                                        sample_min = screenBufferMax->getSample (channel, sbi - 1);
                                }

                                screenBufferMean->addSample (channel, sbi, sample_mean);
                                screenBufferMin->addSample (channel, sbi, sample_min);
                                screenBufferMax->addSample (channel, sbi, sample_max);
                            }
                        }

                        if (triggerChannel >= 0 && trialAveraging == true && channel != nChans)
                        {
                            screenBufferMean->applyGain (channel, sbi, 1, 1 / (numTrials + 1));
                            screenBufferMin->applyGain (channel, sbi, 1, 1 / (numTrials + 1));
                            screenBufferMax->applyGain (channel, sbi, 1, 1 / (numTrials + 1));
                        }

                        sbi++;

                        if (triggerChannel >= 0)
                        {
                            if (sbi == maxSamples - 1)
                            {
                                //std::cout << "CH " << channel << " reached end: " << maxSamples << " samples " << std::endl;

                                if (channel == nChans)
                                {
                                    reachedEnd = true;
                                }

                                break;
                            }
                        }

                        sbi %= maxSamples;

                        // HISTOGRAM DRAWING IS CURRENTLY DISABLED
                        // similarly, for each pixel on the screen, we want a list of all values so we can draw a histogram later
                        // for simplicity, we'll just do this as 2d array, samplesPerPixel[px][samples]
                        // with an additional array sampleCountPerPixel[px] that holds the N samples per pixel

                        //if (channel < nChans) // we're looping over one 'extra' channel for events above, so make sure not to loop over that one here
                        // {
                        // this is for fancy drawing -- not used in new LFP Viewer
                        /*int c = 0;
                            for (int j = dbi; j < nextpix && c < MAX_N_SAMP_PER_PIXEL; j++)
                            {
                                float sample_current = displayBuffer->getSample(channel, j);
                                samplesPerPixel[channel][sbi][c] = sample_current;
                                c++;
                            }
                            if (c > 0){
                                sampleCountPerPixel.set(sbi, c - 1); // save count of samples for this pixel
                            }
                            else{
                                sampleCountPerPixel.set(sbi, 0);
                            }*/
                        //sample_mean = sample_mean / c;

                        //   }

                    } // !isPaused
                }

                if (ratio > 1.0f)
                    leftOverSamples.set (channel, pixelsToFill - i); // +(pixelsToFill - (i - 1)) * ratio);
                else
                    leftOverSamples.set (channel, subSampleOffset - 1.0f);

                //std::cout << "Setting channel " << channel << " sbi to " << sbi << std::endl;
                screenBufferIndex.set (channel, sbi);
                displayBufferIndex.set (channel, newDisplayBufferIndex); // need to store this locally
            }
        }
    }
}

void LfpDisplaySplitter::setTimebase (float t)
{
    timebase = t;

    /*if (timebase <= 0.1)
    {
        stopTimer();
        startTimer(1000);
    }
    else {
        stopTimer();
        startTimer(50);
    }*/

    if (trialAveraging)
    {
        numTrials = -1;
    }

    syncDisplay();
    refreshScreenBuffer();

    reachedEnd = true;
}

const float LfpDisplaySplitter::getXCoord (int chan, int samp)
{
    return samp;
}

const float LfpDisplaySplitter::getYCoord (int chan, int samp)
{
    return *screenBufferMean->getReadPointer (chan, samp);
}

const float LfpDisplaySplitter::getEventState (int samp)
{
    return *eventDisplayBuffer->getReadPointer (0, samp);
}

const float LfpDisplaySplitter::getYCoordMean (int chan, int samp)
{
    return *screenBufferMean->getReadPointer (chan, samp);
}
const float LfpDisplaySplitter::getYCoordMin (int chan, int samp)
{
    return *screenBufferMin->getReadPointer (chan, samp);
}
const float LfpDisplaySplitter::getYCoordMax (int chan, int samp)
{
    return *screenBufferMax->getReadPointer (chan, samp);
}

int LfpDisplaySplitter::getNumChannels()
{
    return nChans;
}

int LfpDisplaySplitter::getNumChannelsVisible()
{
    return lfpDisplay->drawableChannels.size();
}

uint16 LfpDisplaySplitter::getChannelStreamId (int channel)
{
    return processor->getContinuousChannel (channel)->getStreamId();
}

float LfpDisplaySplitter::getScreenBufferMean (int chan)
{
    float total = 0.0f;
    float numPts = 0;
    int sbi = screenBufferIndex[chan];

    for (int samp = 0; samp < (lfpDisplay->getWidth() - leftmargin); samp += 10)
    {
        total += screenBufferMean->getSample (chan, (sbi - samp + screenBufferWidth) % screenBufferWidth);
        numPts++;
    }

    //std::cout << sample << std::endl;

    return total / numPts;
}

float LfpDisplaySplitter::getDisplayBufferMean (int chan)
{
    float total = 0.0f;
    float numPts = 0;
    int dbi = displayBufferIndex[chan];

    for (int i = 0; i < displayBufferSize / 2; i += 2)
    {
        total += displayBuffer->getSample (chan, (dbi - i + displayBufferSize) % displayBufferSize);
        numPts++;
    }

    return total / numPts;
}

float LfpDisplaySplitter::getRMS (int chan)
{
    float rms = 0.0f;
    float numPts = 0;
    int dbi = displayBufferIndex[chan];

    for (int i = 0; i < displayBufferSize / 2; i += 2)
    {
        rms += pow (displayBuffer->getSample (chan, (dbi - i + displayBufferSize) % displayBufferSize), 2);
        numPts++;
    }

    return sqrt (rms / numPts);
}

bool LfpDisplaySplitter::getInputInvertedState()
{
    return options->getInputInvertedState();
}

void LfpDisplaySplitter::setDrawableSampleRate (float samplerate)
{
    displayedSampleRate = samplerate;
}

void LfpDisplaySplitter::setDrawableStream (uint16 sp)
{
    selectedStreamId = sp;
    displayBuffer = processor->displayBufferMap[processor->getDataStream (sp)->getStreamId()];

    updateSettings();
}

void LfpDisplaySplitter::redraw()
{
    if (! isLoading)
    {
        //fullredraw = true;
        repaint();
        // refresh();
    }
}

void LfpDisplaySplitter::paint (Graphics& g)
{
    g.setColour (lfpDisplay->getColourSchemePtr()->getBackgroundColour()); //background colour
    g.fillRect (0, 0, getWidth(), getHeight());

    //g.setColour(Colours::darkgrey);
    //for (int i = leftmargin; i < getWidth() - scrollBarThickness; i += 100)
    //{
    //    g.drawLine(i, 0, i, getHeight(), 1.0f);
    //}

    Colour borderColour;

    if (isSelected)
    {
        borderColour = Colour (252, 210, 0);
    }
    else
    {
        borderColour = Colour (findColour (ThemeColours::componentParentBackground));
    }

    g.setColour (borderColour);

    if (! canvas->makeRoomForOptions (splitID))
        g.drawRect (0, 0, getWidth(), getHeight(), 2);
    else
        g.drawRect (0, 0, getWidth(), getHeight() - 60, 2);

    g.fillRect (2, 2, getWidth() - 4, 28);
}

void LfpDisplaySplitter::visibleAreaChanged()
{
    fullredraw = true;

    //refresh();

    lfpDisplay->refresh();
}

void LfpDisplaySplitter::refresh()
{
    updateScreenBuffer();

    if (shouldRebuildChannelList)
    {
        shouldRebuildChannelList = false;
        lfpDisplay->rebuildDrawableChannelsList(); // calls resized()/refresh() after rebuilding list
    }
    else
    {
        lfpDisplay->refresh(); // redraws only the new part of the screen buffer, unless fullredraw is set to true
    }
}

void LfpDisplaySplitter::comboBoxChanged (juce::ComboBox* comboBox)
{
    if (comboBox == streamSelection.get())
    {
        setDrawableStream (comboBox->getSelectedId());

        select();
    }
}

void LfpDisplaySplitter::handleSpaceKeyPauseEvent()
{
    options->togglePauseButton();
}
