/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#ifndef __GENERICEDITOR_H_DD406E71__
#define __GENERICEDITOR_H_DD406E71__

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../PluginManager/OpenEphysPlugin.h"

#include "../Parameter/ParameterEditor.h"
#include "DelayMonitor.h"
#include "StreamSelector.h"
#include "TTLMonitor.h"

class GenericProcessor;
class DrawerButton;
class TriangleButton;
class UtilityButton;

/**
    Base class for creating processor editors.

    If a processor doesn't have an editor defined, a GenericEditor will be used.

    Classes derived from this class must place their controls as child components.
    They shouldn't try to re-draw any aspects of their background.

    @see GenericProcessor, EditorViewport
*/
class PLUGIN_API GenericEditor : public AudioProcessorEditor
{
public:
    /** Constructor. */
    GenericEditor (GenericProcessor* owner);

    /** Destructor.*/
    virtual ~GenericEditor();

    /*
    ========================================================================
    ============================= JUCE METHODS =============================
    ========================================================================

    */
    /** Draws the editor's background.*/
    void paint (Graphics& g) override;

    /** Called whenever a key is pressed and the editor has keyboard focus.*/
    bool keyPressed (const KeyPress& key) override;

    /** Called when the boundaries of the editor are updated. */
    virtual void resized() override;

    // =====================================================================
    // =====================================================================
    // =====================================================================

    /** Toggles the editor's selection state.*/
    void switchSelectedState();

    /** Highlights an editor and calls editorWasClicked().*/
    void select();

    /** Highlights an editor.*/
    void highlight();

    /** Makes an editor visible if it's not already.*/
    void makeVisible();

    /** Deselects an editor.*/
    void deselect();

    /** Returns an editor's selection state.*/
    bool getSelectionState();

    /** Used to set desired width of editor. */
    void setDesiredWidth (int width);

    /** Called just prior to the start of acquisition, to allow the editor to prepare.*/
    void editorStartAcquisition();

    /** Called just prior to the start of acquisition, to allow custom commands. */
    virtual void startAcquisition() {}

    /** Called after the end of acquisition.*/
    void editorStopAcquisition();

    /** Called after the end of acquisition, to allow custom commands .*/
    virtual void stopAcquisition() {}

    /** Called at the start of a recording, to allow any components to be disabled  **/
    virtual void startRecording() {}

    /** Called at the end of a recording, to allow any components to be enabled **/
    virtual void stopRecording() {}

    /** Returns the name of the editor.*/
    String getName();

    /** Updates name if processor ID changes. */
    void updateName();

    /** Updates name on title bar. */
    void setDisplayName (const String& string);

    /** Get name on title bar. */
    String getDisplayName();

    /** Returns a string containing the editor name and underlying processor ID. */
    String getNameAndId();

    /** Returns a custom channel number for the Channel Selector buttons. Useful for channel mappers */
    virtual int getChannelDisplayNumber (int chan) const;

    /** Determines how wide the editor will be drawn. */
    int desiredWidth;

    /** The unique integer ID of the editor's processor. */
    int nodeId;

    /** Sets the number of the editor's associated tab in the DataViewport. */
    virtual void tabNumber (int t);

    /** Returns the number of the editor's associated tab in the DataViewport. */
    int tabNumber();

    /** Required for MergerEditor only.*/
    virtual void switchSource (int);

    /** Required for MergerEditor only.*/
    virtual void switchSource();

    /** Returns the processor associated with an editor.*/
    GenericProcessor* getProcessor() const;

    /** Required for SplitterEditor only.*/
    virtual void switchDest();

    /** Required for SplitterEditor and MergerEditor only.*/
    virtual void switchIO (int);

    /** Required for SplitterEditor and MergerEditor only.*/
    virtual int getPathForEditor (GenericEditor* editor);

    /** Used by GraphViewer */
    bool isSplitter();

    /** Used by GraphViewer */
    bool isMerger();

    /** Used by GraphViewer */
    bool isUtility();

    /** Used by VisualizerEditor to bring the editor's tab to the foreground.*/
    virtual void editorWasClicked();

    /** Checks to see if a button click occurred on the ChannelSelector drawer button.*/
    bool checkDrawerButton (Button* button);

    /** Selects all the channels in the input array.*/
    void selectChannels (Array<int>);

    /** Refreshes an editor's background colours when the user selects new ones with the ColourSelector.*/
    void refreshColours();

    /** Called when an editor's processor updates its settings (mainly to update channel count).*/
    void update (bool isEnabled);

    /** Allows other UI elements to use background colour of editor. */
    Colour getBackgroundColour();

    /** Changes the background colour of this editor. */
    void setBackgroundColour (Colour colour);

    /** Allows other elements to use background gradient of editor. */
    ColourGradient getBackgroundGradient();

    /** Called by the update() method to allow the editor to update its custom settings.*/
    virtual void updateSettings();

    /** Called when the editor needs to update the view of its parameters.*/
    void updateView();

