/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#ifndef __PARAMETEREDITOR_H_44537DA9__
#define __PARAMETEREDITOR_H_44537DA9__

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "Parameter.h"
#include "../Editors/PopupChannelSelector.h"

class UtilityButton;

/** 
    Base class for ParameterEditors

    All custom ParameterEditors must inherit from this class.
*/
class ParameterEditor : public Component
{
public:

    ParameterEditor(Parameter* param_) : param(param_), name(param_->getName()) { }

    virtual ~ParameterEditor() { }

    virtual void updateView() = 0;

    void setParameter(Parameter* param_)
    {
        param = param_;
        updateView();
    }

    bool shouldDeactivateDuringAcquisition()
    {
        return param->shouldDeactivateDuringAcquisition();
    }

    const String getParameterName() { return name; }

protected:
    Parameter* param;
    
    String name;
};

/** 
    Allows parameters to be changed via text box.

    Only valid for IntParameter and FloatParameter

*/
class PLUGIN_API TextBoxParameterEditor : public ParameterEditor,
    public Label::Listener
{
public:
    TextBoxParameterEditor(Parameter* param);
    virtual ~TextBoxParameterEditor() { }

    void labelTextChanged(Label* label);

    virtual void updateView() override;

    virtual void resized();

private:
    std::unique_ptr<Label> parameterNameLabel;
    std::unique_ptr<Label> valueTextBox;

    int finalWidth;
};


/**
    Allows parameters to be changed via a check box.

    Only valid for BooleanParameter

*/
class PLUGIN_API CheckBoxParameterEditor : public ParameterEditor,
    public Button::Listener
{
public:
    CheckBoxParameterEditor(Parameter* param);
    virtual ~CheckBoxParameterEditor() { }

    void buttonClicked(Button* label);

    virtual void updateView() override;

    virtual void resized();

private:
    std::unique_ptr<Label> parameterNameLabel;
    std::unique_ptr<ToggleButton> valueCheckBox;
};


/**
    Allows parameters to be changed via combo box.

    Only valid for BooleanParameter, IntParameter, and CategoricalParameter

*/
class PLUGIN_API ComboBoxParameterEditor : public ParameterEditor,
    public ComboBox::Listener
{
public:
    ComboBoxParameterEditor(Parameter* param);
    virtual ~ComboBoxParameterEditor() { }

    void comboBoxChanged(ComboBox* comboBox);

    virtual void updateView() override;

    virtual void resized();

private:
    std::unique_ptr<Label> parameterNameLabel;
    std::unique_ptr<ComboBox> valueComboBox;

    int offset;
};

class SliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SliderLookAndFeel() { }
    ~SliderLookAndFeel() { }

    Slider::SliderLayout getSliderLayout (Slider& slider) override;

    void drawRotarySlider (Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, Slider&) override;

    Label* createSliderTextBox (Slider& slider) override;

private:
    Colour blue      = Colour::fromFloatRGBA (0.43f, 0.83f, 1.0f,  1.0f);
    Colour offWhite  = Colour::fromFloatRGBA (0.83f, 0.84f, 0.9f,  1.0f);
    Colour grey      = Colour::fromFloatRGBA (0.42f, 0.42f, 0.42f, 1.0f);
    Colour blackGrey = Colour::fromFloatRGBA (0.2f,  0.2f,  0.2f,  1.0f);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderLookAndFeel);
};

class PLUGIN_API CustomSlider : public Slider
{
public:
    CustomSlider();
    ~CustomSlider();

    void mouseDown (const MouseEvent& event) override;
    void mouseUp (const MouseEvent& event) override;
    
    SliderLookAndFeel sliderLookAndFeel;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomSlider)
};


/**
    Allows parameters to be changed via a slider

    Only valid for IntParameter and FloatParameter

*/
class PLUGIN_API SliderParameterEditor : public ParameterEditor,
    public Slider::Listener
{
public:
    SliderParameterEditor(Parameter* param);
    virtual ~SliderParameterEditor() { }
    
    void sliderValueChanged(Slider* slider) override;

    virtual void updateView() override;

    virtual void resized() override;

private:
    std::unique_ptr<Label> parameterNameLabel;
    std::unique_ptr<CustomSlider> slider;
    
};

/**
    Creates a special editor for a SelectedChannelsParameter

    Displays all of the channels in the currently active DataStream,
    and makes it possible to select them by clicking.

*/
class PLUGIN_API SelectedChannelsParameterEditor : public ParameterEditor,
    public Button::Listener,
    public PopupChannelSelector::Listener
{
public:
    SelectedChannelsParameterEditor(Parameter* param);
    virtual ~SelectedChannelsParameterEditor() { }

    void buttonClicked(Button* label);

    virtual void updateView() override;

    void channelStateChanged(Array<int> selectedChannels);

    virtual void resized();

private:
    std::unique_ptr<UtilityButton> button;
};

/**
    Creates a special editor for a MaskChannelsParameter

    Displays all of the channels in the currently active DataStream,
    and makes it possible to select them by clicking.

*/
class PLUGIN_API MaskChannelsParameterEditor : public ParameterEditor,
    public Button::Listener,
    public PopupChannelSelector::Listener
{
public:
    MaskChannelsParameterEditor(Parameter* param);
    virtual ~MaskChannelsParameterEditor() { }

    void buttonClicked(Button* label);

    virtual void updateView() override;

    void channelStateChanged(Array<int> selectedChannels);

    virtual void resized();

private:
    std::unique_ptr<UtilityButton> button;
};

#endif  // __PARAMETEREDITOR_H_44537DA9__
