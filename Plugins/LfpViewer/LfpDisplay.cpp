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

#include "LfpDisplay.h"
#include "EventDisplayInterface.h"
#include "LfpBitmapPlotter.h"
#include "LfpBitmapPlotterInfo.h"
#include "LfpChannelDisplay.h"
#include "LfpChannelDisplayInfo.h"
#include "LfpDisplayCanvas.h"
#include "LfpDisplayNode.h"
#include "LfpDisplayOptions.h"
#include "LfpTimescale.h"
#include "LfpViewport.h"
#include "PerPixelBitmapPlotter.h"
#include "ShowHideOptionsButton.h"
#include "SupersampledBitmapPlotter.h"

#include "ColourSchemes/DefaultColourScheme.h"
#include "ColourSchemes/LightBackgroundColourScheme.h"
#include "ColourSchemes/MonochromeGrayColourScheme.h"
#include "ColourSchemes/MonochromeGreenColourScheme.h"
#include "ColourSchemes/MonochromePurpleColourScheme.h"
#include "ColourSchemes/MonochromeYellowColourScheme.h"
#include "ColourSchemes/OELogoColourScheme.h"
#include "ColourSchemes/TropicalColourScheme.h"

#define MS_FROM_START Time::highResolutionTicksToSeconds (Time::getHighResolutionTicks() - start) * 1000

#include <cmath>
#include <math.h>
#include <numeric>
#include <vector>

using namespace LfpViewer;

#pragma mark - LfpDisplay -
// ---------------------------------------------------------------

LfpDisplay::LfpDisplay (LfpDisplaySplitter* c, Viewport* v)
    : singleChan (-1),
      canvasSplit (c),
      viewport (v),
      channelsReversed (false),
      channelsOrderedByDepth (false),
      displaySkipAmt (0),
      m_SpikeRasterPlottingFlag (false),
      lastBitmapIndex (0),
      lastFillFrom (-1),
      totalPixelsFilled (0)
{
    perPixelPlotter = std::make_unique<PerPixelBitmapPlotter> (this);
    supersampledPlotter = std::make_unique<SupersampledBitmapPlotter> (this);

    colourSchemeList.add (new DefaultColourScheme());
    colourSchemeList.add (new MonochromeGrayColourScheme());
    colourSchemeList.add (new MonochromeYellowColourScheme());
    colourSchemeList.add (new MonochromePurpleColourScheme());
    colourSchemeList.add (new MonochromeGreenColourScheme());
    colourSchemeList.add (new OELogoColourScheme());
    colourSchemeList.add (new TropicalColourScheme());
    colourSchemeList.add (new LightBackgroundColourScheme());

    plotter = perPixelPlotter.get();
    m_MedianOffsetPlottingFlag = false;

    activeColourScheme = 0;
    totalHeight = 0;
    colourGrouping = "1";

    //hand-built palette (used for event channels)
    channelColours.add (Colour (224, 185, 36));
    channelColours.add (Colour (214, 210, 182));
    channelColours.add (Colour (243, 119, 33));
    channelColours.add (Colour (186, 157, 168));
    channelColours.add (Colour (237, 37, 36));
    channelColours.add (Colour (179, 122, 79));
    channelColours.add (Colour (217, 46, 171));
    channelColours.add (Colour (217, 139, 196));
    channelColours.add (Colour (101, 31, 255));
    channelColours.add (Colour (141, 111, 181));
    channelColours.add (Colour (48, 117, 255));
    channelColours.add (Colour (184, 198, 224));
    channelColours.add (Colour (116, 227, 156));
    channelColours.add (Colour (150, 158, 155));
    channelColours.add (Colour (82, 173, 0));
    channelColours.add (Colour (125, 99, 32));

    range[0] = 1000; // headstage channels
    range[1] = 500; // aux channels
    range[2] = 500000; // adc channels

    scrollX = 0;
    scrollY = 0;

    addMouseListener (this, true);

    for (int ttlLine = 0; ttlLine < 8; ttlLine++)
    {
        eventDisplayEnabled[ttlLine] = true;
    }

    savedChannelState.insertMultiple (0, true, 10000); // max 10k channels

    numChans = 0;

    channelInfo = std::make_unique<LfpChannelDisplayInfo> (c, this);
}

LfpDisplay::~LfpDisplay()
{
}

int LfpDisplay::getNumChannels()
{
    return numChans;
}

const String& LfpDisplay::getColourGrouping()
{
    return colourGrouping;
}

void LfpDisplay::setColourGrouping (const String& grouping)
{
    colourGrouping = grouping;

    if (grouping.equalsIgnoreCase ("By Shank"))
    {
        // Set colour grouping to 1 to allow shank grouping
        // by using the shank index to determine the colour
        getColourSchemePtr()->setColourGrouping (1);
    }
    else
    {
        getColourSchemePtr()->setColourGrouping (grouping.getIntValue());
    }

    setColours(); // so that channel colours get re-assigned
}

ChannelColourScheme* LfpDisplay::getColourSchemePtr()
{
    return colourSchemeList[activeColourScheme];
}

