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


class PLUGIN_API DelayMonitor : public Component,
    public Timer
{
public:
    DelayMonitor();
	~DelayMonitor();

    void setDelay(float delayMs);

    void paint(Graphics& g);

    void timerCallback();

    void startAcquisition();

    void stopAcquisition();

private:
    

    float delay;
};



#endif  // __DELAYMONITOR_H_BDCEE716__
