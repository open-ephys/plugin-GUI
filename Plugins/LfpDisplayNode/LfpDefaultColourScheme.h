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
#ifndef __LFPDEFAULTCOLOURSCHEME_H__
#define __LFPDEFAULTCOLOURSCHEME_H__

#include <VisualizerWindowHeaders.h>

#include <vector>
#include <array>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"
#include "LfpChannelColourScheme.h"
namespace LfpViewer {
#pragma  mark - LfpDefaultColourScheme -
class LfpDefaultColourScheme : public LfpChannelColourScheme
{
public:
    LfpDefaultColourScheme(LfpDisplay*, LfpDisplayCanvas*);
    virtual ~LfpDefaultColourScheme() {}
    
    void paint(Graphics &g) override;
    void resized() override;
    
    virtual const Colour getColourForIndex(int index) const override;
    
private:
    static Array<Colour> colourList;
};
    
}; // namespace
#endif
