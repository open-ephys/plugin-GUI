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

#pragma mark - LfpChannelDisplayInfo -
// -------------------------------

LfpChannelDisplayInfo::LfpChannelDisplayInfo (LfpDisplaySplitter* canvas_, LfpDisplay* display_, LfpDisplayOptions* options_, int ch)
    : LfpChannelDisplay (canvas_, display_, options_, ch),
      x (-1.0f),
      y (-1.0f),
      rms (0.0f),
      mean (0.0f),
      isSingleChannel (false)
{
    enableButton = std::make_unique<UtilityButton> ("");
    enableButton->setRadius (5.0f);

    enableButton->setEnabledState (true);
    enableButton->setCorners (true, true, true, true);
    enableButton->addListener (this);
    enableButton->setClickingTogglesState (true);
    enableButton->setToggleState (true, dontSendNotification);

    addAndMakeVisible (enableButton.get());

    String svgString = "M302.189 329.126H196.105l55.831 135.993c3.889 9.428-.555 19.999-9.444 23.999l-49.165 21.427c-9.165 \
                       4-19.443-.571-23.332-9.714l-53.053-129.136-86.664 89.138C18.729 472.71 0 463.554 0 447.977V18.299C0 \
                       1.899 19.921-6.096 30.277 5.443l284.412 292.542c11.472 11.179 3.007 31.141-12.5 31.141z";

    pointerPath = Drawable::parseSVGPath (svgString);
}

void LfpChannelDisplayInfo::updateType (ContinuousChannel::Type type_)
{
    type = type_;
    typeStr = options->getTypeName (type);
    repaint();
}

void LfpChannelDisplayInfo::buttonClicked (Button* button)
{
    bool state = button->getToggleState();

    display->setEnabledState (state, chan, true);
}

void LfpChannelDisplayInfo::setEnabledState (bool state)
{
    enableButton->setToggleState (state, dontSendNotification);
}

void LfpChannelDisplayInfo::setSingleChannelState (bool state)
{
    isSingleChannel = state;
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
                zoomInfo.componentStartHeight = getChannelHeight();
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
            if (newHeight == getChannelHeight())
            {
                return;
            }

            // set channel heights for all channels
            for (int i = 0; i < display->getNumChannels(); ++i)
            {
                display->channels[i]->setChannelHeight (newHeight);
                display->channelInfo[i]->setChannelHeight (newHeight);
            }

            options->setSpreadSelection (newHeight, false, true); // update combobox

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
    int center = getHeight() / 2 - (isSingleChannel ? (75) : (0));
    const bool showChannelNumbers = options->getChannelNameState();

    // Draw the channel numbers
    if (isRecorded && recordingIsActive)
        g.setColour (Colours::red);
    else
        g.setColour (Colours::grey);

    const String channelString = (isChannelNumberHidden() ? ("--") : showChannelNumbers ? String (getChannelNumber() + 1)
                                                                                        : getName());
    bool isCentered = ! getEnabledButtonVisibility();

    if (isSingleChannel)
        g.setFont (FontOptions (16.0f).withStyle ("SemiBold"));
    else
        g.setFont (FontOptions (14.0f));

    g.drawText (channelString,
                showChannelNumbers ? 6 : 3,
                center - 4,
                getWidth(),
                12,
                isCentered ? Justification::centred : Justification::centredLeft,
                false);

    g.setColour (lineColour);
    g.fillRect (0, 0, 2, getHeight());

    if (getChannelTypeStringVisibility())
    {
        g.setFont (FontOptions (13.0f));
        g.drawText (typeStr, 5, center + 10, 50, 14, Justification::centred, false);
    }

    if (isSingleChannel)
    {
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
    rms = canvasSplit->getRMS (chan);
    mean = canvasSplit->getDisplayBufferMean (chan);

    repaint();
}

void LfpChannelDisplayInfo::resized()
{
    int center = getHeight() / 2 - (isSingleChannel ? (75) : (0));
    setEnabledButtonVisibility (getHeight() >= 16);

    if (getEnabledButtonVisibility())
    {
        enableButton->setBounds (getWidth() - 13, center - 5, 10, 10);
    }

    setChannelNumberIsHidden (getHeight() < 16 && (getDrawableChannelNumber() + 1) % 10 != 0);

    setChannelTypeStringVisibility (getHeight() > 34);
}

void LfpChannelDisplayInfo::setEnabledButtonVisibility (bool shouldBeVisible)
{
    enableButton->setVisible (shouldBeVisible);
}

bool LfpChannelDisplayInfo::getEnabledButtonVisibility()
{
    return enableButton->isVisible();
}

void LfpChannelDisplayInfo::setChannelTypeStringVisibility (bool shouldBeVisible)
{
    channelTypeStringIsVisible = shouldBeVisible;
}

bool LfpChannelDisplayInfo::getChannelTypeStringVisibility()
{
    return channelTypeStringIsVisible || isSingleChannel;
}

void LfpChannelDisplayInfo::setChannelNumberIsHidden (bool shouldBeHidden)
{
    channelNumberHidden = shouldBeHidden;
}

bool LfpChannelDisplayInfo::isChannelNumberHidden()
{
    return channelNumberHidden;
}

String LfpChannelDisplayInfo::getTooltip()
{
    const bool showChannelNumbers = options->getChannelNameState();
    const String channelString = (isChannelNumberHidden() ? ("--") : showChannelNumbers ? String (getChannelNumber() + 1)
                                                                                        : getName());

    return channelString;
}