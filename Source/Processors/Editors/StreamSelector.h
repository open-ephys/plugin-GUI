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

#ifndef __STREAMSELECTOR_H_BDCEE716__
#define __STREAMSELECTOR_H_BDCEE716__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

/**

  Used to apply plugin settings to different streams.

  @see GenericEditor

*/

class DataStream;
class GenericEditor;
class TTLMonitor;
class DelayMonitor;
class UtilityButton;

class StreamScrollButton;
class StreamNameButton;
class StreamEnableButton;

/**
 
 Displays information about a particular stream.
 
 Lives inside the StreamSelector component.
 
 */

class PLUGIN_API StreamInfoView : public Component,
    public Button::Listener
{
public:
    
    /** Constructor */
    StreamInfoView(const DataStream* stream, GenericEditor* editor, bool isEnabled);
    
    /** Destructor*/
    ~StreamInfoView() { }

    /** Returns the streamId associated with this view*/
    uint16 getStreamId() const;

    /** Returns a pointer to the original DataStream*/
    const DataStream* getStream() { return stream; }

    /** Returns true if this stream is enabled*/
    bool getEnabledState() const;

    /** Enables or disables this stream*/
    void setEnabled(bool state);

    /** Returns a pointer to the TTLMonitor for this view*/
    TTLMonitor* getTTLMonitor() { return ttlMonitor.get(); }
    
    /** Returns a pointer to the DelayMonitor for this view*/
    DelayMonitor* getDelayMonitor() { return delayMonitor.get(); }

    /** Renders the StreamInfoView*/
    void paint(Graphics& g);
    
    /** Sets the location of the sub-components*/
    void resized();

    /** Called before new streams are added*/
    void beginUpdate();
    
    /** Updates the DataStream object referenced by this view */
    void update(const DataStream* stream);
    
    /** Updates the info displayed for the underlying stream*/
    void updateInfoString();

    /** Starts animations on TTLMonitor and DelayMonitor */
    void startAcquisition();

    /** Stops animations on TTLMonitor and DelayMonitor */
    void stopAcquisition();

    /** Called when enable button is clicked */
    void buttonClicked(Button* button);

    /** Set to 'true' after update, if stream still exists*/
    bool streamIsStillNeeded;

private:

    const DataStream* stream;
    
    bool isEnabled;

    String infoString;

    GenericEditor* editor;

    std::unique_ptr<StreamEnableButton> enableButton;
    std::unique_ptr<TTLMonitor> ttlMonitor;
    std::unique_ptr<DelayMonitor> delayMonitor;
};

/**

Allows the user to browse through the available DataStreams within a given plugin

Lives inside the GenericEditor

*/
class PLUGIN_API StreamSelector : public Component,
    public Timer,
    public Button::Listener
{
public:
    
    /** Constructor*/
    StreamSelector(GenericEditor* editor);
    
    /** Destructor*/
    ~StreamSelector() { }

    /** Adds a new DataStream*/
    void add(const DataStream*);
    
    /** Removes a DataStream*/
    void remove(StreamInfoView*);

    /** Informs the GenericEditor about the component width*/
    int getDesiredWidth() { return 140; }

    /** Sets the location of the viewport and its sub-components*/
    void resized();
    
    /** Returns a pointer to the currently viewed stream*/
    const DataStream* getCurrentStream();

    /** Returns true if a given stream is enabled*/
    bool checkStream(const DataStream* stream);

    /** Renders the component*/
    void paint(Graphics& g);

    /** Called when stream browser buttons are clicked*/
    void buttonClicked(Button* button);

    /** Returns a pointer to the TTLMonitor for a given DataStream*/
    TTLMonitor* getTTLMonitor(const DataStream* stream);

    /** Returns a pointer to the DelayMonitor for a given DataStream*/
    DelayMonitor* getDelayMonitor(const DataStream* stream);

    /** Returns a pointer to the StreamInfoView for a given DataStream*/
    StreamInfoView* getStreamInfoView(const DataStream* stream);

    /** Starts TTLMonitor and DelayMonitor animations*/
    void startAcquisition();

    /** Stops TTLMonitor and DelayMonitor animations*/
    void stopAcquisition();

    /** Begins TTLMonitor and DelayMonitor animations*/
    void timerCallback();

    /** Used for scrolling animations*/
    void beginUpdate();
    
    /** Signals that all streams have been copied, and returns ID of selected stream*/
    uint16 finishedUpdate();

    /** Returns the total number of streams for this plugin*/
    int getNumStreams() { return streams.size(); }

    /** Used to enable and disable a given stream*/
    void setStreamEnabledState(uint16 streamId, bool isEnabled);

private:

    std::unique_ptr<Viewport> viewport;
    std::unique_ptr<Component> viewedComponent;

    std::unique_ptr<StreamScrollButton> leftScrollButton;
    std::unique_ptr<StreamScrollButton> rightScrollButton;
    std::unique_ptr<StreamNameButton> streamSelectorButton;

    OwnedArray<StreamInfoView> streams;

    std::map<uint16, bool> streamStates;

    int streamInfoViewWidth;
    int streamInfoViewHeight;

    SmoothedValue<float, ValueSmoothingTypes::Linear> scrollOffset;

    int viewedStreamIndex;

    GenericEditor* editor;

};


/**
  Arrow buttons used to select different streams
*/
class StreamScrollButton : public Button
{
public:
    
    /** Constructor*/
    StreamScrollButton(const String& name);
    
    /** Destructor*/
    ~StreamScrollButton() { }

    /** Enables/disables the button*/
    void setEnabledState(bool isEnabled_) { isEnabled = isEnabled_; }

private:
    
    /** Renders the button*/
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;

    bool isEnabled;
};


/**
  Displays the name of the current stream
*/
class StreamNameButton : public Button
{
public:
    
    /** Constructor*/
    StreamNameButton(const String& name);
    
    /** Destructor*/
    ~StreamNameButton() { }

    /** Enables/disables the button*/
    void setEnabledState(bool isEnabled_) { isEnabled = isEnabled_;  }

private:
    
    /** Renders the button*/
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;

    bool isEnabled;
};

/**
  Displays the name of the current stream
*/
class StreamEnableButton : public Button
{
public:
    
    /** Constructor*/
    StreamEnableButton(const String& name);
    
    /** Destructor*/
    ~StreamEnableButton() { }

    /** Enables/disables the button*/
    void setEnabledState(bool isEnabled_) { isEnabled = isEnabled_; }

private:
    
    /** Renders the button*/
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;

    bool isEnabled;
};

#endif  // __STREAMSELECTOR_H_BDCEE716__
