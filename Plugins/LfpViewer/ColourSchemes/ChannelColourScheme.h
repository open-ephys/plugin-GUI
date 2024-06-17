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

#ifndef __CHANNELCOLOURSCHEME_H__
#define __CHANNELCOLOURSCHEME_H__

#include <VisualizerWindowHeaders.h>

#include <array>
#include <vector>

namespace LfpViewer
{
#pragma mark - LfpChannelColourScheme -

/**
 Interface for a colour scheme object
 */
class ChannelColourScheme
{
public:
    /** Constructor */
    ChannelColourScheme (String name_, int numColourChannels_)
        : numColourChannels (numColourChannels_), name (name_)
    {
    }

    /** Destructor */
    virtual ~ChannelColourScheme() {}

    /** Returns the colour for a given channel index */
    virtual const Colour getColourForIndex (int index) const = 0;

    /** Returns the background colour*/
    virtual const Colour getBackgroundColour() const = 0;

    /** Returns the name of the colour scheme */
    String getName() { return name; }

    /** Sets the number of consecutive grouped colours */
    void setColourGrouping (int grouping);

    /** Returns the numbers of consecutive grouped colours*/
    int getColourGrouping();

protected:
    int numColourChannels;
    static int colourGrouping;
    String name;
};

}; // namespace LfpViewer
#endif
