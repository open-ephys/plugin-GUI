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

#include "GenericEditor.h"
#include "TTLMonitor.h"
#include "DelayMonitor.h"

/**

  Used to apply plugin settings to different streams.

  @see GenericEditor

*/

class DataStream;

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

    void paint(Graphics& g);
    void resized();

    void buttonClicked(Button* button);

private:
   
    bool isEnabled;

    const DataStream* stream;

    String infoString;

    GenericEditor* editor;

    std::unique_ptr<UtilityButton> enableButton;
    std::unique_ptr<TTLMonitor> ttlMonitor;
    std::unique_ptr<DelayMonitor> delayMonitor;
};

class PLUGIN_API StreamSelector : public Component,
    public Button::Listener
{
public:
    StreamSelector();
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

private:

    std::unique_ptr<Viewport> viewport;

    std::unique_ptr<UtilityButton> leftScrollButton;
    std::unique_ptr<UtilityButton> rightScrollButton;
    std::unique_ptr<UtilityButton> streamSelectorButton;

    OwnedArray<StreamInfoView> streams;

    int streamInfoViewWidth;
    int streamInfoViewHeight;

};


#endif  // __STREAMSELECTOR_H_BDCEE716__
