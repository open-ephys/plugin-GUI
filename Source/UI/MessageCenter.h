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

//---------------------------------------------------------------

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

    void paint (Graphics& g);

private:
   Label* messageDisplayArea;

   void resized();

   void actionListenerCallback(const String& message);

   Colour messageBackground;

};



#endif  // __MESSAGECENTER_H_2695FC38__