    /** Called when the editor needs to update the view of its parameters.*/
    virtual void updateCustomView();

    /** Allows an editor to update the settings of its visualizer (such as channel count and sample rate).*/
    virtual void updateVisualizer();

    /** Returns the parameter editor for a given parameter name*/
    ParameterEditor* getParameterEditor (const String& parameterName);

    /** An array of pointers to ParameterEditors created based on the Parameters of an editor's underlying processor. */
    OwnedArray<ParameterEditor> parameterEditors;

    /** Stores the font used to display the editor's name. */
    FontOptions titleFont;

    /** True if data acquisition has begun. */
    bool acquisitionIsActive;

    /** Writes editor state to xml */
    void saveToXml (XmlElement* xml);

    /** Writes editor state to xml */
    void loadFromXml (XmlElement* xml);

    /** Writes editor state to xml */
    virtual void saveCustomParametersToXml (XmlElement* xml);

    /** Writes editor state to xml */
    virtual void loadCustomParametersFromXml (XmlElement* xml);

    /** Checks to see whether or not an editor is collapsed */
    bool getCollapsedState();

    /**  Sets the collapsed state for the editor*/
    void switchCollapsedState();

    /**  Sets the collapsed state for the editor*/
    void setCollapsedState (bool);

    /**  Notifies the editor that the collapsed state changed, for non-standard function. */
    virtual void collapsedStateChanged();

    /** Returns the editor of this processor's source */
    GenericEditor* getSourceEditor();

    /** Returns the editor of this processor's destination */
    GenericEditor* getDestEditor();

    /** Returns the editors a splitter or merger is connected to */
    virtual Array<GenericEditor*> getConnectedEditors();

    /** Changes the state of the TTLMonitor */
    void setTTLState (uint16 streamId, int bit, bool state);

    /** Notify editor about changes in the StreamSelector */
    void updateSelectedStream (uint16 streamId);

    /** Get the ID of the stream that's currently selected.*/
    uint16 getCurrentStream() { return selectedStream; }

    /** Called when new TTL and Delay monitors are created */
    void updateDelayAndTTLMonitors();

    /** Notifies editor that the selected stream has changed.*/
    virtual void selectedStreamHasChanged();

    /** Notifies editor that the selected stream has changed.*/
    virtual void streamEnabledStateChanged (uint16 streamId, bool enabledState, bool isLoading = false);

    /** Updates the mean latency for a particular data stream (called by LatencyMeter class)*/
    void setMeanLatencyMs (uint16 streamId, float latencyMs);

    /** Returns the total width of the editor in it's current state. */
    virtual int getTotalWidth();

    /* Returns true if it's a VisualizerEditor. */
    virtual bool isVisualizerEditor() { return false; }

protected:
    /** A pointer to the button that opens the drawer for the ChannelSelector. */
    std::unique_ptr<DrawerButton> drawerButton;

    /** Determines the width of the ChannelSelector drawer when opened. */
    int drawerWidth;

    /** Saves the open/closed state of the ChannelSelector drawer. */
    bool drawerOpen;

    /** Adds a text box editor for a parameter of a given name. */
    void addTextBoxParameterEditor (Parameter::ParameterScope scope, const String& name, int xPos, int yPos);

    /** Adds a check box editor for a parameter of a given name. */
    void addToggleParameterEditor (Parameter::ParameterScope scope, const String& name, int xPos, int yPos);

    /** Adds a bounded value editor for a parameter of a given name. */
    void addBoundedValueParameterEditor (Parameter::ParameterScope scope, const String& name, int xPos, int yPos);

    /** Adds a combo box editor for a parameter of a given name. */
    void addComboBoxParameterEditor (Parameter::ParameterScope scope, const String& name, int xPos, int yPos);

    /** Adds a selected channels editor for a parameter of a given name. */
    void addSelectedChannelsParameterEditor (Parameter::ParameterScope scope, const String& name, int xPos, int yPos);

    /** Adds a mask channels editor for a parameter of a given name. */
    void addMaskChannelsParameterEditor (Parameter::ParameterScope scope, const String& name, int xPos, int yPos);

    /** Adds a path browser editor for a parameter of a given name. */
    void addPathParameterEditor (Parameter::ParameterScope scope, const String& name, int xPos, int yPos);

    /** Adds a selected stream editor for a parameter of a given name. */
    void addSelectedStreamParameterEditor (Parameter::ParameterScope scope, const String& name, int xPos, int yPos);

    /** Adds a time editor for a paramater of a given name. */
    void addTimeParameterEditor (Parameter::ParameterScope scope, const String& name, int xPos, int yPos);

    /** Adds a ttl line editor for a parameter of a given name. */
    void addTtlLineParameterEditor (Parameter::ParameterScope scope, const String& name, int xPos, int yPos);

    /** Adds a sync line editor for a parameter of a given name. 
     * @param syncLineParam is the parameter that will be updated when the sync line is changed
     * @param syncStreamParam is the parameter that will be updated when the main sync stream is changed
    */
    void addSyncLineParameterEditor (TtlLineParameter* syncLineParam, SelectedStreamParameter* syncStreamParam, int xPos, int yPos);

