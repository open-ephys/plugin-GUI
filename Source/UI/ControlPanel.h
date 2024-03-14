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

#ifndef __CONTROLPANEL_H_AD81E528__
#define __CONTROLPANEL_H_AD81E528__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Audio/AudioComponent.h"
#include "../Processors/AudioNode/AudioEditor.h"
#include "../Processors/ProcessorGraph/ProcessorGraph.h"
#include "../Processors/RecordNode/RecordNode.h"
#include "../Processors/RecordNode/RecordEngine.h"
#include "LookAndFeel/CustomLookAndFeel.h"
#include "../AccessClass.h"
#include "../Processors/Editors/GenericEditor.h" // for UtilityButton
#include "FilenameConfigWindow.h"
#include <queue>

/** 

    Allows the user to specify custom file names,
    instead of always using the auto-generated date string

*/
class FilenameEditorButton : public TextButton
{
public:

    /** Constructor */
    FilenameEditorButton();

    /** Destructor */
    ~FilenameEditorButton() {}
};


/**

  Toggles data acquisition on and off.

  The PlayButton is located in the ControlPanel. Clicking it toggles the state
  of the ProcessorGraph to either begin the callbacks that drive data through
  the graph (acquisition on) or end these callbacks (acquisition off).

  Acquisition can also be started by pressing the RecordButton
  (assuming callbacks are not already active).

  @see ControlPanel, ProcessorGraph

*/
class PlayButton : public DrawableButton
{
public:
    
    /** Constructor*/
    PlayButton();
    
    /** Destructor*/
    ~PlayButton() { }
};

/**

  Toggles recording on and off.

  The RecordButton is located in the ControlPanel. Clicking it toggles the
  state of the RecordNode to either begin saving data (recording on) or
  stop saving data (recording off).

  If the RecordButton is pressed while data acquisition is inactive, it
  will automatically start data acquisition before recording.

  @see ControlPanel, RecordNode

*/

class RecordButton : public DrawableButton
{
public:
    
    /** Constructor*/
    RecordButton();
    
    /** Destructor*/
    ~RecordButton() { }
};

/**

  Displays the CPU load used up by the data processing callbacks.

  The CPUMeter is located in the ControlPanel. Whenever acquisition is active,
  it uses a built-in JUCE method to display the CPU load required to run the ProcessorGraph.

  It's not clear how accurate the meter is, nor how it deals with CPUs using multiple cores.

  For a more accurate measurement of CPU load, it's recommended to use a graphical
  interface or type 'top' inside a terminal.

  @see ControlPanel

*/

class CPUMeter : public Label
{
public:
    
    /** Constructor*/
    CPUMeter();
    
    /** Destructor*/
    ~CPUMeter() { }

    /** Updates the load level displayed by the CPUMeter. Called by
         the ControlPanel. */
    void updateCPU(float usage);

    /** Draws the CPUMeter. */
    void paint(Graphics& g);

private:

    Font font;
    float cpu;

};

/**

  Displays the amount of disk space left in the current data directory.

  The DiskSpaceMeter is located in the ControlPanel. When the GUI is launched (or the data directory
  is changed), a built-in JUCE method is used to find the amount of free space.

  Note that the DiskSpaceMeter currently displays only relative, not absolute disk space.

  @see ControlPanel

*/

class DiskSpaceMeter : public Component,
                       public SettableTooltipClient
{
public:
    
    /** Constructor*/
    DiskSpaceMeter();
    
    /** Destructor*/
    ~DiskSpaceMeter() { }

    /** Updates the free disk space displayed by the DiskSpaceMeter. Called by
    	the ControlPanel. */
    void updateDiskSpace(float percent);

    /** Draws the DiskSpaceMeter. */
    void paint(Graphics& g);

private:

    Font font;

    float diskFree;

};

/**

  Displays the time.

  The Clock is located in the ControlPanel. If acquisition (but not recording) is
  active, it displays (in yellow) the cumulative amount of time that the GUI has been acquiring data since
  the application was launched. If recording is active, the Clock displays (in red) the
  cumulative amount of time that recording has been active.

  The Clock uses built-in JUCE functions for getting the system time. It does not
  currently interact with timestamps from ProcessorGraph sources.

  @see ControlPanel

*/

class Clock : public Component
{
public:

