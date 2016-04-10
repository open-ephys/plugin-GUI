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

#ifndef __CHANNELSELECTOR_H_68124E35__
#define __CHANNELSELECTOR_H_68124E35__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"
#include "../../UI/Utils/TiledButtonGroupManager.h"

#include "../Channel/Channel.h"

#include <stdio.h>

class ChannelSelectorRegion;
class ChannelSelectorButton;
class EditorButton;
class ChannelSelectorBox;
class ShowAlertMessage;

/**
Automatically creates an interactive editor for selecting channels.

Contains tabs for "Params", "Audio", and "Record", which allow
channels to be selected for different purposes.

@see GenericEditor

*/


class PLUGIN_API ChannelSelector : public Component,
    public Button::Listener,
    public Timer
{
public:

    /** constructor */
    ChannelSelector(bool createButtons, Font& titleFont);

    /** destructor */
    ~ChannelSelector();

    /** button callback */
    void buttonClicked(Button* button);

    /** Return an array of selected channels. */
    Array<int> getActiveChannels();

    /** Set the selected channels. */
    void setActiveChannels(Array<int>);

    /** Set the total number of channels. */
    void setNumChannels(int);

    /** get the total number of channels. */
    int getNumChannels();

    /** Return whether a particular channel should be recording. */
    bool getRecordStatus(int chan);

    /** Return whether a particular channel should be monitored. */
    bool getAudioStatus(int chan);

    /** Return whether a particular channel is selected for editing parameters. */
    bool getParamStatus(int chan);

    /** Set whether a particular channel should be recording. */
    void setRecordStatus(int, bool);

    /** Set whether a particular channel should be monitored. */
    void setAudioStatus(int, bool);

    /** Sets all audio monitors to 'false' */
    void clearAudio();

    /** Set whether a particular channel is selected for editing parameters. */
    void setParamStatus(int, bool);

    /** Return component's desired width. */
    int getDesiredWidth();

    /** Called prior to the start of data acquisition.*/
    void startAcquisition();

    /** Called immediately after data acquisition ends.*/
    void stopAcquisition();

    /** Inactivates all the ChannelSelectorButtons under the "param" tab.*/
    void inactivateButtons();

    /** Activates all the ChannelSelectorButtons under the "param" tab.*/
    void activateButtons();

    /** Inactivates all the ChannelSelectorButtons under the "rec" tab.*/
    void inactivateRecButtons();

    /** Activates all the ChannelSelectorButtons under the "rec" tab.*/
    void activateRecButtons();

    /** Refreshes Parameter Colors on change*/
    void refreshParameterColors();

    /** Controls the behavior of ChannelSelectorButtons; they can either behave
    like radio buttons (only one selected at a time) or like toggle buttons (an
    arbitrary number can be selected at once).*/
    void setRadioStatus(bool);

    void paramButtonsToggledByDefault(bool t);
    //void paramButtonsActiveByDefault(bool t) {paramsActive = t;}

    /** Used to scroll channels */
    void shiftChannelsVertical(float amount);

    bool eventsOnly;

private:

    EditorButton* audioButton;
    EditorButton* recordButton;
    EditorButton* paramsButton;
    EditorButton* allButton;
    EditorButton* noneButton;
    EditorButton* selectButtonParam;   //Select Channels in parameter tab
    EditorButton* deselectButtonParam; //Deselect Channels in parameter tab
    EditorButton* selectButtonRecord;  //Select Channels in record tab
    EditorButton* deselectButtonRecord;//Deselect Channels in record tab
    EditorButton* selectButtonAudio;   //Select Channels in audio tab
    EditorButton* deselectButtonAudio; //Deselect Channels in audio tab

    /** An array of ChannelSelectorButtons used to select the channels that
    will be updated when a parameter is changed.
    paramBox: TextBox where user input is taken for param tab.
    */
    TiledButtonGroupManager parameterButtonsManager;
    ChannelSelectorBox* paramBox;

    /** An array of ChannelSelectorButtons used to select the channels that
    are sent to the audio monitor.
    audioBox: TextBox where user input is taken for audio tab
    */
    TiledButtonGroupManager audioButtonsManager;
    ChannelSelectorBox* audioBox;

    /** An array of ChannelSelectorButtons used to select the channels that
    will be written to disk when the record button is pressed.
    recordBox: TextBox where user input is taken for record tab
    */
    TiledButtonGroupManager recordButtonsManager;
    ChannelSelectorBox* recordBox;

    bool paramsToggled;
    bool paramsActive;
    bool recActive;
    bool radioStatus;

    bool isNotSink;
    bool moveRight;
    bool moveLeft;

    int offsetLR;
    float offsetUD;

    int numColumnsLessThan100;
    int numColumnsGreaterThan100;
    int overallHeight;

    int parameterOffset;
    int audioOffset;
    int recordOffset;

    int desiredOffset;

    void resized();

    void addButton();
    void removeButton();
    void refreshButtonBoundaries();

    /** Controls the speed of animations. */
    void timerCallback();

    /** Draws the ChannelSelector. */
    void paint(Graphics& g);

    Font& titleFont;

    enum { AUDIO, RECORD, PARAMETER };

    bool acquisitionIsActive;

    ChannelSelectorRegion* channelSelectorRegion;

};

/**

A button within the ChannelSelector that allows the user to switch
between tabs of all the channels.

@see ChannelSelector

*/

class EditorButton : public Button
{
public:
    EditorButton(const String& name, Font& f);
    ~EditorButton();

    bool getState();

    void setState(bool state);

private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    void resized();

    Path outlinePath;

    int type;
    Font buttonFont;

    bool isEnabled;

    ColourGradient selectedGrad, selectedOverGrad, neutralGrad, neutralOverGrad;
};


/**

A button within the ChannelSelector representing an individual channel.

@see ChannelSelector

*/

class ChannelSelectorButton : public Button
{
public:
    ChannelSelectorButton(int num, int type, Font& f);
    ~ChannelSelectorButton();

    int getType();
    int getChannel();
    //Channel* getChannel() {return ch;}
    void setActive(bool t);
    void setChannel(int n);
    void setChannel(int n, int d);

private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    //Channel* ch;

    int type;
    int num;
    int displayNum;
    Font buttonFont;
    bool isActive;
};

/*
  A textbox within the channelSelector to select multiple channels at a time.
*/
class ChannelSelectorBox :public TextEditor
{
public:
    ChannelSelectorBox();
    ~ChannelSelectorBox();


    std::vector<int> getBoxInfo(int len);  // Extract Information from the box.
    int convertToInteger(std::string s);   // Conversion of string to integer.
};

#endif  // __CHANNELSELECTOR_H_68124E35__
