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

#ifndef __VISUALIZER_H_C5943EC1__
#define __VISUALIZER_H_C5943EC1__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

/**

  Abstract base class for displaying data in a tab or window.

  Can also be used to create a larger settings interface 
  than is possible inside a plugin's editor.

  @see LfpDisplayCanvas, SpikeDisplayCanvas

*/

class PLUGIN_API Visualizer : public Component,
                              public Timer

{
public:

    /** Constructor */
	Visualizer() { }

    /** Destructor */
	virtual ~Visualizer() { }

    /** Called when the component's tab becomes visible again.*/
    virtual void refreshState() = 0;

    /** Called when parameters of the underlying data processor are changed.*/
    virtual void update() = 0;

    /** Called instead of "repaint" to avoid redrawing underlying components if not necessary.*/
    virtual void refresh() = 0;

    /** Called when data acquisition is active.*/
    virtual void beginAnimation() = 0;

    /** Called when data acquisition ends.*/
    virtual void endAnimation() = 0;

    /** Starts the animation. */
	void startCallbacks();

    /** Stops the animation. */
	void stopCallbacks();

    /** Calls refresh(). */
	void timerCallback();

    /** Refresh rate in Hz. */
    float refreshRate = 50;

    /** Saves parameters as XML */
    virtual void saveCustomParametersToXml(XmlElement* xml) { }

    /** Loads parameters from XML */
    virtual void loadCustomParametersFromXml(XmlElement* xml) { }

};


#endif  // __VISUALIZER_H_C5943EC1__
