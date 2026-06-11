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

#include "LfpChannelDisplayInfo.h"
#include "EventDisplayInterface.h"
#include "LfpBitmapPlotter.h"
#include "LfpBitmapPlotterInfo.h"
#include "LfpChannelDisplay.h"
#include "LfpDisplay.h"
#include "LfpDisplayCanvas.h"
#include "LfpDisplayNode.h"
#include "LfpDisplayOptions.h"
#include "LfpTimescale.h"
#include "LfpViewport.h"
#include "PerPixelBitmapPlotter.h"
#include "ShowHideOptionsButton.h"
#include "SupersampledBitmapPlotter.h"

#include <math.h>

using namespace LfpViewer;

namespace
{
constexpr int channelLabelHeight = 12;
constexpr int channelButtonSize = 10;
constexpr int singleChannelInfoOffset = 75;

bool shouldDrawDenseChannelLabel (const int channelHeight, const int drawableChannelNumber, bool isSingleChannel)
{
    if (isSingleChannel || channelHeight >= 15)
        return true;

    return (drawableChannelNumber + 1) % 10 == 0;
}
} // namespace

#pragma mark - LfpChannelDisplayInfo -
// -------------------------------

LfpChannelDisplayInfo::LfpChannelDisplayInfo (LfpDisplaySplitter* canvas_, LfpDisplay* display_)
    : canvasSplit (canvas_),
      display (display_),
      recordingIsActive (false),
      x (-1.0f),
      y (-1.0f),
      rms (0.0f),
      mean (0.0f),
      samplerate (0),
      isSingleChannel (false)
{
    String svgString = "M302.189 329.126H196.105l55.831 135.993c3.889 9.428-.555 19.999-9.444 23.999l-49.165 21.427c-9.165 \
                       4-19.443-.571-23.332-9.714l-53.053-129.136-86.664 89.138C18.729 472.71 0 463.554 0 447.977V18.299C0 \
                       1.899 19.921-6.096 30.277 5.443l284.412 292.542c11.472 11.179 3.007 31.141-12.5 31.141z";

    pointerPath = Drawable::parseSVGPath (svgString);
}

void LfpChannelDisplayInfo::buttonClicked (Button* button)
{
    if (button == nullptr)
        return;

    bool state = button->getToggleState();
    const int channelNumber = button->getComponentID().getIntValue();

    display->setEnabledState (state, channelNumber, true);
}

void LfpChannelDisplayInfo::setSingleChannelState (bool state)
{
    isSingleChannel = state;
    syncEnableButtons();
    repaint();
}

int LfpChannelDisplayInfo::getChannelSampleRate()
{
    return samplerate;
}

void LfpChannelDisplayInfo::setChannelSampleRate (int samplerate_)
{
    samplerate = samplerate_;
}

void LfpChannelDisplayInfo::mouseDrag (const MouseEvent& e)
{
    if (e.mods.isLeftButtonDown()) // double check that we initiate only for left click and hold
    {
        if (e.mods.isCommandDown() && ! display->getSingleChannelState()) // CTRL + drag -> change channel spacing
        {
            // init state in our track zooming info struct
            if (! display->trackZoomInfo.isScrollingY)
            {
                auto& zoomInfo = display->trackZoomInfo;

                zoomInfo.isScrollingY = true;
                zoomInfo.componentStartHeight = display->getChannelHeight();
                zoomInfo.zoomPivotRatioY = (getY() + e.getMouseDownY()) / (float) display->getHeight();
                zoomInfo.zoomPivotRatioX = (getX() + e.getMouseDownX()) / (float) display->getWidth();
                zoomInfo.zoomPivotViewportOffset = getPosition() + e.getMouseDownPosition() - canvasSplit->viewport->getViewPosition();

                zoomInfo.unpauseOnScrollEnd = ! display->isPaused();
                if (! display->isPaused())
                    display->options->togglePauseButton (true);
            }

            int h = display->trackZoomInfo.componentStartHeight;
            int hdiff = 0;
            int dragDeltaY = -0.1 * (e.getScreenPosition().getY() - e.getMouseDownScreenY()); // invert so drag up -> scale up

            if (dragDeltaY > 0)
            {
                hdiff = 2 * dragDeltaY;
            }
            else
            {
                if (h > 5)
                    hdiff = 2 * dragDeltaY;
            }

            if (abs (h) > 100) // accelerate scrolling for large ranges
                hdiff *= 3;

            int newHeight = h + hdiff;

            // constrain the spread resizing to max and min values;
            if (newHeight < display->trackZoomInfo.minZoomHeight)
            {
                newHeight = display->trackZoomInfo.minZoomHeight;
            }
            else if (newHeight > display->trackZoomInfo.maxZoomHeight)
            {
                newHeight = display->trackZoomInfo.maxZoomHeight;
            }

            // return early if there is nothing to update
            if (newHeight == display->getChannelHeight())
            {
                return;
            }

            // set channel heights for all channels
            display->setChannelHeight (newHeight);

            display->options->setSpreadSelection (newHeight, false, true); // update combobox

            canvasSplit->fullredraw = true; //issue full redraw - scrolling without modifier doesnt require a full redraw

            display->setBounds (0, 0, display->getWidth() - 0, display->getChannelHeight() * display->drawableChannels.size()); // update height so that the scrollbar is correct

            int newViewportY = display->trackZoomInfo.zoomPivotRatioY * display->getHeight() - display->trackZoomInfo.zoomPivotViewportOffset.getY();
            if (newViewportY < 0)
                newViewportY = 0; // make sure we don't adjust beyond the edge of the actual view

            canvasSplit->viewport->setViewPosition (0, newViewportY);
        }
    }
}

