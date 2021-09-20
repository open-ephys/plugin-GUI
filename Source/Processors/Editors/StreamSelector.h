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

class PLUGIN_API StreamInfoView : public Component,
    public Button::Listener
{
public:
    StreamInfoView(const DataStream* stream, GenericEditor* editor);
    ~StreamInfoView();

    uint16 getStreamId() const;

    const DataStream* getStream() { return stream; }

    bool getEnabledState() const;

    void setEnabled(bool state);

    TTLMonitor* getTTLMonitor() { return ttlMonitor.get(); }
    DelayMonitor* getDelayMonitor() { return delayMonitor.get(); }

    void paint(Graphics& g);
    void resized();

    void startAcquisition();

    void stopAcquisition();

    void buttonClicked(Button* button);

private:
   
    bool isEnabled;

    const DataStream* stream;

    String infoString;

    GenericEditor* editor;

    std::unique_ptr<StreamEnableButton> enableButton;
    std::unique_ptr<TTLMonitor> ttlMonitor;
    std::unique_ptr<DelayMonitor> delayMonitor;
};

class PLUGIN_API StreamSelector : public Component,
    public Timer,
    public Button::Listener
{
public:
    StreamSelector(GenericEditor* editor);
    ~StreamSelector();

    void add(StreamInfoView*);
    void remove(StreamInfoView*);

    int getDesiredWidth() { return 140; }

    void resized();

    void clear();

    const DataStream* getCurrentStream();

    bool isStreamEnabled(const DataStream* stream);

    void paint(Graphics& g);

    void buttonClicked(Button* button);

    TTLMonitor* getTTLMonitor(const DataStream* stream);

    DelayMonitor* getDelayMonitor(const DataStream* stream);

    void startAcquisition();

    void stopAcquisition();

    void timerCallback();

private:

    std::unique_ptr<Viewport> viewport;
    std::unique_ptr<Component> viewedComponent;

    std::unique_ptr<StreamScrollButton> leftScrollButton;
    std::unique_ptr<StreamScrollButton> rightScrollButton;
    std::unique_ptr<StreamNameButton> streamSelectorButton;

    OwnedArray<StreamInfoView> streams;

    StreamInfoView* getStreamInfoView(const DataStream* stream);

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
    StreamScrollButton(const String& name);
    ~StreamScrollButton() { }

    void setEnabledState(bool isEnabled_) { isEnabled = isEnabled_; }

private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;

    bool isEnabled;
};


/**
  Displays the name of the current stream
*/
class StreamNameButton : public Button
{
public:
    StreamNameButton(const String& name);
    ~StreamNameButton() { }

    void setEnabledState(bool isEnabled_) { isEnabled = isEnabled_;  }

private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;

    bool isEnabled;
};

/**
  Displays the name of the current stream
*/
class StreamEnableButton : public Button
{
public:
    StreamEnableButton(const String& name);
    ~StreamEnableButton() { }

    void setEnabledState(bool isEnabled_) { isEnabled = isEnabled_; }

private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;

    bool isEnabled;
};

#endif  // __STREAMSELECTOR_H_BDCEE716__
