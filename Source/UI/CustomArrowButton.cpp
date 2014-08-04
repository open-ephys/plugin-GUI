/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2014 Florian Franzen

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

#include "CustomArrowButton.h"


CustomArrowButton::CustomArrowButton(const String& name, float direction)
    : Button(name)
{
    path.addTriangle(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.applyTransform(AffineTransform::rotation(float_Pi * 2.0f * direction, 0.5f, 0.5f));
}

CustomArrowButton::~CustomArrowButton() {}

void CustomArrowButton::paintButton(Graphics& g, bool /*isMouseOverButton*/, bool isButtonDown)
{

    const float borderWidth = isEnabled() ? (isButtonDown ? 1.2f : 0.5f) : 0.3f;

    juce::LookAndFeel_V1::drawGlassLozenge(g,
                                           borderWidth, borderWidth,
                                           getWidth() - borderWidth * 2.0f, getHeight() - borderWidth * 2.0f,
                                           Colours::orange, borderWidth, -1.0f,
                                           true, true, true, true);

    if (isEnabled())
    {
        Path p(path);
        p.scaleToFit(getWidth() * 0.25f, getHeight() * 0.25f,
                     getWidth() * 0.5f, getHeight() * 0.5f, false);

        g.setColour(Colours::darkgrey);
        g.fillPath(p);
    }
}