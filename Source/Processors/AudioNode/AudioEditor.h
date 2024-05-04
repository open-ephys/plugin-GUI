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
#ifndef __AUDIOEDITOR_H_9D6F1FC3__
#define __AUDIOEDITOR_H_9D6F1FC3__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "AudioNode.h"
#include <stdio.h>
#include "../../Utils/Utils.h"

class AudioNode;
class AudioComponent;

/**
  Toggles audio output on and off.

  @see AudioNode, AudioEditor

*/
class MuteButton : public ImageButton
{
public:

    /** Constructor */
    MuteButton();

    /** Destructor */
    ~MuteButton() { }

    /** Updates the button's images */
    void updateImages();
  
private:

    Image onimage, offimage;

};


/**
  Used to show and hide the AudioConfigurationWindow.

  @see AudioNode, AudioEditor

*/
class AudioWindowButton : public Button
{
public:

    /** Constructor */
    AudioWindowButton();

    /** Destructor */
    ~AudioWindowButton() { }

    /** Renders the button */
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    /** Sets the latency string*/
    void setText(const String& newText);

private:
    String textString;
};


/**
  Allows the user to access audio output settings.

  @see AudioNode, AudioEditor

*/
class AudioConfigurationWindow : public DocumentWindow
{
public:

    /** Constructor */
    AudioConfigurationWindow (AudioDeviceManager& adm, AudioWindowButton* b);

    /** Destructor */
    ~AudioConfigurationWindow() { }

    /** Draws the background*/
    void paint (Graphics& g)    override;

    /** Doesn't do anything currently*/
    void resized()              override;

private:

    /** Saves settings to the recovery config and hides the window*/
    void closeButtonPressed();

    AudioWindowButton* controlButton;
};

/**
  Holds the interface for editing audio output parameters.

  @see AudioNode

*/
class AudioEditor : public AudioProcessorEditor
                  , public Button::Listener
                  , public Slider::Listener
                  , public ComponentListener
{
public:

    /** Constructor */
    AudioEditor (AudioNode* owner);

    /** Destructor (removes slider LookAndFeel)*/
    ~AudioEditor();

    /** Draws the "gate" label */
    void paint (Graphics& g)                override;

    /** Sets sub-component locations*/
    void resized()                          override;

    /** Changes the latency label*/
    void updateBufferSizeText();

    /** Called after acquisition is stopped */
    void enable();

    /** Called after acquisition begins*/
    void disable();

    /** Saves settings*/
    void saveStateToXml     (XmlElement* xml);

    /** Loads settings*/
    void loadStateFromXml   (XmlElement* xml);

    /** Gets AudioConfigurationWindow state */
    bool isAudioConfigurationWindowVisible() { return audioConfigurationWindow != nullptr; };

private:

    /** Responds to button presses*/
    void buttonClicked (Button* buttonThatWasClicked) override;

    /** Responds to volume + gate sliders*/
    void sliderValueChanged (Slider* slider) override;

    /** Responds to configuration window closing*/
    void componentVisibilityChanged(Component& component) override;

    float lastValue;

    bool isEnabled;

    ScopedPointer<MuteButton>           muteButton;
    ScopedPointer<AudioWindowButton>    audioWindowButton;

    ScopedPointer<AudioConfigurationWindow> audioConfigurationWindow;

    ScopedPointer<Slider> volumeSlider;
    ScopedPointer<Slider> noiseGateSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioEditor);
};


#endif  // __AUDIOEDITOR_H_9D6F1FC3__