    enum Mode {
        DEFAULT,
        HHMMSS
    };
    
    /** Constructor*/
    Clock();
    
    /** Destructor*/
    ~Clock() { }

    /** Starts the acquisition (yellow) clock.*/
    void start();

    /** Stops the acquisition (yellow) clock.*/
    void stop();

    /** Starts the recording (red) clock.*/
    void startRecording();

    /** Stops the recording (red) clock.*/
    void stopRecording();

    /** Sets the cumulative recording time to zero.*/
    void resetRecordTime();

    /** Renders the clock.*/
    void paint(Graphics& g);

    /** Sets the clock mode*/
	void setMode(Mode m);

    /** Gets the clock mode*/
    Mode getMode() { return mode; }

    /** Responds to right clicks*/
    void mouseDown(const MouseEvent& e);

private:

    /** Draws the current time.*/
    void drawTime(Graphics& g);

    int64 lastTime;

    int64 totalTime;
    int64 totalRecordTime;

    bool isRunning;
    bool isRecording;

    Font clockFont;

    Mode mode;

};

/**

  Used to show and hide the file browser within the ControlPanel.

  The ControlPanel contains a JUCE FilenameComponent used to change the
  data directory. When not in use, this component can be hidden using
  the ControlPanelButton.

  @see ControlPanel

*/

class ControlPanelButton : public Component, public SettableTooltipClient
{
public:
    
    /** Constructor*/
    ControlPanelButton(ControlPanel* cp_);
    
    /** Destructor*/
    ~ControlPanelButton() { }

    /** Returns the open/closed state of the ControlPanelButton.*/
    bool isOpen()
    {
        return open;
    }

    /** Toggles the open/closed state of the ControlPanelButton.*/
    void toggleState();

    /** Sets the open/closed state of the ControlPanelButton.*/
    void setState(bool);

    /** Draws the button. */
    void paint(Graphics& g);

    /** Responds to mouse clicks within the button. */
    void mouseDown(const MouseEvent& e);

private:

    ControlPanel* cp;
    
    Path openPath, closedPath;

    bool open;

};

class UtilityButton;

/**

  Provides general application controls along the top of the MainWindow.

  Displays useful information and provides buttons to control acquistion and recording.

  The ControlPanel contains the PlayButton, the RecordButton, the CPUMeter,
  the DiskSpaceMeter, the Clock, the AudioEditor, and a FilenameComponent for switching the
  current data directory.

  @see UIComponent

*/

class ControlPanel : public Component,
    public Button::Listener,
    public Timer,
    public Label::Listener,
    public ComboBox::Listener,
    public ComponentListener

