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
 Customized version of the ArrowButton with an added OpenEphys LookAndFeel.

 @see ArrowButton
 */
class CustomArrowButton  : public Button
{
public:
    /** Creates an CustomArrowButton.

     @param name        the name to give the button
     @param direction   the direction the arrow should points to (0.0 = right, 0.25 = down, 0.5 = left and 0.75 = up)
     */
    CustomArrowButton(const String& name, float direction);

    /** Destructor. */
    ~CustomArrowButton();

    /** @internal */
    void paintButton(Graphics&, bool isMouseOverButton, bool isButtonDown);

private:
    Path path;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomArrowButton)
};


#endif   // __CUSTOMARROWBUTTON__
