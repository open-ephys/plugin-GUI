/*
    -----------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

#ifndef __MESSAGECENTER_H_2695FC38__
#define __MESSAGECENTER_H_2695FC38__


#ifdef WIN32
#include <Windows.h>
#endif
#include "../../JuceLibraryCode/JuceHeader.h"

/**
  
  Allows the application to display messages to the user.

  The MessageCenter is located along the bottom left of the application window.

  @see UIComponent

*/

class MessageCenter : public Component,
					  public ActionListener

{
public:
    MessageCenter();
    ~MessageCenter();

    /** Draws the message center.*/
    void paint (Graphics& g);

private:

   /** A JUCE label used to display message text. */
   Label* messageDisplayArea;

   /** Called when the boundaries of the MessageCenter are changed. */
   void resized();

   /** Called when a new message is received. */
   void actionListenerCallback(const String& message);

   /** The background color (changes to yellow when a new message arrives). */
   Colour messageBackground;

};



#endif  // __MESSAGECENTER_H_2695FC38__