void LfpDisplay::updateRange (int i)
{
    // Check if this is an AUX channel with auto-scaling enabled
    if (channels[i]->getType() == ContinuousChannel::Type::AUX && options->isAuxAutoScaleEnabled())
    {
        // Apply individual auto-scaling based on this channel's InputRange
        float rangeMin = canvasSplit->displayBuffer->channelMetadata[i].inputRangeMin;
        float rangeMax = canvasSplit->displayBuffer->channelMetadata[i].inputRangeMax;
        float autoRange = rangeMax - rangeMin;

        if (autoRange > 0.0f)
        {
            channels[i]->setRange (autoRange);
        }
        else
        {
            // Fall back to the default range for this type
            channels[i]->setRange (range[channels[i]->getType()]);
        }
    }
    else
    {
        // Use the standard range for the channel type
        channels[i]->setRange (range[channels[i]->getType()]);
    }
}

void LfpDisplay::setNumChannels (int newChannelCount)
{
    if (numChans > newChannelCount)
    {
        for (int i = newChannelCount; i < numChans; i++)
        {
            removeChildComponent (channels[i]);
        }

        channels.removeLast (numChans - newChannelCount);
    }

    totalHeight = 0;

    cachedDisplayChannelHeight = canvasSplit->getChannelHeight();

    if (newChannelCount > 0)
    {
        for (int i = 0; i < newChannelCount; i++)
        {
            LfpChannelDisplay* lfpChan;

            if (i >= numChans)
            {
                // std::cout << "ADDING NEW CHANNEL " << i << std::endl;

                lfpChan = new LfpChannelDisplay (canvasSplit, this, options, i);
                channels.add (lfpChan);
            }
            else
            {
                lfpChan = channels[i];
            }

            lfpChan->setChannelHeight (canvasSplit->getChannelHeight());

            if (! getSingleChannelState())
            {
                lfpChan->setEnabledState (savedChannelState[i]);
            }

            totalHeight += lfpChan->getChannelHeight();
        }
    }

    numChans = newChannelCount;
}

void LfpDisplay::setColours()
{
    if (drawableChannels.size() == 0)
        return;

    if (! getSingleChannelState())
    {
        for (int i = 0; i < drawableChannels.size(); i++)
        {
            if (colourGrouping.equalsIgnoreCase ("By Shank"))
            {
                auto* channel = drawableChannels[i];

                if (channel->hasGroupMetadata())
                {
                    const int colourIdx = (channel->getGroup() * 2) % 8;
                    channel->setColour (getColourSchemePtr()->getColourForIndex (colourIdx));
                }
                else
                {
                    /*
                    Legacy depth-based shank colouring ranges:
                    Shank 1: depth < 10000
                    Shank 2: 10000 ≤ depth < 20000
                    Shank 3: 20000 ≤ depth < 30000
                    Shank 4: depth ≥ 30000
                    */
                    const int depth = channel->getDepth();
                    const int colourIdx = depth < 10000 ? 0 : depth < 20000 ? 2
                                                          : depth < 30000   ? 4
                                                                            : 6;
                    channel->setColour (getColourSchemePtr()->getColourForIndex (colourIdx));
                }
            }
            else
            {
                drawableChannels[i]->setColour (getColourSchemePtr()->getColourForIndex (i));
            }
        }
    }
    else
    {
        if (colourGrouping.equalsIgnoreCase ("By Shank"))
        {
            auto* channel = drawableChannels[0];

            if (channel->hasGroupMetadata())
            {
                const int colourIdx = channel->getGroup();
                channel->setColour (getColourSchemePtr()->getColourForIndex (colourIdx));
            }
            else
            {
                const int depth = channel->getDepth();
                const int colourIdx = depth < 10000 ? 0 : depth < 20000 ? 2
                                                      : depth < 30000   ? 4
                                                                        : 6;
                channel->setColour (getColourSchemePtr()->getColourForIndex (colourIdx));
            }
        }
        else
        {
            drawableChannels[0]->setColour (getColourSchemePtr()->getColourForIndex (singleChan));
        }
    }

    channelInfo->repaint();

    if (displayIsPaused)
    {
        timeOffsetChanged = true;
        canRefresh = true;
    }
    else
    {
        canvasSplit->fullredraw = true;
        colourSchemeChanged = true;

        refresh();
    }
}

void LfpDisplay::setActiveColourSchemeIdx (int index)
{
    activeColourScheme = index;
}

int LfpDisplay::getActiveColourSchemeIdx()
{
    return activeColourScheme;
}

int LfpDisplay::getNumColourSchemes()
{
    return colourSchemeList.size();
}

Array<String> LfpDisplay::getColourSchemeNameArray()
{
    Array<String> nameList;
    for (auto scheme : colourSchemeList)
        nameList.add (scheme->getName());

    return nameList;
}

int LfpDisplay::getTotalHeight()
{
    return totalHeight;
}

int LfpDisplay::getChannelBitmapYOrigin() const
{
    return channelBitmapYOrigin;
}

