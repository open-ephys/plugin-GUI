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

#ifndef __DEFAULTCOLOURSCHEME_H__
#define __DEFAULTCOLOURSCHEME_H__

#include <VisualizerWindowHeaders.h>

#include <array>
#include <vector>

#include "ChannelColourScheme.h"

namespace LfpViewer
{
#pragma mark - DefaultColourScheme -
class DefaultColourScheme : public ChannelColourScheme
{
public:
    /** Constructor */
    DefaultColourScheme();

    /** Destructor */
    virtual ~DefaultColourScheme() {}

    /** Returns colour at a given channel index */
    virtual const Colour getColourForIndex (int index) const override;

    /** Returns the background colour*/
    virtual const Colour getBackgroundColour() const override;

private:
    static Array<Colour> colourList;
};

}; // namespace LfpViewer
#endif
