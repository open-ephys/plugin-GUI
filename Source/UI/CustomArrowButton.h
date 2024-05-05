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

#ifndef __CUSTOMARROWBUTTON__
#define __CUSTOMARROWBUTTON__

#include "../../JuceLibraryCode/JuceHeader.h"

/**
 Arrow button used to open/close components

 @see ArrowButton
 */
class CustomArrowButton : public ToggleButton
{
public:
    
    /** Constructor*/
    CustomArrowButton(float rotation = 0.0f, float width = 22.0f);
    
    /** Destructor*/
    ~CustomArrowButton() { }

    /** Draws the button. */
    void paint(Graphics& g) override;
    
    /** Sets the background colour (colour of center of arrow)  */
    void setCustomBackground(bool useCustomBackground, Colour colour);

private:

    Path openPath, closedPath;
    Colour backgroundColour;
    float width;
    bool customBackground = false;

};

#endif   // __CUSTOMARROWBUTTON__
