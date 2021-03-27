/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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
#ifndef __CHANNELCOLOURSCHEME_H__
#define __CHANNELCOLOURSCHEME_H__

#include <VisualizerWindowHeaders.h>

#include <vector>
#include <array>

#include "../LfpDisplayClasses.h"
#include "../LfpDisplayNode.h"
namespace LfpViewer {
#pragma  mark - LfpChannelColourScheme -
/**
 Interface for a color scheme object
 */
class ChannelColourScheme : public Component
{
public:
    ChannelColourScheme(int numColourChannels_, LfpDisplay* display, LfpDisplaySplitter* split)
    : lfpDisplay(display)
    , canvasSplit(split)
    , numColourChannels(numColourChannels_)
    { }
    
    virtual ~ChannelColourScheme() {}
    
    void paint(Graphics &g) override {}
    void resized() override {}
    
    virtual const Colour getColourForIndex(int index) const = 0;
    
    /** Returns true if a color scheme has configurable UI elements that
        must be drawn to the options drawer. Subclasses should override this
        if they have drawable elements in the options drawer. */
    virtual bool hasConfigurableElements() { return false; }
    
    void setColourGrouping(int grouping);
    int getColourGrouping();

    virtual const Colour getBackgroundColour() const = 0;
    
protected:
    LfpDisplay * lfpDisplay;
    LfpDisplaySplitter * canvasSplit;
    
    int numColourChannels;
    static int colourGrouping;

};
    
}; // namespace
#endif
