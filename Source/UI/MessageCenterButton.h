/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2020 Open Ephys

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

#ifndef MessageCenterButton_h
#define MessageCenterButton_h

#include "../../JuceLibraryCode/JuceHeader.h"

/**

  Allows the user to expand and collapse the message center editor.

  @see MessageCenter.

*/

class MessageCenterButton : public DrawableButton
{
public:
    MessageCenterButton() : DrawableButton("Message Center Button", DrawableButton::ImageFitted), isExpanded(false)
    {
    
        Path upArrow;
        Path downArrow;
        
        upArrow.startNewSubPath (4.0f, 0.0f);
        upArrow.lineTo(12.0f, 5.0f);
        upArrow.lineTo(20.0f, 0.0f);
        
        downArrow.startNewSubPath (4.0f, 5.0f);
        downArrow.lineTo(12.0f, 0.0f);
        downArrow.lineTo(20.0f, 5.0f);

        expanded.setPath(upArrow);
        expanded.setFill(Colours::white.withAlpha(0.0f));
        expanded.setStrokeFill(Colour(220,220,220));
        expanded.setStrokeThickness(1.0f);
        
        expandedOver.setPath(upArrow);
        expandedOver.setFill(Colours::white.withAlpha(0.0f));
        expandedOver.setStrokeFill(Colour(230,230,230));
        expandedOver.setStrokeThickness(1.0f);
    
        collapsed.setPath(downArrow);
        collapsed.setFill(Colours::white.withAlpha(0.0f));
        collapsed.setStrokeFill(Colour(130,130,130));
        collapsed.setStrokeThickness(1.0f);
        
        collapsedOver.setPath(downArrow);
        collapsedOver.setFill(Colours::white.withAlpha(0.0f));
        collapsedOver.setStrokeFill(Colour(150,150,150));
        collapsedOver.setStrokeThickness(1.0f);
    
        setImages(&collapsed, &collapsedOver, &collapsedOver);

        setClickingTogglesState(false);
    }
    
    ~MessageCenterButton() { }
    
    void switchState() {
        
        isExpanded = !isExpanded;
        if (isExpanded)
        {
            setImages(&expanded, &expandedOver, &expandedOver);
        } else {
            setImages(&collapsed, &collapsedOver, &collapsedOver);
        }
        
    }

    DrawablePath expanded, expandedOver, collapsed, collapsedOver;
    
private:
    
    bool isExpanded;

};

#endif /* MessageCenterButton_h */
