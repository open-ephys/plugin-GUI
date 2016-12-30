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
#include "../Channel/InfoObjects.h"

#include <stdio.h>

class ChannelSelectorRegion;
class ChannelSelectorButton;
class EditorButton;
class ChannelSelectorBox;
class ShowAlertMessage;

namespace Channels
{
enum ChannelsType
{
    AUDIO_CHANNELS = 0
    , RECORD_CHANNELS
    , PARAM_CHANNELS
};
}

/**
    A class that is used for selecting subset of channels by providing range in TextEditor component
    and clicking apppropriate button (either add or remove given range of channels).
*/
class SlicerChannelSelectorComponent    : public Component
                                        , public Button::Listener
                                        , public KeyListener
{
public:
    SlicerChannelSelectorComponent (Channels::ChannelsType channelsType,
                                    const String& componentName = String::empty);

    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void changeChannelsSelectionButtonClicked (SlicerChannelSelectorComponent* sender,
                                                           Button* buttonThatWasClicked,
                                                           bool isSelect) = 0;
        virtual void channelSelectorCollapsedStateChanged (SlicerChannelSelectorComponent* sender,
                                                           bool isCollapsed) = 0;
    };

    void paint (Graphics& g)    override;
    void resized()              override;

    void buttonClicked (Button* buttonThatWasClicked) override;

    bool keyPressed (const KeyPress& key, Component* originatingComponent) override;

    /** Returns the conetent of the channel selector text editor */
    String getText() const;

    /** Returns the type of the channels on which the component affects */
    Channels::ChannelsType getChannelsType() const;

    /** Either collapse or show full component */
    void setCollapsed (bool isCollapsed);

    /** Sets a listener who will handle clicks of control buttons,
        like "Select channels button", "Deselect channels button, etc. */
    void setListener (SlicerChannelSelectorComponent::Listener* listener);

    static const int MAX_HEIGHT = 45;

private:
    Channels::ChannelsType m_channelsType;

    bool m_isCollapsed;

    const Image m_dropdownArrowImage;
    const Image m_dropdownArrowImageCollapsed;

    /** Stores a pointer to the button listner which will handle clicks
        of control buttons (e.g. "Select/Deselect" channels buttons) */
    Listener* m_controlsButtonListener;

    ScopedPointer<TextEditor> m_channelSelectorTextEditor;

    ScopedPointer<Button>   m_selectChannelsButton;
    ScopedPointer<Button>   m_deselectChannelsButton;
    ScopedPointer<ImageButton> m_showComponentButton;

    // ====================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlicerChannelSelectorComponent)
};


/**
Automatically creates an interactive editor for selecting channels.

Contains tabs for "Params", "Audio", and "Record", which allow
channels to be selected for different purposes.

@see GenericEditor

*/
class PLUGIN_API ChannelSelector : public Component
                                 , public Button::Listener
                                 , private SlicerChannelSelectorComponent::Listener
                                 , public Timer
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

    /** An array of ChannelSelectorButtons used to select the channels that
    will be updated when a parameter is changed.
    paramBox: TextBox where user input is taken for param tab.
    */
    TiledButtonGroupManager parameterButtonsManager;
    SlicerChannelSelectorComponent parameterSlicerChannelSelector;

    /** An array of ChannelSelectorButtons used to select the channels that
    are sent to the audio monitor.
    audioBox: TextBox where user input is taken for audio tab
    */
    TiledButtonGroupManager audioButtonsManager;
    SlicerChannelSelectorComponent audioSlicerChannelSelector;

    /** An array of ChannelSelectorButtons used to select the channels that
    will be written to disk when the record button is pressed.
    recordBox: TextBox where user input is taken for record tab
    */
    TiledButtonGroupManager recordButtonsManager;
    SlicerChannelSelectorComponent recordSlicerChannelSelector;

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
    void paint (Graphics& g);

    // SlicerChannelSelectorComponent methods
    // =================================================================================================
    /** Implements behaviour for button for selecting/deselecting channels using list slicing */
    void changeChannelsSelectionButtonClicked (SlicerChannelSelectorComponent* sender,
                                               Button* buttonThatWasClicked,
                                               bool isSelect)       override;

    /** This is a convenient way to signal that collapsed change of channel selector has been changed */
    void channelSelectorCollapsedStateChanged (SlicerChannelSelectorComponent* sender,
                                               bool isCollapsed)    override;
    // =================================================================================================

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
    EditorButton (const String& name, const Font& f);
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



#endif  // __CHANNELSELECTOR_H_68124E35__
