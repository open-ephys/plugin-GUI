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
#ifndef __LFPGRADIENTCOLOURSCHEME_H__
#define __LFPGRADIENTCOLOURSCHEME_H__

#include <VisualizerWindowHeaders.h>

#include <vector>
#include <array>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"
#include "LfpMonochromaticColourScheme.h"
namespace LfpViewer {
#pragma  mark - LfpGradientColourScheme
    
class LfpGradientColourScheme : public LfpMonochromaticColourScheme
{
public:
    
    LfpGradientColourScheme(LfpDisplay*, LfpDisplayCanvas*);
    
    void paint(Graphics &) override;
    void resized() override;
    
    void sliderValueChanged(Slider *sl) override;
    void mouseUp(const MouseEvent &e) override;
    
    void setLerpToHue(Colour c);
    Colour getLerpToHue();
    
private:
    Colour baseHueB;
    Colour swatchHueB;
    Rectangle<int> colourSwatchRectB;
    
    ScopedPointer<Label> baseHueLabelB;
    ScopedPointer<Slider> baseHueSliderB;
    
    void calculateColourSeriesFromBaseHue() override;
};
    
};

#endif  // __LFPDISPLAYCANVAS_H_Alpha__

