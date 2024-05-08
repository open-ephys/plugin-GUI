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
#include "LookAndFeel/CustomLookAndFeel.h"

CustomArrowButton::CustomArrowButton(float rotation, float width_) : ToggleButton(),
    width(width_)
{
    //openPath.addTriangle(-1f, -1f, 1f, -1.0f, 0.0f, 1.0f );
    //openPath.applyTransform(AffineTransform::scale(2.2f, 2.2f));
    //openPath.applyTransform(AffineTransform::translation(-5.f, -5.f));
    //openPath.applyTransform(AffineTransform::rotation(rotation));
    //openPath.applyTransform(AffineTransform::translation(5.f, 5.f));
    //closedPath.addTriangle(1.5f, 5.0f, 7.0f, 2.2f, 7.0f, 7.8f);
    //closedPath.applyTransform(AffineTransform::scale(2.2f, 2.2f));
    //closedPath.applyTransform(AffineTransform::rotation(rotation));
    
    juce::Path trianglePath;
            
    // Define vertices of the triangle centered around zero
    juce::Point<float> vertices[3];
    vertices[0] = juce::Point<float>(0, 0.6);
    vertices[1] = juce::Point<float>(0.5, -0.36);
    vertices[2] = juce::Point<float>(-0.5, -0.36);
    
    //vertices[0] = juce::Point<float>(1.5, 5.0);
    //vertices[1] = juce::Point<float>(7.0, 2.2);
    //vertices[2] = juce::Point<float>(7.0, 7.8);
    
    trianglePath.startNewSubPath(vertices[0]);
    trianglePath.lineTo(vertices[1]);
    trianglePath.lineTo(vertices[2]);
    trianglePath.closeSubPath();
    
    openPath = trianglePath.createPathWithRoundedCorners(0.2f);
    openPath.applyTransform(AffineTransform::rotation(rotation));
    
    closedPath = Path(openPath);
    closedPath.applyTransform(AffineTransform::rotation(MathConstants<float>::pi/2));
    
    openPath.applyTransform(AffineTransform::scale(width * 0.58f));
    openPath.applyTransform(AffineTransform::translation(width/2, width/2));
    
    closedPath.applyTransform(AffineTransform::scale(width * 0.58f));
    closedPath.applyTransform(AffineTransform::translation(width/2, width/2));
    
    backgroundColour = findColour(ThemeColors::componentBackground);
}

void CustomArrowButton::setCustomBackground(bool useCustom, Colour colour)
{
    customBackground = useCustom;
    backgroundColour = colour;
    repaint();
}

void CustomArrowButton::paint(Graphics& g)
{
    Colour foregroundColour;
    
    if (customBackground)
        foregroundColour = backgroundColour.darker(0.9f).withAlpha(0.5f);
    else
        foregroundColour = findColour(ThemeColors::defaultText).withAlpha(0.3f);
    
    if (isOver())
        g.setColour(foregroundColour.brighter(0.4f));
    else
        g.setColour(foregroundColour);
    
    g.fillEllipse(0, 0, width, width);

    if (customBackground)
        g.setColour(backgroundColour);
    else
        g.setColour(findColour(ThemeColors::componentBackground));
    
    if (getToggleState())
        g.fillPath(openPath);
    else
        g.fillPath(closedPath);
}