Rectangle<int> LfpDisplay::getChannelBitmapWindowBounds()
{
    const int bitmapWidth = jmax (1, getWidth() - canvasSplit->leftmargin);
    const int componentHeight = jmax (1, getHeight());

    if (viewport == nullptr || viewport->getViewHeight() <= 0)
        return Rectangle<int> (0, 0, bitmapWidth, jmin (componentHeight, 10));

    const int channelHeightForMargin = drawableChannels.size() > 0 ? drawableChannels[0]->getHeight() : 30;
    const int margin = jmax (64, channelHeightForMargin * 2);
    const int viewTop = jlimit (0, componentHeight, viewport->getViewPositionY());
    const int viewBottom = jlimit (0, componentHeight, viewTop + jmax (1, viewport->getViewHeight()));

    if (! lfpChannelBitmap.isNull()
        && lfpChannelBitmap.getWidth() == bitmapWidth
        && channelBitmapYOrigin >= 0
        && channelBitmapYOrigin <= viewTop
        && channelBitmapYOrigin + lfpChannelBitmap.getHeight() >= viewBottom
        && channelBitmapYOrigin + lfpChannelBitmap.getHeight() <= componentHeight)
    {
        return Rectangle<int> (0, channelBitmapYOrigin, bitmapWidth, lfpChannelBitmap.getHeight());
    }

    const int bitmapTop = jmax (0, viewTop - margin);
    const int bitmapBottom = jmin (componentHeight, viewBottom + margin);

    return Rectangle<int> (0, bitmapTop, bitmapWidth, jmax (1, bitmapBottom - bitmapTop));
}

void LfpDisplay::ensureChannelBitmapForVisibleArea()
{
    const auto bitmapBounds = getChannelBitmapWindowBounds();

    if (lfpChannelBitmap.isNull()
        || lfpChannelBitmap.getWidth() != bitmapBounds.getWidth()
        || lfpChannelBitmap.getHeight() != bitmapBounds.getHeight()
        || channelBitmapYOrigin != bitmapBounds.getY())
    {
        lfpChannelBitmap = Image (Image::ARGB,
                                  bitmapBounds.getWidth(),
                                  bitmapBounds.getHeight(),
                                  true,
                                  SoftwareImageType());
        channelBitmapYOrigin = bitmapBounds.getY();
        canvasSplit->fullredraw = true;
    }
}

void LfpDisplay::restoreViewPosition()
{
    if (! getSingleChannelState())
        viewport->setViewPosition (scrollX, scrollY);
}

void LfpDisplay::resized()
{
    int totalHeight = 0;

    //LOGD(" !! LFP DISPLAY RESIZED TO: ", getWidth(), " pixels.");

    if (getWidth() == 0)
    {
        //LOGD("   ::: Not visible, returning.");
        return;
    }

    int64 start = Time::getHighResolutionTicks();

    for (int i = 0; i < drawableChannels.size(); i++)
    {
        LfpChannelDisplay* disp = drawableChannels[i];

        if (disp->getHidden())
            continue;

        disp->setBounds (canvasSplit->leftmargin,
                         totalHeight - (disp->getChannelOverlap() * canvasSplit->channelOverlapFactor) / 2,
                         getWidth() - canvasSplit->leftmargin,
                         disp->getChannelHeight() + (disp->getChannelOverlap() * canvasSplit->channelOverlapFactor));

        totalHeight += disp->getChannelHeight();
    }

    channelInfo->setBounds (2, 0, jmax (0, canvasSplit->leftmargin - 2), getHeight());
    channelInfo->resized();

    if (! getSingleChannelState())
    {
        viewport->setViewPosition (scrollX, scrollY);
        //std::cout << "Setting view position to " << scrollY << std::endl;
    }
    else
    {
        //std::cout << "Setting view position for single channel " << std::endl;
        viewport->setViewPosition (0, 0);
    }

    canvasSplit->fullredraw = true;
    ensureChannelBitmapForVisibleArea();

    //LOGD("    RESIZED IN: ", MS_FROM_START, " milliseconds");
    start = Time::getHighResolutionTicks();

    if (displayIsPaused)
    {
        timeOffsetChanged = true;
        canRefresh = true;
    }

    refresh();

    //LOGD("    REFRESHED IN: ", MS_FROM_START, " milliseconds");
}

void LfpDisplay::paint (Graphics& g)
{
    if (lfpChannelBitmap.isNull())
        return;

    g.drawImageAt (lfpChannelBitmap, canvasSplit->leftmargin, channelBitmapYOrigin);
}

void LfpDisplay::sync()
{
    if (! displayIsPaused)
    {
        lastBitmapIndex = 0;
        totalPixelsFilled = 0;
    }
}