{
public:
    /** Constructor */
    ControlPanel(ProcessorGraph* graph, AudioComponent* audio);

    /** Destructor */
    ~ControlPanel();

    /** Disables the callbacks of the ProcessorGraph (used to
        drive data acquisition).*/
    void disableCallbacks();

    /** Sets whether or not the FilenameComponent is visible.*/
    void openState(bool isOpen);

    /** Toggles the visibility of the FilenameComponent.*/
    void toggleState();

    /** Return current acquisition state.*/
    bool getAcquisitionState();

    /** Used to manually turn recording on and off.*/
    void setAcquisitionState(bool state);

    /** Called to start acquisition*/
    void startAcquisition(bool startRecording = false);

    /** Called to end acquisition */
    void stopAcquisition();

    /** Returns a boolean that indicates whether or not the FilenameComponet
        is visible. */
    bool isOpen()
    {
        return open;
    }

    /** Notifies the control panel when the filename is updated */
    void labelTextChanged(Label*);

    /** Used to manually turn recording on and off.*/
    void setRecordingState(bool isRecording, bool force=false);

    /** Returns true if recording is active, false otherwise. */
    bool getRecordingState();

    /** Sets the parent recording directory.

        The parent recording directory is inherited by all subsequent Record Nodes
        that are placed in the signal chain. Existing Record Nodes will not be
        affected by this change.

        The actual recording location is determined by three strings:

        <parent_directory>/<directory_name>/Record Node <ID>

        "parent_directory" can be set independently in the Control Panel
        and individual Record Nodes.

        "directory_name" is generated when recording starts, and is
        shared by all Record Nodes

        "Record Node <ID>" is based on the processor ID for each Record Node,
        and can't be changed manually.
    */
    void setRecordingParentDirectory(String path);

    /** Returns the current parent recording diretory*/
    File getRecordingParentDirectory();

    /** Gets the base name of the recording directory */
    String getRecordingDirectoryBaseText();

    /** Sets the base name of the recording directory (overrides the auto-generated text,
        but not prepend or append text)*/
    void setRecordingDirectoryBaseText(String text);

    /** Gets the name of the current recording directory (including prepend and append text)
    
        Returns an empty string if recording has not been started yet.
    */
    String getRecordingDirectoryName();

    /** Will generate a new directory name the next time recording is started*/
    void createNewRecordingDirectory();

    /** Returns the prepend text used to generate the recording directory name */
    String getRecordingDirectoryPrependText();

    /** Manually sets the text to be prepended to the recording directory (overrides auto-generated text)*/
    void setRecordingDirectoryPrependText(String text);

    /** Returns the append text used to generate the recording directory name */
    String getRecordingDirectoryAppendText();

    /** Manually sets the text to be appended to the recording directory (overrides auto-generated text)*/
    void setRecordingDirectoryAppendText(String text);

    /** Save settings. */
    void saveStateToXml(XmlElement*);

    /** Load settings. */
    void loadStateFromXml(XmlElement*);

    /** Returns a list of recently used directories for saving data. */
    StringArray getRecentlyUsedFilenames();

    /** Sets the list of recently used directories for saving data. */
    void setRecentlyUsedFilenames (const StringArray& filenames);

    /** Queries the RecordEnginerManager for available engines when the GUI launches*/
    void updateRecordEngineList();

    /** Selects a new record engine */
    void setSelectedRecordEngine(int index);

    /** Returns a list of available engines*/
    std::vector<RecordEngineManager*> getAvailableRecordEngines();

    /** Returns the name of the currently selected record engine*/
    String getSelectedRecordEngineId();

    /** Sets the current record engine (will only apply to future Record Nodes) */
	bool setSelectedRecordEngineId(String id);

    /** Generates the current datetime based on the input formatting string */
    String generateDatetimeFromFormat(String format);

    std::unique_ptr<FilenameEditorButton> filenameText;
    std::unique_ptr<FilenameConfigWindow> filenameConfigWindow;

    std::unique_ptr<Clock> clock;

private:

    /** Informs the Control Panel that recording has begun.*/
    void startRecording();

    /** Informs the Control Panel that recording has stopped.*/
    void stopRecording();

    /** Generates prepend string for recording directory */
    String generatePrepend(String format);

    /** Generates append string for recording directory */
    String generateAppend(String format);

    /** Generates the next recording directory based on field settings **/
    String generateFilenameFromFields(bool usePlaceholderText);
    
    bool forceRecording;

    std::unique_ptr<PlayButton> playButton;

    std::unique_ptr<CPUMeter> cpuMeter;
    std::unique_ptr<DiskSpaceMeter> diskMeter;
    std::unique_ptr<FilenameComponent> filenameComponent;
    std::unique_ptr<UtilityButton> newDirectoryButton;
    std::unique_ptr<ControlPanelButton> cpb;
    std::unique_ptr<RecordButton> recordButton;
    std::unique_ptr<ComboBox> recordSelector;

    Array<std::shared_ptr<FilenameFieldComponent>> filenameFields;

    /* Popup window for editing recording filename fields */
    void componentBeingDeleted(Component &component);

    ProcessorGraph* graph;
    AudioComponent* audio;
    AudioEditor* audioEditor;

    void paint(Graphics& g);

    void resized();

    void paintButton(Graphics& g);
    void buttonClicked(Button* button);

    void comboBoxChanged(ComboBox* combo);

    bool initialize;

    void timerCallback();

    /** Updates the values displayed by the CPUMeter and DiskSpaceMeter.*/
    void refreshMeters();

    bool keyPressed(const KeyPress& key);

    Font font;

    bool open;

    Path p1, p2;

    /** Draws the boundaries around the FilenameComponent.*/
    void createPaths();

    String recordingDirectoryName;

    Colour backgroundColour;

    OwnedArray<RecordEngineManager> recordEngines;
    std::unique_ptr<UtilityButton> recordOptionsButton;
    int lastEngineIndex;

};


#endif  // __CONTROLPANEL_H_AD81E528__
