/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "CustomArrowButton.h"


CustomArrowButton::CustomArrowButton (const String& name, float direction)
   : Button (name)
{
    path.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.applyTransform (AffineTransform::rotation (float_Pi * 2.0f * direction, 0.5f, 0.5f));
}

CustomArrowButton::~CustomArrowButton() {}

void CustomArrowButton::paintButton (Graphics& g, bool /*isMouseOverButton*/, bool isButtonDown)
{
    
    const float borderWidth = isEnabled() ? (isButtonDown ? 1.2f : 0.5f) : 0.3f;
    
    LookAndFeel::drawGlassLozenge(g,
                     borderWidth, borderWidth,
                     getWidth() - borderWidth * 2.0f, getHeight() - borderWidth * 2.0f,
                     Colours::orange, borderWidth, -1.0f,
                     true, true, true, true);
    
    if (isEnabled()) {
        Path p (path);
        p.scaleToFit (getWidth() * 0.25f, getHeight() * 0.25f,
                      getWidth() * 0.5f, getHeight() * 0.5f, false);
        
        g.setColour (Colours::darkgrey);
        g.fillPath (p);
    }
}