void LfpDisplay::refresh()
{
    if (numChans == 0)
        return;

    ensureChannelBitmapForVisibleArea();

    int totalXPixels = lfpChannelBitmap.getWidth();
    int totalYPixels = lfpChannelBitmap.getHeight();

    int topBorder = channelBitmapYOrigin;
    int bottomBorder = topBorder + totalYPixels;

    //std::cout << "refresh display " << std::endl;

    // X-bounds of this update
    int fillfrom = canvasSplit->lastScreenBufferIndex[0];
    int fillto = canvasSplit->screenBufferIndex[0];

    // If the display is paused, check and draw for backwards scrolling
    if (displayIsPaused)
    {
        // Check if the time offset has changed or if a full redraw is needed
        if ((timeOffsetChanged && canRefresh)
            || canvasSplit->fullredraw)
        {
            //std::cout << "Time offset: " << timeOffset << std::endl;

            int playhead = pausePoint + int (timeOffset);
            int rightEdge = totalXPixels;
            int maxScreenBufferIndex = canvasSplit->screenBufferIndex[0];

            timeOffsetChanged = false;
            canRefresh = false;

            //std::cout << "playhead: " << playhead << ", right edge: " << rightEdge << ", maxScreenBufferIndex: " << maxScreenBufferIndex << std::endl;

            lfpChannelBitmap.clear (Rectangle<int> (0, 0, totalXPixels, totalYPixels));

            for (int i = 0; i < drawableChannels.size(); i++)
            {
                int componentTop = drawableChannels[i]->getY();
                int componentBottom = drawableChannels[i]->getBottom();

                if ((topBorder <= componentBottom && bottomBorder >= componentTop)) // only draw things that are visible
                {
                    drawableChannels[i]->pxPaintHistory (playhead, rightEdge, maxScreenBufferIndex);
                }
            }

            channelInfo->repaint();

            repaint();

            canvasSplit->fullredraw = false;

            return;
        }
        else
        {
            return;
        }
    }

    //if (lastFillFrom == fillfrom && !canvasSplit->fullredraw)
    //     return;

    int totalPixelsToFill = 0;

    if (fillto > fillfrom)
    {
        totalPixelsToFill = fillto - fillfrom;
    }
    else if (fillto < fillfrom)
    {
        totalPixelsToFill = canvasSplit->screenBufferWidth - fillfrom + fillto;
    }

    //if (totalPixelsToFill > 0)
    //    std::cout << fillfrom << " : " << fillto << " ::: " << "totalPixelsToFill: " << totalPixelsToFill << std::endl;

    int fillfrom_local, fillto_local;
    int channelFillFrom = fillfrom;
    int channelFillTo = fillto;

    if (canvasSplit->fullredraw)
    {
        int playhead = lastBitmapIndex;
        int rightEdge = totalXPixels;
        int maxScreenBufferIndex = canvasSplit->screenBufferIndex[0];

        //std::cout << "playhead: " << playhead << ", right edge: " << rightEdge << ", maxScreenBufferIndex: " << maxScreenBufferIndex << std::endl;

        lfpChannelBitmap.clear (Rectangle<int> (0, 0, totalXPixels, totalYPixels));

        for (int i = 0; i < drawableChannels.size(); i++)
        {
            int componentTop = drawableChannels[i]->getY();
            int componentBottom = drawableChannels[i]->getBottom();

            if ((topBorder <= componentBottom && bottomBorder >= componentTop)) // only draw things that are visible
            {
                drawableChannels[i]->pxPaintHistory (playhead, rightEdge, maxScreenBufferIndex);
            }
        }

        channelInfo->repaint();

        canvasSplit->fullredraw = false;

        repaint();

        /* if (colourSchemeChanged)
        {
            colourSchemeChanged = false;
            lastBitmapIndex += totalPixelsToFill;
            lastBitmapIndex %= totalXPixels;
        }*/

        return;
    }
    else
    {
        if (totalPixelsToFill >= totalXPixels)
        {
            channelFillFrom = (fillto - totalXPixels + canvasSplit->screenBufferWidth) % canvasSplit->screenBufferWidth;
            fillfrom_local = 0;
            fillto_local = totalXPixels;
        }
        else
        {
            fillfrom_local = lastBitmapIndex;
            fillto_local = (lastBitmapIndex + totalPixelsToFill) % totalXPixels;
        }

        //if (fillto != 0)
        //{
        //    std::cout << fillfrom << " : " << fillto << " ::: " <<
        //        fillfrom_local << " : " << fillto_local << " :: " << totalPixelsToFill << " ::: " << totalXPixels << std::endl;
        // }

        if (fillfrom_local < fillto_local)
        {
            int x1 = fillfrom_local;
            int x2 = (fillto_local - fillfrom_local) + 2;
            lfpChannelBitmap.clear (Rectangle<int> (x1, 0, x2, totalYPixels));
            //std::cout << "Clearing from " << x1 << " to " << x1 + x2 << " (" << totalYPixels << "ypix)" << std::endl;
        }
        else if (fillfrom_local > fillto_local)
        {
            int x1 = fillfrom_local;
            int x2 = totalXPixels - fillfrom_local;
            int x3 = 0;
            int x4 = fillto_local + 2;
            //std::cout << "Clearing from " << x1 << " to " << x1 + x2 << " (" << totalYPixels << "ypix)" << std::endl;
            lfpChannelBitmap.clear (Rectangle<int> (x1, 0, x2, totalYPixels));
            //std::cout << "Clearing from " << x3 << " to " << x3 + x4 << " (" << totalYPixels << "ypix)" << std::endl;
            lfpChannelBitmap.clear (Rectangle<int> (x3, 0, x4, totalYPixels));
        }
        else
        {
            return; // no change, do nothing
        }
    }

    for (int i = 0; i < drawableChannels.size(); i++)
    {
        int componentTop = drawableChannels[i]->getY();
        int componentBottom = drawableChannels[i]->getBottom();

        if ((topBorder <= componentBottom && bottomBorder >= componentTop)) // only draw things that are visible
        {
            drawableChannels[i]->ifrom = channelFillFrom; // canvasSplit->lastScreenBufferIndex[0];
            drawableChannels[i]->ito = channelFillTo; // canvasSplit->screenBufferIndex[0];
            drawableChannels[i]->ifrom_local = fillfrom_local;
            drawableChannels[i]->ito_local = fillto_local;
            drawableChannels[i]->pxPaint(); // draws to lfpChannelBitmap
        }
    }

    repaint();

    totalPixelsFilled += totalPixelsToFill;

    if (totalPixelsFilled > (totalXPixels / (2 * canvasSplit->timebase)) && singleChan != -1)
    {
        channelInfo->updateMeanAndRMS();
        totalPixelsFilled = 0;
    }

    lastBitmapIndex += totalPixelsToFill;
    lastBitmapIndex %= lfpChannelBitmap.getWidth();

    lastFillFrom = fillfrom;
}

