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

#ifndef __STREAMSELECTORBUTTON_H_BDCEE716__
#define __STREAMSELECTORBUTTON_H_BDCEE716__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

/**

  Used to toggle data streams on and off

  @see Merger, Splitter.

*/

class DataStream;


class PLUGIN_API StreamSelectorButton : public Button
{
public:
    StreamSelectorButton(const DataStream* stream);
    ~StreamSelectorButton();

    uint16 getStreamId() const;

    const DataStream* getStream() { return stream; }
    bool getSelectedState() const;

    void setEnabled(bool state);

private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    bool isEnabled;
    bool isSelected;

    const DataStream* stream;

    String infoString;

    ColourGradient selectedGrad, selectedOverGrad, neutralGrad, neutralOverGrad;
};

class PLUGIN_API StreamButtonHolder : public Component
{
public:
    StreamButtonHolder();
    ~StreamButtonHolder();

    void add(StreamSelectorButton* button);
    void remove(StreamSelectorButton* button);

    int getDesiredWidth() { return 100; }

    void resized();

    void clear();

    const DataStream* getCurrentStream();

private:

    OwnedArray<StreamSelectorButton> buttons;

    int buttonHeight = 35;
    int buttonSpacing = 5;
};


#endif  // __STREAMSELECTORBUTTON_H_BDCEE716__
