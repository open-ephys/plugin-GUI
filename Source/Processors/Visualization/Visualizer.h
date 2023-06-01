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
#include "../GenericProcessor/GenericProcessor.h"
#include "../Editors/GenericEditor.h"
#include "../Parameter/ParameterEditor.h"

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
	Visualizer(GenericProcessor* parentProcessor = nullptr);

    /** Destructor */
	virtual ~Visualizer() { }

    // ------------------------------------------------------------
    //                  CRITICAL CLASS MEMBER
    // ------------------------------------------------------------

    /** Refresh rate in Hz. Update this value to change the refresh rate
        of your visualizer. */
    float refreshRate = 50;

    // ------------------------------------------------------------
    //                  PURE VIRTUAL METHODS 
    //       (must be implemented by all Visualizers)
    // ------------------------------------------------------------

    /** Renders the Visualizer on each animation callback cycle
        Called instead of Juce's "repaint()" to avoid redrawing underlying components
        if not necessary.*/
    virtual void refresh() = 0;

    /** Called when the Visualizer's tab becomes visible after being hidden .*/
    virtual void refreshState() = 0;

    // ------------------------------------------------------------
    //                   VIRTUAL METHODS 
    //       (can optionally be overriden by sub-classes)
    // ------------------------------------------------------------

    /** Called by the update() method to allow the visualizer to update its custom settings.*/
    virtual void updateSettings() { }

    /** Called when data acquisition begins. 
        If the Visualizer includes live rendering, it should call
        startCallbacks() within this method. */
    virtual void beginAnimation() { startCallbacks(); }

    /** Called when data acquisition ends.
       If the Visualizer includes live rendering, it should call
       stopCallbacks() within this method. */
    virtual void endAnimation() { stopCallbacks(); }

    /** Saves visualizer parameters to XMLoejct */
    virtual void saveCustomParametersToXml(XmlElement* xml) { }

    /** Loads visualizer parameters from XML object */
    virtual void loadCustomParametersFromXml(XmlElement* xml) { }

    // ------------------------------------------------------------
    //                     OTHER METHODS
    // ------------------------------------------------------------
    
    /** Called when the Visualizer is first created, and optionally when
        the parameters of the underlying processor are changed. */
    void update();

    /** Starts animation callbacks at refreshRate Hz. */
	void startCallbacks();

    /** Stops animation callbacks. */
	void stopCallbacks();

    /** Calls refresh(). */
	void timerCallback();

    /** Returns the parameter editor for a given parameter name*/
    ParameterEditor* getParameterEditor(const String& parameterName);

    /** An array of pointers to ParameterEditors created based on the Parameters of an editor's underlying processor. */
    OwnedArray<ParameterEditor> parameterEditors;
    
    /** Called when the editor needs to update the view of its parameters.*/
    void updateView();

protected:

    /** Adds a text box editor for a parameter of a given name. */
    void addTextBoxParameterEditor (const String& name, int xPos, int yPos, Component *parentComponent = nullptr);

    /** Adds a check box editor for a parameter of a given name. */
    void addCheckBoxParameterEditor(const String& name, int xPos, int yPos, Component *parentComponent = nullptr);

    /** Adds a slider editor for a parameter of a given name. */
    void addSliderParameterEditor(const String& name, int xPos, int yPos, Component *parentComponent = nullptr);

    /** Adds a combo box editor for a parameter of a given name. */
    void addComboBoxParameterEditor(const String& name, int xPos, int yPos, Component *parentComponent = nullptr);

    /** Adds a selected channels editor for a parameter of a given name. */
    void addSelectedChannelsParameterEditor(const String& name, int xPos, int yPos, Component *parentComponent = nullptr);
    
    /** Adds a selected channels editor for a parameter of a given name. */
    void addMaskChannelsParameterEditor(const String& name, int xPos, int yPos, Component *parentComponent = nullptr);

    /** Adds a custom editor for a parameter of a given name. */
    void addCustomParameterEditor(ParameterEditor* editor, int xPos, int yPos, Component *parentComponent = nullptr);

private:

    GenericProcessor* processor;
};


#endif  // __VISUALIZER_H_C5943EC1__