void LfpDisplay::setRange (float r, ContinuousChannel::Type type)
{
    range[type] = r;

    if (channels.size() > 0)
    {
        for (int i = 0; i < numChans; i++)
        {
            if (channels[i]->getType() == type)
            {
                channels[i]->setRange (range[type]);
            }
        }

        if (displayIsPaused)
        {
            timeOffsetChanged = true;
            canRefresh = true;

            refresh();
        }
    }
}

void LfpDisplay::setAutoRangeForAuxChannels()
{
    if (canvasSplit->displayBuffer == nullptr || channels.size() == 0)
        return;

    // Apply individual ranges to each AUX channel based on their InputRange
    for (int i = 0; i < numChans; i++)
    {
        if (channels[i]->getType() == ContinuousChannel::Type::AUX)
        {
            // Get the InputRange from the channel metadata
            if (i < canvasSplit->displayBuffer->channelMetadata.size())
            {
                float rangeMin = canvasSplit->displayBuffer->channelMetadata[i].inputRangeMin;
                float rangeMax = canvasSplit->displayBuffer->channelMetadata[i].inputRangeMax;
                float autoRange = rangeMax - rangeMin;

                if (autoRange > 0.0f)
                {
                    channels[i]->setRange (autoRange);
                }
                else
                {
                    // Fall back to the default range for this type
                    channels[i]->setRange (range[ContinuousChannel::Type::AUX]);
                }
            }
        }
    }

    if (displayIsPaused)
    {
        timeOffsetChanged = true;
        canRefresh = true;
        refresh();
    }
}

int LfpDisplay::getRange()
{
    return getRange (options->getSelectedType());
}

int LfpDisplay::getRange (ContinuousChannel::Type type)
{
    for (int i = 0; i < numChans; i++)
    {
        if (channels[i]->getType() == type)
            return channels[i]->getRange();
    }
    return 0;
}

void LfpDisplay::setChannelHeight (int r, bool resetSingle)
{
    if (! getSingleChannelState())
        cachedDisplayChannelHeight = r;

    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setChannelHeight (r);
    }

    if (singleChan == -1)
    {
        int overallHeight = drawableChannels.size() * getChannelHeight();

        setSize (getWidth(), overallHeight);
    }
}

void LfpDisplay::setInputInverted (bool isInverted)
{
    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setInputInverted (isInverted);
    }
}

Array<bool> LfpDisplay::getInputInverted()
{
    Array<bool> invertedState;

    for (int i = 0; i < numChans; i++)
    {
        invertedState.add (channels[i]->getInputInverted());
    }

    return invertedState;
}

void LfpDisplay::setDrawMethod (bool isDrawMethod)
{
    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setDrawMethod (isDrawMethod);
    }

    if (isDrawMethod)
    {
        plotter = supersampledPlotter.get();
    }
    else
    {
        plotter = perPixelPlotter.get();
    }

    resized();
}

int LfpDisplay::getChannelHeight()
{
    if (drawableChannels.size() > 0)
        return drawableChannels[0]->getChannelHeight();
    else
        return 0;
}

void LfpDisplay::cacheNewChannelHeight (int r)
{
    cachedDisplayChannelHeight = r;
}

bool LfpDisplay::getChannelsReversed()
{
    return channelsReversed;
}

void LfpDisplay::setChannelsReversed (bool state)
{
    channelsReversed = state;

    rebuildDrawableChannelsList();
}

void LfpDisplay::orderChannelsByDepth (bool state)
{
    channelsOrderedByDepth = state;

    rebuildDrawableChannelsList();
}

bool LfpDisplay::shouldOrderChannelsByDepth()
{
    return channelsOrderedByDepth;
}

int LfpDisplay::getChannelDisplaySkipAmount()
{
    return displaySkipAmt;
}

void LfpDisplay::setChannelDisplaySkipAmount (int skipAmt)
{
    displaySkipAmt = skipAmt;

    if (! getSingleChannelState())
        rebuildDrawableChannelsList();

    canvasSplit->redraw();
}

void LfpDisplay::setScrollPosition (int x, int y)
{
    scrollX = x;
    scrollY = y;
}