void LfpChannelDisplayInfo::mouseUp (const MouseEvent& e)
{
    if (e.mods.isLeftButtonDown() && display->trackZoomInfo.isScrollingY)
    {
        display->trackZoomInfo.isScrollingY = false;
        if (display->trackZoomInfo.unpauseOnScrollEnd)
        {
            display->pause (false);
        }
    }
}

void LfpChannelDisplayInfo::recordingStarted()
{
    recordingIsActive = true;
    repaint();
}

void LfpChannelDisplayInfo::recordingStopped()
{
    recordingIsActive = false;
    repaint();
}

void LfpChannelDisplayInfo::paint (Graphics& g)
{
    if (display->options == nullptr)
        return;

    const bool showChannelNumbers = display->options->getChannelNameState();
    const int channelHeight = display->getChannelHeight();
    const bool isCentered = ! isSingleChannel && channelHeight < 15;

    for (int i = 0; i < display->drawableChannels.size(); ++i)
    {
        auto* channel = display->drawableChannels[i];
        const int center = getTrackCenterY (*channel);
        const bool drawChannelLabel = shouldDrawDenseChannelLabel (channelHeight, channel->getDrawableChannelNumber(), isSingleChannel);
        const bool showTypeString = isSingleChannel || channel->getChannelHeight() > 34;

        g.setColour (channel->getRecorded() && recordingIsActive ? Colours::red : Colours::grey);
        g.setFont (isSingleChannel ? FontOptions (16.0f).withStyle ("SemiBold")
                                   : FontOptions (14.0f));

        String channelString = drawChannelLabel ? (showChannelNumbers ? String (channel->getChannelNumber() + 1)
                                                                            : channel->getName())
                                                      : "--";

        if (drawChannelLabel)
        {
            if (showChannelNumbers)
                channelString = String (channel->getChannelNumber() + 1);
            else
                channelString = channel->getName();
        }
        else
        {
            channelString = channelHeight >= 10 ? "--" : "";
        }

        g.drawText (channelString,
                    showChannelNumbers ? 6 : 3,
                    center - (channelLabelHeight / 2),
                    getWidth() - (showChannelNumbers ? 6 : 3),
                    channelLabelHeight,
                    isCentered ? Justification::centred : Justification::centredLeft,
                    false);

        g.setColour (channel->getColour());
        g.fillRect (0, channel->getY(), 2, channel->getHeight());

        if (showTypeString)
        {
            constexpr int textHeight = 14;
            constexpr int textStartX = 5;
            const int textY = center + 10;
            const String typeStr = display->options->getTypeName (channel->getType());

            g.setFont (FontOptions (13.0f));
            g.setColour (channel->getColour());
            const auto currentFont = g.getCurrentFont();

            const int typeWidth = currentFont.getStringWidth (typeStr);
            const int typeBoundsWidth = typeWidth + 2;
            g.drawText (typeStr, textStartX, textY, typeBoundsWidth, textHeight, Justification::centredLeft, false);

            const String& unitsText = channel->getUnits();
            if (unitsText.isNotEmpty())
            {
                const int unitsX = textStartX + typeWidth + 5;
                const int unitsWidth = getWidth() - unitsX - 4;

                if (unitsWidth > 0)
                {
                    g.setColour (Colours::grey.withAlpha (0.8f));
                    g.setFont (FontOptions (12.0f));
                    g.drawFittedText (unitsText, unitsX, textY, unitsWidth, textHeight, Justification::centredLeft, 1, 0.8f);
                }
            }
        }
    }

    if (isSingleChannel && display->drawableChannels.size() > 0)
    {
        const int center = getTrackCenterY (*display->drawableChannels[0]);

        g.setColour (Colours::grey);
        g.setFont (FontOptions (13.0f));

        g.drawText ("MEAN:", 5, center + 40, 50, 12, Justification::centred, false);
        g.drawText (String (mean, 2), 5, center + 60, 50, 12, Justification::centred, false);

        g.drawText (String (rms, 2), 5, center + 110, 50, 12, Justification::centred, false);
        g.drawText ("RMS:", 5, center + 90, 50, 12, Justification::centred, false);

        if (x > 0)
        {
            g.setColour (Colours::darkgrey);
            g.fillPath (pointerPath, pointerPath.getTransformToScaleToFit (23, center + 140, 13, 13, true));
            g.drawText (String (y, 2), 5, center + 160, 50, 10, Justification::centred, false);
        }
    }
}

