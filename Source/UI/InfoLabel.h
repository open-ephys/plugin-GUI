/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#ifndef __INFOLABEL_H_14DA9A62__
#define __INFOLABEL_H_14DA9A62__


#include "../../JuceLibraryCode/JuceHeader.h"

/**

  Displays general instructions about how to use the application.

  Inhabits a tab in the DataViewport.

  @see UIComponent, DataViewport

*/

class InfoLabel : public Component

{
public:
    InfoLabel();
    ~InfoLabel();

    /** Draws the InfoLabel.*/
    void paint(Graphics& g);

private:

    /** The text displayed to the user.*/
    String infoString;

    /** Font used to draw the label.*/
    Font labelFont;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InfoLabel);


};




#endif  // __INFOLABEL_H_14DA9A62__
