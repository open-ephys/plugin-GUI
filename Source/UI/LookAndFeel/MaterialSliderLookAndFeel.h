/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#ifndef MATERIALSLIDERLOOKANDFEEL_H_INCLUDED
#define MATERIALSLIDERLOOKANDFEEL_H_INCLUDED

#include "../../JuceLibraryCode/JuceHeader.h"


/**

   Used to modify the appearance of the sliders following Google Material Design Guidelines.

   Currently contains methods for drawing linear sliders only.

*/

class MaterialSliderLookAndFeel : public LookAndFeel_V2
{
public:
    MaterialSliderLookAndFeel();
    ~MaterialSliderLookAndFeel();

    void drawLinearSlider (Graphics& g,
                           int x, int y,
                           int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const Slider::SliderStyle style,
                           Slider& slider) override;


    void drawLinearSliderThumb (Graphics& g,
                                int x, int y,
                                int width, int height,
                                float sliderPos, float minSliderPos, float maxSliderPos,
                                const Slider::SliderStyle style,
                                Slider& slider) override;


    void drawLinearSliderBackground  (Graphics& g,
                                      int x, int y,
                                      int width, int height,
                                      float sliderPos, float minSliderPos, float maxSliderPos,
                                      const Slider::SliderStyle style,
                                      Slider& slider) override;


private:
    void drawRoundThumb (Graphics& g,
                         const float x, const float y,
                         const float diameter,
                         const Colour& colour,
                         float outlineThickness);
};


#endif  // MATERIALSLIDERLOOKANDFEEL_H_INCLUDED
