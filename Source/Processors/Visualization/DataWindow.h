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

#ifndef __DATAWINDOW_H_FDDAB8D0__
#define __DATAWINDOW_H_FDDAB8D0__

#include "../../../JuceLibraryCode/JuceHeader.h"

/**

  Allows visualizers to be placed in their own window.

  @see DataViewport

*/

class DataWindow : public DocumentWindow
{
public:
    DataWindow(Button* button, String name);
    ~DataWindow();
    
    void closeButtonPressed();
    
    class Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener()  {}

        /** Called when the window is closed. */
        virtual void windowClosed () = 0;
    };

    /**
     * @brief      Registers a listener to receive event when this is closed. If
     *             the listener is already registered, this will not register it
     *             again.
     */
    void addListener (Listener* newListener);

    /**
     * @brief      Removes a previously-registered DataWindow listener
     */
    void removeListener (Listener* listener);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DataWindow);

    Button* controlButton;
    ListenerList<Listener> closeWindowListeners;
};


#endif  // __DATAWINDOW_H_FDDAB8D0__
