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

#include "MaterialSliderLookAndFeel.h"


static const Colour COLOUR_BORDER (Colour (0x0)); // may be usefull for future

// If we want to drop shadow for slider later we can just change this parameter
static const bool DROP_SHADOW = false;


MaterialSliderLookAndFeel::MaterialSliderLookAndFeel()
{
}


MaterialSliderLookAndFeel::~MaterialSliderLookAndFeel()
{
}


void MaterialSliderLookAndFeel::drawLinearSlider (Graphics& g,
                                                  int x, int y,
                                                  int width, int height,
                                                  float sliderPos, float minSliderPos, float maxSliderPos,
                                                  const Slider::SliderStyle sliderStyle,
                                                  Slider& slider)
{
    if (sliderStyle == Slider::LinearVertical || sliderStyle == Slider::LinearHorizontal)
    {
        drawLinearSliderBackground (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, sliderStyle, slider);
        drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, sliderStyle, slider);
    }
}


void MaterialSliderLookAndFeel::drawLinearSliderBackground (Graphics& g,
                                                            int x, int y,
                                                            int width, int height,
                                                            float sliderPos, float minSliderPos, float maxSliderPos,
                                                            const Slider::SliderStyle style,
                                                            Slider& slider)
{
    const float sliderRadius = getSliderThumbRadius (slider) - 7.0f;
    Path on;
    Path off;

    if (slider.isHorizontal())
    {
        const float iy = y + height * 0.5f - sliderRadius * 0.5f;
        Rectangle<float> r (x - sliderRadius * 0.5f, iy, width + sliderRadius, sliderRadius);
        const float onW = r.getWidth() * ((float) slider.valueToProportionOfLength (slider.getValue()));

        on.addRectangle (r.removeFromLeft (onW));
        off.addRectangle (r);
    }
    else
    {
        const float ix = x + width * 0.5f - sliderRadius * 0.5f;
        Rectangle<float> r (ix, y - sliderRadius * 0.5f, sliderRadius, height + sliderRadius);
        const float onH = r.getHeight() * ((float) slider.valueToProportionOfLength (slider.getValue()));

        on.addRectangle (r.removeFromBottom (onH));
        off.addRectangle (r);
    }

    g.setColour (slider.findColour (Slider::backgroundColourId));
    g.fillPath (off);

    g.setColour (slider.findColour (Slider::trackColourId));
    g.fillPath (on);

    g.setColour (COLOUR_BORDER);
    g.strokePath (on,  PathStrokeType (1.f));
    g.strokePath (off, PathStrokeType (1.f));
}


void MaterialSliderLookAndFeel::drawLinearSliderThumb (Graphics& g,
                                                       int x, int y,
                                                       int width, int height,
                                                       float sliderPos, float minSliderPos, float maxSliderPos,
                                                       const Slider::SliderStyle style,
                                                       Slider& slider)
{
    const float sliderRadius = (float) (getSliderThumbRadius (slider) - 2);

    bool isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());
    Colour knobColour (slider.findColour (Slider::thumbColourId)
                       .withMultipliedSaturation ((slider.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                       .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f));

    if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
    {
        float kx;
        float ky;

        if (style == Slider::LinearVertical)
        {
            kx = x + width * 0.5f;
            ky = sliderPos;
        }
        else
        {
            kx = sliderPos;
            ky = y + height * 0.5f;
        }

        const float outlineThickness = slider.isEnabled() ? 0.8f : 0.3f;

        drawRoundThumb (g,
                        kx - sliderRadius,
                        ky - sliderRadius,
                        sliderRadius * 2.0f,
                        knobColour, outlineThickness);
    }
}


void MaterialSliderLookAndFeel::drawRoundThumb (Graphics& g,
                                                const float x, const float y,
                                                const float diameter,
                                                const Colour& colour,
                                                float outlineThickness)
{
    const float halfThickness = outlineThickness * 0.5f;

    Path p;
    p.addEllipse (x + halfThickness, y + halfThickness, diameter - outlineThickness, diameter - outlineThickness);

    if (DROP_SHADOW)
    {
        const DropShadow ds (Colours::black, 3, juce::Point<int> (1, 1));
        ds.drawForPath (g, p);
    }

    g.setColour (colour);
    g.fillPath (p);

    g.setColour (COLOUR_BORDER);
    g.strokePath (p, PathStrokeType (outlineThickness));
}

