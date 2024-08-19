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

#include "LfpChannelDisplay.h"
#include "ColourSchemes/ChannelColourScheme.h"
#include "EventDisplayInterface.h"
#include "LfpBitmapPlotter.h"
#include "LfpBitmapPlotterInfo.h"
#include "LfpChannelDisplayInfo.h"
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

LfpChannelDisplay::LfpChannelDisplay (LfpDisplaySplitter* c, LfpDisplay* d, LfpDisplayOptions* o, int channelNumber)
    : canvasSplit (c), display (d), options (o), isSelected (false), isRecorded (false), recordingIsActive (false), chan (channelNumber), name (""), drawableChan (channelNumber), channelOverlap (300), channelHeight (30), range (250.0f), isEnabled (true), inputInverted (false), canBeInverted (true), drawMethod (false), isHidden (false), ifrom (0), ito (0), ifrom_local (0), ito_local (0)
{
    name = String (channelNumber + 1); // default is to make the channelNumber the name

    channelHeightFloat = (float) channelHeight;

    channelFont = FontOptions (channelHeight * 0.6);

    lineColour = Colour (255, 255, 255);

    type = options->getChannelType (channelNumber);
    typeStr = options->getTypeName (type);
}

LfpChannelDisplay::~LfpChannelDisplay()
{
}

void LfpChannelDisplay::resized()
{
    // all of this will likely need to be moved into the sharedLfpDisplay image in the lfpDisplay, not here
    // now that the complete height is know, the sharedLfpDisplay image that we'll draw the pixel-wise lfp plot to needs to be resized
    //lfpChannelBitmap = Image(Image::ARGB, getWidth(), getHeight(), false);
}

void LfpChannelDisplay::updateType (ContinuousChannel::Type type_)
{
    type = type_;
    typeStr = options->getTypeName (type);
}

void LfpChannelDisplay::setType (ContinuousChannel::Type type_)
{
    type = type_;
    typeStr = options->getTypeName (type);
}

void LfpChannelDisplay::setEnabledState (bool state)
{
    /*if (state)
        std::cout << "ENABLING CHANNEL " << chan << std::endl;
    else
        std::cout << "DISABLING CHANNEL " << chan << std::endl;*/

    isEnabled = state;
}

void LfpChannelDisplay::setHidden (bool isHidden_)
{
    isHidden = isHidden_;
}

