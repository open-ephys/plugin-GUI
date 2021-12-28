/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#ifndef __DELAYMONITOR_H_BDCEE716__
#define __DELAYMONITOR_H_BDCEE716__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

class GenericEditor;

/** 
* 
    Displays the amount of time elapsed between the 
    start of each data processing cycle and the end of 
    one plugin's process() callback.

    Makes it possible to see which plugins are taking
    the most time to complete their work.

*/
class PLUGIN_API DelayMonitor : 
    public Component,
    public Timer
{
public:

    /** Constructor */
    DelayMonitor();

    /** Destructor */
	~DelayMonitor() { }

    /** Sets the most recent delay (in ms)*/
    void setDelay(float delayMs);
    
    /** Enable or disable this component*/
    void setEnabled(bool isEnabled);

    /** Render the delay*/
    void paint(Graphics& g);

    /** Calls 'repaint' to display the latest delay*/
    void timerCallback();

    /** Starts the 500 ms painting timer */
    void startAcquisition();

    /** Stops the timer*/
    void stopAcquisition();

private:
    
    bool isEnabled;
    Colour colour;
    float delay;
    Font font;
};



#endif  // __DELAYMONITOR_H_BDCEE716__