void LfpChannelDisplayInfo::updateXY (float x_, float y_)
{
    x = x_;
    y = y_;
}

void LfpChannelDisplayInfo::updateMeanAndRMS()
{
    const int currentChannel = display->getSingleChannelShown();

    if (currentChannel < 0)
        return;

    rms = canvasSplit->getRMS (currentChannel);
    mean = canvasSplit->getDisplayBufferMean (currentChannel);

    repaint();
}

void LfpChannelDisplayInfo::resized()
{
    syncEnableButtons();
}

int LfpChannelDisplayInfo::getTrackCenterY (const LfpChannelDisplay& channel) const
{
    return channel.getY() + channel.getHeight() / 2 - (isSingleChannel ? singleChannelInfoOffset : 0);
}

int LfpChannelDisplayInfo::getClosestDrawableTrackIndex (int y) const
{
    int closest = -1;
    int minDistance = std::numeric_limits<int>::max();

    for (int i = 0; i < display->drawableChannels.size(); ++i)
    {
        const int distance = abs (y - getTrackCenterY (*display->drawableChannels[i]));

        if (distance < minDistance)
        {
            minDistance = distance;
            closest = i;
        }
    }

    return closest;
}

void LfpChannelDisplayInfo::syncEnableButtons()
{
    const int numDrawableChannels = display->drawableChannels.size();

    if (enableButtons.size() != numDrawableChannels)
    {
        enableButtons.clear (true);

        for (int i = 0; i < numDrawableChannels; ++i)
        {
            auto* button = new UtilityButton ("");
            button->setRadius (5.0f);
            button->setEnabledState (true);
            button->setCorners (true, true, true, true);
            button->addListener (this);
            button->setClickingTogglesState (true);

            addAndMakeVisible (button);
            enableButtons.add (button);
        }
    }

    for (int i = 0; i < numDrawableChannels; ++i)
    {
        auto* channel = display->drawableChannels[i];
        auto* button = enableButtons[i];
        const bool shouldBeVisible = isSingleChannel || channel->getChannelHeight() >= 15;
        const int center = getTrackCenterY (*channel);

        button->setComponentID (String (channel->getChannelNumber()));
        button->setToggleState (channel->getEnabledState(), dontSendNotification);
        button->setVisible (shouldBeVisible);

        if (shouldBeVisible)
        {
            button->setBounds (getWidth() - 13,
                               center - (channelButtonSize / 2),
                               channelButtonSize,
                               channelButtonSize);
        }
    }
}

String LfpChannelDisplayInfo::getTooltip()
{
    if (display->options == nullptr)
        return {};

    const int trackIndex = getClosestDrawableTrackIndex (getMouseXYRelative().getY());

    if (trackIndex < 0)
        return {};

    const bool showChannelNumbers = display->options->getChannelNameState();
    auto* channel = display->drawableChannels[trackIndex];
    const String channelString = showChannelNumbers ? String (channel->getChannelNumber() + 1)
                                                    : channel->getName();

    return channelString;
}
