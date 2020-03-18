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
#ifndef __SHOWHIDEOPTIONSBUTTON_H__
#define __SHOWHIDEOPTIONSBUTTON_H__

#include <VisualizerWindowHeaders.h>

#include <vector>
#include <array>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"
namespace LfpViewer {
#pragma  mark - ShowHideOptionsButton -
//==============================================================================
/**
 
 Toggles view options drawer for LfpDisplayCanvas.
 
 */
class ShowHideOptionsButton : public Button
{
public:
    ShowHideOptionsButton(LfpDisplayOptions*);
    virtual ~ShowHideOptionsButton();
    void paintButton(Graphics& g, bool, bool);
    LfpDisplayOptions* options;
};
    
}; // namespace
#endif