void LfpChannelDisplay::pxPaint()
{
    if (! isEnabled || isHidden || getWidth() == 0)
    {
        return; // return early if THIS display is not enabled
    }

    Image::BitmapData bdLfpChannelBitmap (display->lfpChannelBitmap, Image::BitmapData::readWrite);

    int center = getHeight() / 2;

    //int ifrom = canvasSplit->lastScreenBufferIndex[0]; // base everything on the first channel
    //int ito = screenBufferIndex;

    // max and min of channel in absolute px coords for event displays etc - actual data might be drawn outside of this range
    int jfrom_wholechannel = (int) (getY() + center - channelHeight / 2) + 1 + 0;
    int jto_wholechannel = (int) (getY() + center + channelHeight / 2) - 0;

    // max and min of channel, this is the range where actual data is drawn
    int jfrom_wholechannel_clip = (int) (getY() + center - (channelHeight) *canvasSplit->channelOverlapFactor) + 1;
    int jto_wholechannel_clip = (int) (getY() + center + (channelHeight) *canvasSplit->channelOverlapFactor) - 0;

    if (jfrom_wholechannel < 0)
    {
        jfrom_wholechannel = 0;
    };
    if (jto_wholechannel >= display->lfpChannelBitmap.getHeight())
    {
        jto_wholechannel = display->lfpChannelBitmap.getHeight() - 1;
    };

    // draw most recent drawn sample position
    if (ito_local < display->lfpChannelBitmap.getWidth() - 1)
    {
        for (int k = jfrom_wholechannel; k <= jto_wholechannel; k += 2) // draw line
        {
            bdLfpChannelBitmap.setPixelColour (ito_local + 1, k, Colours::yellow);
        }
    }

    bool clipWarningHi = false; // keep track if something clipped in the display, so we can draw warnings after the data pixels are done
    bool clipWarningLo = false;

    bool saturateWarningHi = false; // similar, but for saturating the amplifier, not just the display - make this warning very visible
    bool saturateWarningLo = false;

    int stepSize = 1;
    int from = 0;
    int to = 0;
    int endIndex;

    if (ito < ifrom)
        endIndex = ito + canvasSplit->screenBufferWidth;
    else
        endIndex = ito;

    if (ito_local < ifrom_local)
        ito_local = getWidth() + ito_local;

    if (fullredraw)
    {
        ifrom_local = 0; //canvas->leftmargin;
        ito_local = getWidth();
        fullredraw = false;
    }

    //if (chan == 0)
    //{
    //	std::cout << "Channel 0 drawing from " << ifrom_local << " to " << ito_local << std::endl;
    //}

    bool drawWithOffsetCorrection = display->getMedianOffsetPlotting();

    LfpBitmapPlotterInfo plotterInfo; // hold and pass plotting info for each plotting method class

    for (int ii = ifrom; ii <= endIndex; ii++)
    {
        int i = (ifrom_local + ii - ifrom) % getWidth();
        int index = ii % canvasSplit->screenBufferWidth;

        //draw zero line
        int m = getY() + center;

        if (m > 0 && m < display->lfpChannelBitmap.getHeight())
        {
            bdLfpChannelBitmap.setPixelColour (i, m, Colour (50, 50, 50));
        }

        //draw range markers
        if (isSelected)
        {
            int start = getY() + center - channelHeight / 2;
            int jump = channelHeight / 4;

            for (m = start; m <= start + jump * 4; m += jump)
            {
                if (m > 0 && m < display->lfpChannelBitmap.getHeight())
                {
                    //if ( bdLfpChannelBitmap.getPixelColour(i,m).isTransparent()) // make sure we're not drawing over an existing plot from another channel
                    bdLfpChannelBitmap.setPixelColour (i, m, Colour (80, 80, 80));
                }
            }
        }

        // draw event markers
        const int rawEventState = canvasSplit->getEventState (index); // get event state

        for (int ev_ch = 0; ev_ch < 8; ev_ch++) // for all event channels
        {
            if (display->getEventDisplayState (ev_ch)) // check if plotting for this channel is enabled
            {
                if (rawEventState & (1 << ev_ch)) // events are  represented by a bit code, so we have to extract the individual bits with a mask
                {
                    //                        std::cout << "Drawing event." << std::endl;
                    const Colour currentcolour = display->channelColours[ev_ch * 2];

                    for (int k = jfrom_wholechannel; k <= jto_wholechannel; k++) // draw line
                    {
                        bdLfpChannelBitmap.setPixelColour (i,
                                                           k,
                                                           bdLfpChannelBitmap.getPixelColour (i, k).interpolatedWith (currentcolour, 0.3f));
                    }
                }
            }
        }

        // set max-min range for plotting
        double a = (canvasSplit->getYCoordMax (chan, index) / range * channelHeightFloat);
        double b = (canvasSplit->getYCoordMin (chan, index) / range * channelHeightFloat);

        double mean = (canvasSplit->getScreenBufferMean (chan) / range * channelHeightFloat);

        if (drawWithOffsetCorrection)
        {
            a -= mean;
            b -= mean;
        }

        double a_raw = canvasSplit->getYCoordMax (chan, index);
        double b_raw = canvasSplit->getYCoordMin (chan, index);
        double from_raw = 0;
        double to_raw = 0;

        if (a < b)
        {
            from = (a);
            to = (b);
            from_raw = (a_raw);
            to_raw = (b_raw);
        }
        else
        {
            from = (b);
            to = (a);
            from_raw = (b_raw);
            to_raw = (a_raw);
        }

        // start by clipping so that we're not populating pixels that we dont want to plot
        int lm = channelHeightFloat * canvasSplit->channelOverlapFactor;
        if (lm > 0)
            lm = -lm;

        if (from > -lm)
        {
            from = -lm;
            clipWarningHi = true;
        };
        if (to > -lm)
        {
            to = -lm;
            clipWarningHi = true;
        };
        if (from < lm)
        {
            from = lm;
            clipWarningLo = true;
        };
        if (to < lm)
        {
            to = lm;
            clipWarningLo = true;
        };

        // test if raw data is clipped for displaying saturation warning
        if (from_raw > options->selectedSaturationValueFloat)
        {
            saturateWarningHi = true;
        };
        if (to_raw > options->selectedSaturationValueFloat)
        {
            saturateWarningHi = true;
        };
        if (from_raw < -options->selectedSaturationValueFloat)
        {
            saturateWarningLo = true;
        };
        if (to_raw < -options->selectedSaturationValueFloat)
        {
            saturateWarningLo = true;
        };

        bool spikeFlag = display->getSpikeRasterPlotting()
                         && (from_raw - canvasSplit->getYCoordMean (chan, index) < display->getSpikeRasterThreshold()
                             || to_raw - canvasSplit->getYCoordMean (chan, index) < display->getSpikeRasterThreshold());

        from = from + getHeight() / 2; // so the plot is centered in the channeldisplay
        to = to + getHeight() / 2;

        int samplerange = to - from;

        plotterInfo.channelID = chan;
        plotterInfo.y = getY();
        plotterInfo.from = from;
        plotterInfo.to = to;
        plotterInfo.samp = i;
        plotterInfo.lineColour = lineColour;

        // Do the actual plotting for the selected plotting method
        if (! display->getSpikeRasterPlotting())
            display->getPlotterPtr()->plot (bdLfpChannelBitmap, plotterInfo);

        // now draw warnings, if needed
        if (canvasSplit->drawClipWarning) // draw simple warning if display cuts off data
        {
            if (clipWarningHi)
            {
                for (int j = 0; j <= 3; j++)
                {
                    int clipmarker = jto_wholechannel_clip;

                    if (clipmarker > 0 && clipmarker < display->lfpChannelBitmap.getHeight())
                    {
                        bdLfpChannelBitmap.setPixelColour (i, clipmarker - j, Colour (255, 255, 255));
                    }
                }
            }

            if (clipWarningLo)
            {
                for (int j = 0; j <= 3; j++)
                {
                    int clipmarker = jfrom_wholechannel_clip;

                    if (clipmarker > 0 && clipmarker < display->lfpChannelBitmap.getHeight())
                    {
                        bdLfpChannelBitmap.setPixelColour (i, clipmarker + j, Colour (255, 255, 255));
                    }
                }
            }

            clipWarningHi = false;
            clipWarningLo = false;
        }

        if (spikeFlag) // draw spikes
        {
            for (int k = jfrom_wholechannel; k <= jto_wholechannel; k++)
            { // draw line
                if (k > 0 && k < display->lfpChannelBitmap.getHeight())
                {
                    bdLfpChannelBitmap.setPixelColour (i, k, lineColour);
                }
            };
        }

        if (canvasSplit->drawSaturationWarning) // draw bigger warning if actual data gets cuts off
        {
            if (saturateWarningHi || saturateWarningLo)
            {
                for (int k = jfrom_wholechannel; k <= jto_wholechannel; k++)
                { // draw line
                    Colour thiscolour = Colour (255, 0, 0);
                    if (fmod ((i + k), 50) > 25)
                    {
                        thiscolour = Colour (255, 255, 255);
                    }
                    if (k > 0 && k < display->lfpChannelBitmap.getHeight())
                    {
                        bdLfpChannelBitmap.setPixelColour (i, k, thiscolour);
                    }
                };
            }

            saturateWarningHi = false; // we likely just need one of this because for this warning we dont care if its saturating on the positive or negative side
            saturateWarningLo = false;

        } // if i < getWidth()

    } //  for (int index = ifrom; index < ito; index++)
}