bool LfpDisplay::getMedianOffsetPlotting()
{
    return m_MedianOffsetPlottingFlag;
}

void LfpDisplay::setMedianOffsetPlotting (bool isEnabled)
{
    m_MedianOffsetPlottingFlag = isEnabled;
}

bool LfpDisplay::getSpikeRasterPlotting()
{
    return m_SpikeRasterPlottingFlag;
}

void LfpDisplay::setSpikeRasterPlotting (bool isEnabled)
{
    m_SpikeRasterPlottingFlag = isEnabled;
}

float LfpDisplay::getSpikeRasterThreshold()
{
    return m_SpikeRasterThreshold;
}

void LfpDisplay::setSpikeRasterThreshold (float thresh)
{
    m_SpikeRasterThreshold = thresh;
}

void LfpDisplay::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    //std::cout << "Mouse wheel " <<  e.mods.isCommandDown() << "  " << wheel.deltaY << std::endl;
    //TODO Changing ranges with the wheel is currently broken. With multiple ranges, most
    //of the wheel range code needs updating

    //std::cout << "Y: " << scrollY << std::endl;

    if (e.mods.isCommandDown() && singleChan == -1) // CTRL + scroll wheel -> change channel spacing
    {
        int h = getChannelHeight();
        int hdiff = 0;

        // std::cout << wheel.deltaY << std::endl;

        if (wheel.deltaY > 0)
        {
            hdiff = 2;
        }
        else
        {
            if (h > 5)
                hdiff = -2;
        }

        if (abs (h) > 100) // accelerate scrolling for large ranges
            hdiff *= 3;

        int newHeight = h + hdiff;

        // constrain the spread resizing to max and min values;
        if (newHeight < trackZoomInfo.minZoomHeight)
        {
            newHeight = trackZoomInfo.minZoomHeight;
            hdiff = 0;
        }
        else if (newHeight > trackZoomInfo.maxZoomHeight)
        {
            newHeight = trackZoomInfo.maxZoomHeight;
            hdiff = 0;
        }

        setChannelHeight (newHeight);
        int oldX = viewport->getViewPositionX();
        int oldY = viewport->getViewPositionY();

        setBounds (0, 0, getWidth() - 0, getChannelHeight() * drawableChannels.size()); // update height so that the scrollbar is correct

        int mouseY = e.getMouseDownY(); // should be y pos relative to inner viewport (0,0)
        int scrollBy = (mouseY / h) * hdiff * 2; // compensate for motion of point under current mouse position
        viewport->setViewPosition (oldX, oldY + scrollBy); // set back to previous position plus offset

        options->setSpreadSelection (newHeight); // update combobox
    }
    else
    {
        if (e.mods.isAltDown()) // ALT + scroll wheel -> change channel range (was SHIFT but that clamps wheel.deltaY to 0 on OSX for some reason..)
        {
            auto chanType = options->getSelectedType();

            if (chanType == ContinuousChannel::AUX && options->isAuxAutoScaleEnabled())
                return;

            int h = getRange();

            int step = options->getRangeStep (chanType);

            // std::cout << wheel.deltaY << std::endl;

            if (wheel.deltaY > 0)
            {
                setRange (h + step, chanType);
            }
            else
            {
                if (h > step + 1)
                    setRange (h - step, chanType);
            }

            options->setRangeSelection (h); // update combobox
        }
        else // just scroll
        {
            //  passes the event up to the viewport so the screen scrolls
            if (viewport != nullptr && e.eventComponent == this) // passes only if it's not a listening event
            {
                viewport->mouseWheelMove (e.getEventRelativeTo (viewport), wheel);
            }
        }
    }

    if (! getSingleChannelState())
    {
        scrollX = viewport->getViewPositionX();
        scrollY = viewport->getViewPositionY();
    }
}

void LfpDisplay::toggleSingleChannel (LfpChannelDisplay* drawableChannel)
{
    if (! getSingleChannelState())
    {
        singleChan = drawableChannel->getChannelNumber();

        rebuildDrawableChannelsList();
    }
    else
    {
        channelInfo->setSingleChannelState (false);

        singleChan = -1;

        setChannelHeight (cachedDisplayChannelHeight);

        reactivateChannels();
        rebuildDrawableChannelsList();
    }
}

void LfpDisplay::reactivateChannels()
{
    for (int n = 0; n < channels.size(); n++)
        setEnabledState (savedChannelState[n], n, true);
}

