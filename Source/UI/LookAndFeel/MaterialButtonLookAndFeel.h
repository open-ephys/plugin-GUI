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

#ifndef MATERIALBUTTONLOOKANDFEEL_H_INCLUDED
#define MATERIALBUTTONLOOKANDFEEL_H_INCLUDED

#include <JuceHeader.h>
#include "../../Processors/PluginManager/OpenEphysPlugin.h"


/**

   Used to modify the appearance of the buttons following Google Material Design Guidelines.

   Currently contains methods for drawing flat buttons only.

*/
class PLUGIN_API MaterialButtonLookAndFeel : public LookAndFeel_V2
{
public:
    MaterialButtonLookAndFeel();

    void drawButtonBackground (Graphics& g,
                               Button& button,
                               const Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override;

    void drawButtonText (Graphics& g,
                         TextButton& button,
                         bool isMouseOverButton, bool isButtonDown) override;

private:

    // =========================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MaterialButtonLookAndFeel)
};


#endif  // MATERIALBUTTONLOOKANDFEEL_H_INCLUDED