void LfpChannelDisplay::pxPaintHistory (int playhead, int rightEdge, int maxScreenBufferIndex)
{
    if (! isEnabled || isHidden || getWidth() == 0)
    {
        return; // return early if THIS display is not enabled
    }

    Image::BitmapData bdLfpChannelBitmap (display->lfpChannelBitmap, Image::BitmapData::readWrite);

    int center = getHeight() / 2;

    // max and min of channel in absolute px coords for event displays etc - actual data might be drawn outside of this range
    int jfrom_wholechannel = (int) (getY() + center - channelHeight / 2) + 1 + 0;
    int jto_wholechannel = (int) (getY() + center + channelHeight / 2) - 0;

    // max and min of channel, this is the range where actual data is drawn
    int jfrom_wholechannel_clip = (int) (getY() + center - (channelHeight) *canvasSplit->channelOverlapFactor) + 1;
    int jto_wholechannel_clip = (int) (getY() + center + (channelHeight) *canvasSplit->channelOverlapFactor) - 0;

    if (jfrom_wholechannel < 0)
    {
        jfrom_wholechannel = 0;
    };
    if (jto_wholechannel >= display->lfpChannelBitmap.getHeight())
    {
        jto_wholechannel = display->lfpChannelBitmap.getHeight() - 1;
    };

    if (playhead < rightEdge - 1)
    {
        for (int k = jfrom_wholechannel; k <= jto_wholechannel; k += 2) // draw yellow line
        {
            bdLfpChannelBitmap.setPixelColour (playhead + 1, k, Colours::yellow);
        }
    }

    bool clipWarningHi = false; // keep track if something clipped in the display, so we can draw warnings after the data pixels are done
    bool clipWarningLo = false;

    bool saturateWarningHi = false; // similar, but for saturating the amplifier, not just the display - make this warning very visible
    bool saturateWarningLo = false;

    int stepSize = 1;
    int from = 0;
    int to = 0;
    int endIndex;

    //if (chan == 0)
    //{
    //     std::cout << "Channel 0 drawing history" << std::endl;
    // }

    bool drawWithOffsetCorrection = display->getMedianOffsetPlotting();

    LfpBitmapPlotterInfo plotterInfo; // hold and pass plotting info for each plotting method class

    for (int ii = 0; ii < rightEdge; ii++)
    {
        int i = ii;
        int index = 0;

        if (playhead > rightEdge)
        {
            int hiddenPixels = playhead - rightEdge;

            index = maxScreenBufferIndex - hiddenPixels - rightEdge + ii;
        }
        else
        {
            if (ii < playhead)
            {
                index = maxScreenBufferIndex - playhead + ii;
            }
            else
            {
                //int numExtraPixels = rightEdge - playhead;
                int offset = ii - rightEdge;
                index = maxScreenBufferIndex - playhead + offset;
            }
        }

        if (index < 0)
            index = canvasSplit->screenBufferWidth + index;

        //if (index < 0) // past edge of screenbuffer
        //    continue;

        //if (ii == 0)
        //    std::cout << "First screenBufferIndex: " << index << std::endl;

        //draw zero line
        int m = getY() + center;

        if (m > 0 && m < display->lfpChannelBitmap.getHeight())
        {
            //if (bdLfpChannelBitmap.getPixelColour(i, m).isTransparent()) { // make sure we're not drawing over an existing plot from another channel
            bdLfpChannelBitmap.setPixelColour (i, m, Colour (50, 50, 50));
            // }
        }

        //draw range markers
        if (isSelected)
        {
            int start = getY() + center - channelHeight / 2;
            int jump = channelHeight / 4;

            for (m = start; m <= start + jump * 4; m += jump)
            {
                if (m > 0 && m < display->lfpChannelBitmap.getHeight())
                {
                    bdLfpChannelBitmap.setPixelColour (i, m, Colour (80, 80, 80));
                }
            }
        }

        // draw event markers
        const int rawEventState = canvasSplit->getEventState (index); // get event state

        for (int ev_ch = 0; ev_ch < 8; ev_ch++) // for all event channels
        {
            if (display->getEventDisplayState (ev_ch)) // check if plotting for this channel is enabled
            {
                if (rawEventState & (1 << ev_ch)) // events are  represented by a bit code, so we have to extract the individual bits with a mask
                {
                    //                        std::cout << "Drawing event." << std::endl;
                    const Colour currentcolour = display->channelColours[ev_ch * 2];

                    for (int k = jfrom_wholechannel; k <= jto_wholechannel; k++) // draw line
                    {
                        bdLfpChannelBitmap.setPixelColour (i,
                                                           k,
                                                           bdLfpChannelBitmap.getPixelColour (i, k).interpolatedWith (currentcolour, 0.3f));
                    }
                }
            }
        }

        // set max-min range for plotting
        double a = (canvasSplit->getYCoordMax (chan, index) / range * channelHeightFloat);
        double b = (canvasSplit->getYCoordMin (chan, index) / range * channelHeightFloat);

        double mean = (canvasSplit->getScreenBufferMean (chan) / range * channelHeightFloat);

        if (drawWithOffsetCorrection)
        {
            a -= mean;
            b -= mean;
        }

        double a_raw = canvasSplit->getYCoordMax (chan, index);
        double b_raw = canvasSplit->getYCoordMin (chan, index);
        double from_raw = 0;
        double to_raw = 0;

        if (a < b)
        {
            from = (a);
            to = (b);
            from_raw = (a_raw);
            to_raw = (b_raw);
        }
        else
        {
            from = (b);
            to = (a);
            from_raw = (b_raw);
            to_raw = (a_raw);
        }

        // start by clipping so that we're not populating pixels that we dont want to plot
        int lm = channelHeightFloat * canvasSplit->channelOverlapFactor;
        if (lm > 0)
            lm = -lm;

        if (from > -lm)
        {
            from = -lm;
            clipWarningHi = true;
        };
        if (to > -lm)
        {
            to = -lm;
            clipWarningHi = true;
        };
        if (from < lm)
        {
            from = lm;
            clipWarningLo = true;
        };
        if (to < lm)
        {
            to = lm;
            clipWarningLo = true;
        };

        // test if raw data is clipped for displaying saturation warning
        if (from_raw > options->selectedSaturationValueFloat)
        {
            saturateWarningHi = true;
        };
        if (to_raw > options->selectedSaturationValueFloat)
        {
            saturateWarningHi = true;
        };
        if (from_raw < -options->selectedSaturationValueFloat)
        {
            saturateWarningLo = true;
        };
        if (to_raw < -options->selectedSaturationValueFloat)
        {
            saturateWarningLo = true;
        };

        bool spikeFlag = display->getSpikeRasterPlotting()
                         && (from_raw - canvasSplit->getYCoordMean (chan, index) < display->getSpikeRasterThreshold()
                             || to_raw - canvasSplit->getYCoordMean (chan, index) < display->getSpikeRasterThreshold());

        from = from + getHeight() / 2; // so the plot is centered in the channeldisplay
        to = to + getHeight() / 2;

        int samplerange = to - from;

        plotterInfo.channelID = chan;
        plotterInfo.y = getY();
        plotterInfo.from = from;
        plotterInfo.to = to;
        plotterInfo.samp = i;
        plotterInfo.lineColour = lineColour;

        // Do the actual plotting for the selected plotting method
        if (! display->getSpikeRasterPlotting())
            display->getPlotterPtr()->plot (bdLfpChannelBitmap, plotterInfo);

        // now draw warnings, if needed
        if (canvasSplit->drawClipWarning) // draw simple warning if display cuts off data
        {
            if (clipWarningHi)
            {
                for (int j = 0; j <= 3; j++)
                {
                    int clipmarker = jto_wholechannel_clip;

                    if (clipmarker > 0 && clipmarker < display->lfpChannelBitmap.getHeight())
                    {
                        bdLfpChannelBitmap.setPixelColour (i, clipmarker - j, Colour (255, 255, 255));
                    }
                }
            }

            if (clipWarningLo)
            {
                for (int j = 0; j <= 3; j++)
                {
                    int clipmarker = jfrom_wholechannel_clip;

                    if (clipmarker > 0 && clipmarker < display->lfpChannelBitmap.getHeight())
                    {
                        bdLfpChannelBitmap.setPixelColour (i, clipmarker + j, Colour (255, 255, 255));
                    }
                }
            }

            clipWarningHi = false;
            clipWarningLo = false;
        }

        if (spikeFlag) // draw spikes
        {
            for (int k = jfrom_wholechannel; k <= jto_wholechannel; k++)
            { // draw line
                if (k > 0 && k < display->lfpChannelBitmap.getHeight())
                {
                    bdLfpChannelBitmap.setPixelColour (i, k, lineColour);
                }
            };
        }

        if (canvasSplit->drawSaturationWarning) // draw bigger warning if actual data gets cuts off
        {
            if (saturateWarningHi || saturateWarningLo)
            {
                for (int k = jfrom_wholechannel; k <= jto_wholechannel; k++)
                { // draw line
                    Colour thiscolour = Colour (255, 0, 0);
                    if (fmod ((i + k), 50) > 25)
                    {
                        thiscolour = Colour (255, 255, 255);
                    }
                    if (k > 0 && k < display->lfpChannelBitmap.getHeight())
                    {
                        bdLfpChannelBitmap.setPixelColour (i, k, thiscolour);
                    }
                };
            }

            saturateWarningHi = false; // we likely just need one of this because for this warning we dont care if its saturating on the positive or negative side
            saturateWarningLo = false;

        } // if i < getWidth()

    } //  for (int index = ifrom; index < ito; index++)
}

