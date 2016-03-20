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

#include "MaterialButtonLookAndFeel.h"


MaterialButtonLookAndFeel::MaterialButtonLookAndFeel()
{
}


void MaterialButtonLookAndFeel::drawButtonBackground (Graphics& g,
                                                      Button& button,
                                                      const Colour& backgroundColour,
                                                      bool isMouseOverButton, bool isButtonDown)
{
    // Only flat button style available for now
    const bool isFlatButton = true;
    if (isFlatButton)
    {
        const bool isButtonToggled = button.getToggleState();

        Colour bgColourToUse;
        if (isButtonToggled)
            bgColourToUse = backgroundColour;
        else if (isButtonDown || isMouseOverButton)
            bgColourToUse = backgroundColour.darker();

        const float cornerSize = 0.f;
        g.setColour (bgColourToUse);
        g.fillRoundedRectangle (button.getLocalBounds().toFloat(), cornerSize);
    }
}


void MaterialButtonLookAndFeel::drawButtonText (Graphics& g,
                                                TextButton& button,
                                                bool isMouseOverButton, bool isButtonDown)
{
    auto buttonText = button.getButtonText().toUpperCase();

    auto textColour = button.getToggleState()
                        ? button.findColour (TextButton::textColourOnId)
                        : button.findColour (TextButton::textColourOffId);

    const int padding = 4;
    g.setFont (Font ("Default Bold", 13.f, Font::plain));
    g.setColour (textColour);
    g.drawFittedText (buttonText,
                      padding, 0,
                      button.getWidth() - padding * 2, button.getHeight(),
                      Justification::centred,
                      1);
}


