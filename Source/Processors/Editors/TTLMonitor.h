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

#ifndef __TTLMONITOR_H_BDCEE716__
#define __TTLMONITOR_H_BDCEE716__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

class EventChannel;
class GenericEditor;

/**

  Used to display the status of a TTL bit.

  @see GenericEditor, EventChannel.

*/
class PLUGIN_API TTLBitDisplay : public Component
{
public:
    TTLBitDisplay(Colour colour, String tooltipString);
    ~TTLBitDisplay();

    String getTooltip();

    void setState(bool state);

    void paint(Graphics& g);

    bool changedSinceLastRedraw;

private:

    Colour colour;
    String tooltipString;
    bool state;
};

/**
  Used to display the status of TTL events within
  a GenericEditor.

  @see GenericEditor, EventChannel.

*/
class PLUGIN_API TTLMonitor : public Component,
    public Timer
{
public:
    TTLMonitor();
	~TTLMonitor();

	int updateSettings(Array<EventChannel*> eventChannels);

    void setState(int bit, bool state);
    
    void startAcquisition();
    void stopAcquisition();

    void timerCallback();

private:

    Array<Colour> colours; 

    OwnedArray<TTLBitDisplay> displays;
};



#endif  // __TTLMONITOR_H_BDCEE716__