void LfpChannelDisplay::drawEventOverlay (int x, int yfrom, int yto, Image::BitmapData* image)
{
    // draw event markers
    const int rawEventState = canvasSplit->getEventState (x); // get event state

    for (int ev_ch = 0; ev_ch < 8; ev_ch++) // for all event channels
    {
        if (display->getEventDisplayState (ev_ch)) // check if plotting for this channel is enabled
        {
            if (rawEventState & (1 << ev_ch)) // events are  represented by a bit code, so we have to extract the individual bits with a mask
            {
                //                        std::cout << "Drawing event." << std::endl;
                const Colour currentcolour = display->channelColours[ev_ch * 2];

                for (int k = yfrom; k <= yto; k++) // draw line
                {
                    image->setPixelColour (x,
                                           k,
                                           image->getPixelColour (x, k).interpolatedWith (currentcolour, 0.3f));
                }
            }
        }
    }
}

void LfpChannelDisplay::paint (Graphics& g) {}

PopupMenu LfpChannelDisplay::getOptions()
{
    PopupMenu menu;
    menu.addItem (1, "Invert signal", true, inputInverted);
    menu.addItem (2, "Monitor", true);

    return menu;
}

void LfpChannelDisplay::changeParameter (int id)
{
    switch (id)
    {
        case 1:
            setInputInverted (! inputInverted);
        case 2:
            canvasSplit->monitorChannel (chan);
        default:
            break;
    }
}