void LfpDisplay::rebuildDrawableChannelsList()
{
    if (getSingleChannelState())
    {
        int newHeight = viewport->getHeight();

        int channelIndex = -1;

        for (int i = 0; i < channels.size(); i++)
        {
            if (channels[i]->getChannelNumber() == singleChan)
            {
                channelIndex = i;
                break;
            }
        }

        if (channelIndex > -1)
        {
            if (drawableChannels.size() != 1 || numChans == 1) // if we haven't already gone through this ordeal
            {
                auto* lfpChannel = channels[channelIndex];

                removeAllChildren();

                // disable unused channels
                for (int i = 0; i < drawableChannels.size(); i++)
                {
                    if (drawableChannels[i] != lfpChannel)
                        drawableChannels[i]->setEnabledState (false);
                }

                // update drawableChannels, give only the single channel to focus on
                drawableChannels = Array<LfpChannelDisplay*>();
                drawableChannels.add (lfpChannel);

                lfpChannel->setEnabledState (true);
                channelInfo->setSingleChannelState (true);

                addAndMakeVisible (lfpChannel);
                addAndMakeVisible (channelInfo.get());

                // set channel height and position (so that we allocate the smallest
                // necessary image size for drawing)
                setChannelHeight (newHeight, false);

                setSize (getWidth(), getChannelHeight());
                resized();

                viewport->setViewPosition (0, 0);

                setColours();

                // this guards against an exception where the editor sets the drawable samplerate
                // before the lfpDisplay is fully initialized
                if (getHeight() > 0 && getWidth() > 0)
                {
                    canvasSplit->resizeToChannels();
                }
            }

            return;
        }
        else
        {
            reactivateChannels();

            singleChan = -1; // our channel no longer exists
        }
    }

    removeAllChildren(); // start with clean slate

    Array<LfpChannelDisplay*> channelsToDraw; // all visible channels will be added to this array
    Array<int> filteredChannels;
    if (canvasSplit->displayBuffer)
    {
        filteredChannels = canvasSplit->getFilteredChannels();
    }
    // iterate over all channels and select drawable ones
    for (int i = 0, drawableChannelNum = 0, filterChannelIndex = 0; i < channels.size(); i++)
    {
        //std::cout << "Checking for hidden channels" << std::endl;
        int channelNumber = filteredChannels.size() ? canvasSplit->displayBuffer->channelMetadata[i].description.getIntValue() : -1;
        //the filter list can have channels that aren't selected for acqusition; this skips those filtered channels
        while (filterChannelIndex < filteredChannels.size() && channelNumber > filteredChannels[filterChannelIndex])
        {
            filterChannelIndex++;
        }
        if (filteredChannels.size() == 0 || (filterChannelIndex < filteredChannels.size() && channelNumber == filteredChannels[filterChannelIndex]))
        {
            if (displaySkipAmt == 0 || ((filteredChannels.size() ? filterChannelIndex : i) % displaySkipAmt == 0)) // no skips, add all channels
            {
                channels[i]->setHidden (false);
                channels[i]->setDrawableChannelNumber (drawableChannelNum++);

                channelsToDraw.add (channels[i]);

                addAndMakeVisible (channels[i]);
            }
            filterChannelIndex++;
        }
        else // skip some channels
        {
            channels[i]->setHidden (true);
        }
    }

    if (channelsOrderedByDepth && channelsToDraw.size() > 0)
    {
        LOGD ("Sorting channels by depth.");

        const int numChannels = channelsToDraw.size();

        std::vector<float> depths (numChannels);
        std::vector<float> xposValues (numChannels);
        std::vector<bool> hasYposMetadata (numChannels);
        std::vector<bool> hasXposMetadata (numChannels);
        std::vector<int> groups (numChannels);
        std::vector<bool> hasGroupMetadata (numChannels);

        bool allSame = true;
        bool anyYposMetadata = false;
        bool anyXposMetadata = false;
        bool anyGroupMetadata = false;
        float last = channelsToDraw[0]->getDepth();

        for (int i = 0; i < channelsToDraw.size(); i++)
        {
            auto* channel = channelsToDraw[i];
            const float depth = channel->getDepth();

            if (depth != last)
                allSame = false;

            depths[i] = depth;
            xposValues[i] = channel->getXpos();
            hasYposMetadata[i] = channel->hasYposMetadata();
            hasXposMetadata[i] = channel->hasXposMetadata();
            groups[i] = channel->getGroup();
            hasGroupMetadata[i] = channel->hasGroupMetadata();

            anyYposMetadata = anyYposMetadata || hasYposMetadata[i];
            anyXposMetadata = anyXposMetadata || hasXposMetadata[i];
            anyGroupMetadata = anyGroupMetadata || hasGroupMetadata[i];
            last = depth;
        }

        const bool positionMetadataAvailable = anyYposMetadata && anyXposMetadata;
        const bool groupMetadataAvailable = anyGroupMetadata;

        if (! groupMetadataAvailable && allSame && ! anyYposMetadata)
        {
            LOGD ("No depth info found.");
        }
        else
        {
            std::vector<int> V (numChannels);

            std::iota (V.begin(), V.end(), 0); //Initializing
            sort (V.begin(), V.end(), [&] (int i, int j)
                  {
                      const float depthDiff = depths[i] - depths[j];
                      const float depthEpsilon = 1.0e-3f;

                      if (groupMetadataAvailable && groups[i] != groups[j])
                          return groups[i] < groups[j];

                      if (std::abs (depthDiff) >= depthEpsilon)
                          return depths[i] < depths[j];

                      if (positionMetadataAvailable)
                          return xposValues[i] < xposValues[j];

                      return i < j; // deterministic fallback
                  });

            Array<LfpChannelDisplay*> orderedDrawableChannels;

            for (int i = 0; i < channelsToDraw.size(); i++)
            {
                // re-order by depth
                orderedDrawableChannels.add (channelsToDraw[V[i]]);
            }

            channelsToDraw = orderedDrawableChannels;
        }
    }

    drawableChannels = Array<LfpChannelDisplay*>(); // this is what will determine the actual channel order

    if (getChannelsReversed())
    {
        LOGD ("Reversing channel order.");
        for (int i = channelsToDraw.size() - 1; i >= 0; --i)
        {
            drawableChannels.add (channelsToDraw[i]);
        }
    }
    else
    {
        for (int i = 0; i < channelsToDraw.size(); ++i)
        {
            drawableChannels.add (channelsToDraw[i]);
        }
    }

    addAndMakeVisible (channelInfo.get());
    channelInfo->setSingleChannelState (false);

    // this guards against an exception where the editor sets the drawable samplerate
    // before the lfpDisplay is fully initialized
    if (getHeight() > 0 && getWidth() > 0)
    {
        canvasSplit->resizeToChannels();
    }

    setColours();

    resized();

    //LOGD("Finished standard channel rebuild.");
}