    /** Adds a custom editor for a parameter of a given name. */
    void addCustomParameterEditor (ParameterEditor* editor, int xPos, int yPos);

    /** A pointer to the editor's StreamSelector. */
    std::unique_ptr<StreamSelectorTable> streamSelector;

    /** Holds the value of the stream that's currently visible*/
    uint16 selectedStream;

private:
    class ButtonResponder : public Button::Listener
    {
    public:
        ButtonResponder (GenericEditor* editor_) : editor (editor_) {}
        void buttonClicked (Button* button);

    private:
        GenericEditor* editor;
    };

    ButtonResponder drawerButtonListener;

    /** Stores the editor's background colour. */
    Colour backgroundColour;

    /** Stores the editor's background gradient. */
    ColourGradient backgroundGradient;

    bool isSelected;
    bool isEnabled;
    bool isCollapsed;

    int tNum;
    int originalWidth;

    String name;
    String displayName;

    std::map<uint16, DelayMonitor*> delayMonitors;
    std::map<uint16, TTLMonitor*> ttlMonitors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericEditor);
};

/**
  Used to show and hide the StreamSelector.

  Appears on the right-hand size of all plugins that process
  at least one DataStream (except RecordNodeEditor).

  @see GenericEditor, StreamSelector
*/
class PLUGIN_API DrawerButton : public Button
{
public:
    /** Constructor */
    DrawerButton (const String& name);

    /** Destructor */
    ~DrawerButton();

private:
    /** Render the button */
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

/**
  A button that displays a triangle facing up or down.

  Useful for incrementing or decrementing values (as in SpikeDetectorEditor).

  @see GenericEditor
*/
class PLUGIN_API TriangleButton : public Button
{
public:
    TriangleButton (int direction_);
    ~TriangleButton();

private:
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    int direction;
};

/**
  A button that displays a "load" icon.

  @see GenericEditor
*/
class PLUGIN_API LoadButton : public ImageButton
{
public:
    LoadButton (const String& name);
    ~LoadButton();
};

/**
  A button that displays a "save" icon.

  @see GenericEditor
*/
class PLUGIN_API SaveButton : public ImageButton
{
public:
    SaveButton (const String& name);
    ~SaveButton();
};

/**
  A button that displays text.

  @see GenericEditor
*/
class PLUGIN_API UtilityButton : public Button
{
public:
    UtilityButton (String label_);
    ~UtilityButton();

    void setCorners (bool UL, bool UR, bool LL, bool LR);
    void setRadius (float r);

    void setEnabledState (bool);
    bool getEnabledState();

    void setLabel (String label);
    String getLabel();

    void setFont (const FontOptions& font);

private:
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    String label;
    FontOptions font;
    bool roundUL, roundUR, roundLL, roundLR;
    float radius;
    Path outlinePath;
    Path fillPath;

    bool isEnabled;
    bool isUsingCustomFont;

    void resized() override;
    ;
};

class PLUGIN_API ColourButton : public Button
{
public:
    ColourButton (String label_, FontOptions font_);
    ~ColourButton();

    void setEnabledState (bool);
    bool getEnabledState();

    void setColours (Colour foreground, Colour background);
    void setLabel (String label);
    String getLabel();

    void setVerticalOrientation (bool state);
    void setUserDefinedData (int d);
    int getUserDefinedData();

    void setShowEnabled (bool state);

private:
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    int userDefinedData;
    bool vert;
    String label;
    FontOptions font;
    Colour fontColour, backgroundColour;
    bool showEnabledStatus;
    bool isEnabled;
};

/**
  Used to change the spike detection threshold.

  @see SpikeDetectorEditor
*/
class PLUGIN_API ThresholdSlider : public Slider
{
public:
    ThresholdSlider (FontOptions f);
    ~ThresholdSlider();

    void setActive (bool);

    void setValues (Array<double>);

private:
    void paint (Graphics& g) override;

    Path makeRotaryPath (double, double, double);

    FontOptions font;

    bool isActive;

    Array<double> valueArray;
};

/**
  Used to monitor a variable in real-time as a level bar.

  @see SpikeDetectorEditor
*/
class PLUGIN_API LevelMonitor : public Timer,
                                public Button
{
public:
    /** Constructor */
    LevelMonitor (GenericProcessor*);

    /** Detructor */
    ~LevelMonitor();

    /** Set update freq */
    void setUpdateFreq (int freq) { updateFreq = freq; };

    /** Sets fill amount */
    void setFillPercentage (float percentage);

    /** Draws the button */
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    /** Updates the display */
    void timerCallback() override = 0;

protected:
    GenericProcessor* processor;

    int updateFreq;

    float fillPercentage;
    float lastUpdateTime;
    bool stateChangeSinceLastUpdate;
};

#endif // __GENERICEDITOR_H_DD406E71__