void LfpChannelDisplay::setRange (float r)
{
    range = r;
}

float LfpChannelDisplay::getRange()
{
    return range;
}

void LfpChannelDisplay::select()
{
    isSelected = true;

    // if (isSelected)
    //    std::cout << "Selected channel " << chan << std::endl;
}

void LfpChannelDisplay::deselect()
{
    isSelected = false;
}

bool LfpChannelDisplay::getSelected()
{
    return isSelected;
}

void LfpChannelDisplay::setColour (Colour c)
{
    lineColour = c;
}

void LfpChannelDisplay::setChannelHeight (int c)
{
    channelHeight = c;

    channelHeightFloat = (float) channelHeight;

    if (! inputInverted)
        channelHeightFloat = -channelHeightFloat;

    channelOverlap = channelHeight * 2;
}

int LfpChannelDisplay::getChannelHeight()
{
    return channelHeight;
}

void LfpChannelDisplay::setChannelOverlap (int overlap)
{
    channelOverlap = overlap;
}

int LfpChannelDisplay::getChannelOverlap()
{
    return channelOverlap;
}

int LfpChannelDisplay::getChannelNumber()
{
    return chan;
}

String LfpChannelDisplay::getName()
{
    return name;
}

int LfpChannelDisplay::getDrawableChannelNumber()
{
    return drawableChan;
}

void LfpChannelDisplay::setDrawableChannelNumber (int channelId)
{
    drawableChan = channelId;
}

void LfpChannelDisplay::setCanBeInverted (bool _canBeInverted)
{
    canBeInverted = _canBeInverted;
}

void LfpChannelDisplay::setInputInverted (bool isInverted)
{
    if (canBeInverted)
    {
        inputInverted = isInverted;
        setChannelHeight (channelHeight);
    }
}

bool LfpChannelDisplay::getInputInverted()
{
    return inputInverted;
}

void LfpChannelDisplay::setDrawMethod (bool isDrawMethod)
{
    drawMethod = isDrawMethod;
}

void LfpChannelDisplay::setName (String name_)
{
    name = name_;
}

void LfpChannelDisplay::setGroup (int group_)
{
    group = group_;
}

void LfpChannelDisplay::setDepth (float depth_)
{
    //std::cout << "Channel " << name << ", depth = " << depth_ << std::endl;
    depth = depth_;
}

void LfpChannelDisplay::setRecorded (bool recorded_)
{
    isRecorded = recorded_;
}

ContinuousChannel::Type LfpChannelDisplay::getType()
{
    return type;
}