LfpBitmapPlotter* const LfpDisplay::getPlotterPtr() const
{
    return plotter;
}

bool LfpDisplay::getSingleChannelState()
{
    return singleChan >= 0;
}

int LfpDisplay::getSingleChannelShown()
{
    return singleChan;
}

void LfpDisplay::setSingleChannelView (int chan)
{
    singleChan = chan;
}

void LfpDisplay::pause (bool shouldPause)
{
    displayIsPaused = shouldPause;

    options->setPausedState (shouldPause);

    canvasSplit->pause (shouldPause);

    if (! shouldPause)
    {
        timeOffset = 0.0f;
        sync();
        canvasSplit->fullredraw = true;
        //refresh();
    }
    else
    {
        pausePoint = lastBitmapIndex;
    }
}

void LfpDisplay::timerCallback()
{
    canRefresh = true;
}

void LfpDisplay::setTimeOffset (float offset)
{
    timeOffset = offset;
    timeOffsetChanged = true;
    canRefresh = true;

    refresh();

    //if (offset != timeOffset)
    //{

    // }

    //LOGD("Time offset: ", offset);
}

bool LfpDisplay::isPaused()
{
    return displayIsPaused;
}

void LfpDisplay::mouseDown (const MouseEvent& event)
{
    if (drawableChannels.isEmpty())
    {
        return;
    }

    //int y = event.getMouseDownY(); //relative to each channel pos
    MouseEvent canvasevent = event.getEventRelativeTo (viewport);
    int y = canvasevent.getMouseDownY() + viewport->getViewPositionY(); // need to account for scrolling
    int x = canvasevent.getMouseDownX();

    int dist = 0;
    int mindist = 10000;
    int closest = 5;
    float chanRange = getRange();

    for (int n = 0; n < drawableChannels.size(); n++) // select closest instead of relying on eventComponent
    {
        drawableChannels[n]->deselect();

        int cpos = (drawableChannels[n]->getY() + (drawableChannels[n]->getHeight() / 2));
        dist = int (abs (y - cpos));

        //std::cout << "Mouse down at " << y << " pos is "<< cpos << " n: " << n << "  dist " << dist << std::endl;

        if (dist < mindist)
        {
            mindist = dist - 1;
            closest = n;
            chanRange = drawableChannels[n]->getRange();
        }
    }

    //std::cout << "Closest channel" << closest << std::endl;

    drawableChannels[closest]->select();
    options->setSelectedType (drawableChannels[closest]->getType());

    if (event.mods.isRightButtonDown())
    { // if right click
        PopupMenu channelMenu = channels[closest]->getOptions();
        const int result = channelMenu.show();
        drawableChannels[closest]->changeParameter (result);
    }
    else // if left click
    {
        if (event.getNumberOfClicks() == 2)
        {
            channelInfo->updateXY (0, 0); // reset mouse-selected point
            toggleSingleChannel (drawableChannels[closest]);
        }
        else if (getSingleChannelState()) // show info for point that was selected
        {
            channelInfo->updateXY (
                float (x) / getWidth() * canvasSplit->timebase,
                (-(float (y) - viewport->getViewPositionY()) / viewport->getViewHeight() * float (chanRange)) + float (chanRange / 2));
        }
    }

    canvasSplit->select();

    canvasSplit->fullredraw = true; //issue full redraw

    refresh();
}

bool LfpDisplay::setEventDisplayState (int ttlLine, bool state)
{
    eventDisplayEnabled[ttlLine] = state;
    return eventDisplayEnabled[ttlLine];
}

bool LfpDisplay::getEventDisplayState (int ttlLine)
{
    return eventDisplayEnabled[ttlLine];
}

void LfpDisplay::setEnabledState (bool state, int chan, bool updateSaved)
{
    if (chan < numChans)
    {
        channels[chan]->setEnabledState (state);
        channelInfo->resized();
        channelInfo->repaint();

        if (updateSaved)
            savedChannelState.set (chan, state);
    }
}

bool LfpDisplay::getEnabledState (int chan)
{
    if (chan < numChans)
    {
        return channels[chan]->getEnabledState();
    }

    return false;
}
