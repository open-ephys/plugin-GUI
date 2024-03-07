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
#include "../Parameter/Parameter.h"
#include "../Parameter/ParameterEditor.h"
#include "../Parameter/ParameterOwner.h"
#include "../Parameter/ParameterEditorOwner.h"

/**

  Abstract base class for displaying data in a tab or window.

  Can also be used to create a larger settings interface 
  than is possible inside a plugin's editor.

  @see LfpDisplayCanvas, SpikeDisplayCanvas

*/

class PLUGIN_API Visualizer : public Component,
                              public Timer,
                              public ParameterOwner,
                              public ParameterEditorOwner

{
public:

    /** Constructor */
	Visualizer(GenericProcessor* parentProcessor = nullptr);

    /** Destructor */
    virtual ~Visualizer();

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

    /** Called when data acquisition begins. 
        If the Visualizer includes live rendering, it should call
        startCallbacks() within this method. */
    virtual void beginAnimation() { startCallbacks(); }

    /** Called when data acquisition ends.
       If the Visualizer includes live rendering, it should call
       stopCallbacks() within this method. */
    virtual void endAnimation() { stopCallbacks(); }

    /** Save parameter settings to XML (called by all visualizers).*/
    void saveToXml (XmlElement* xml);

    /** Load parameter settings from XML (called by all visualizers). */
    void loadFromXml(XmlElement* xml);

    /** Saves custom visualizer parameters to XMLobject */
    virtual void saveCustomParametersToXml(XmlElement* xml) { }

    /** Loads custom visualizer parameters from XML object */
    virtual void loadCustomParametersFromXml(XmlElement* xml) { }

    // ------------------------------------------------------------
    //                     OTHER METHODS
    // ------------------------------------------------------------
    
    /** Called when the Visualizer is first created, and optionally when
        the parameters of the underlying processor are changed. */
    void update();

    /** Called by the update() method to allow the visualizer to update its custom settings.*/
    virtual void updateSettings() { }

    /** Starts animation callbacks at refreshRate Hz. */
	void startCallbacks();

    /** Stops animation callbacks. */
	void stopCallbacks();

    /** Calls refresh(). */
	void timerCallback();

    void addParameterEditorOwner(ParameterEditorOwner* paramEditorOwner);

    /** Initiates parameter value update */
    void parameterChangeRequest(Parameter*);

    GenericProcessor* getProcessor() { return processor; }

protected:

    // --------------------------------------------
    //     PARAMETERS
    // --------------------------------------------

    /** Adds a boolean parameter, which will later be accessed by name*/
    void addBooleanParameter(const String& name,
        const String& displayName,
        const String& description,
        bool defaultValue,
        bool deactivateDuringAcquisition = false);

    /** Adds an integer parameter, which will later be accessed by name*/
    void addIntParameter(const String& name,
        const String& displayName,
        const String& description,
        int defaultValue,
        int minValue,
        int maxValue,
        bool deactivateDuringAcquisition = false);
    
    /** Adds a string parameter, which will later be accessed by name*/
    void addStringParameter(const String& name,
        const String& displayName,
        const String& description,
        String defaultValue,
        bool deactivateDuringAcquisition = false);

    /** Adds an float parameter, which will later be accessed by name*/
    void addFloatParameter(const String& name,
        const String& displayName,
        const String& description,
        const String& unit,
        float defaultValue,
        float minValue,
        float maxValue,
        float stepSize,
        bool deactivateDuringAcquisition = false);

    /** Adds a selected channels parameter, which will later be accessed by name*/
    void addSelectedChannelsParameter(const String& name,
        const String& displayName,
        const String& description,
        int maxSelectedChannels = std::numeric_limits<int>::max(),
        bool deactivateDuringAcquisition = false);
    
    /** Adds a mask channels parameter, which will later be accessed by name*/
    void addMaskChannelsParameter(const String& name,
        const String& displayName,
        const String& description,
        bool deactivateDuringAcquisition = false);

    /** Adds a categorical parameter, which will later be accessed by name*/
    void addCategoricalParameter(const String& name,
        const String& displayName,
        const String& description,
        Array<String> categories,
        int defaultIndex,
        bool deactivateDuringAcquisition = false);
    
    /** Adds a selected stream parameter which holds the currentlu selected stream */
    void addSelectedStreamParameter(const String& name,
        const String& displayName,
        const String& description,
        Array<String> streamNames,
        const int defaultIndex,
        bool deactivateDuringAcquisition = true);
    
    /** Adds a boolean parameter, which will later be accessed by name*/
    void addNotificationParameter(const String& name,
        const String& displayName,
        const String& description,
        bool deactivateDuringAcquisition = false);

private:

    GenericProcessor* processor;

    OwnedArray<ParameterEditorOwner> parameterEditorOwners;

    Array<ParameterEditor*> allParamEditors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Visualizer);
};


#endif  // __VISUALIZER_H_C5943EC1__
