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
#include "../Editors/SyncLineSelector.h"
#include "../Editors/PopupTimeEditor.h"
#include "../Synchronizer/Synchronizer.h"

#include "../../UI/LookAndFeel/CustomLookAndFeel.h"

/** 
    Base class for ParameterEditors

    All custom ParameterEditors must inherit from this class.
*/
class PLUGIN_API ParameterEditor : public Component,
                        public Parameter::Listener
{
public:

    /** Constructor */
    ParameterEditor(Parameter* param_) : param(param_), name(param_->getName()) 
    {
        if (param->getKey().compare("UNKNOWN") != 0)
            param->addListener(this);
        
        layout = Layout::nameOnRight;

        m_updateOnSelectedStreamChanged = true;
    }

    /** Destructor */
    virtual ~ParameterEditor() { 
    
        if(param != nullptr)
            param->removeListener(this);
    
    }

    /** Used to specify layout */
    enum Layout {
        nameOnTop,
        nameOnBottom,
        nameOnLeft,
        nameOnRight,
        nameHidden
    };
    
    /** Sets the layout for this editor */
    void setLayout(Layout layout);

    /** Implements Parameter::Listener */
    void parameterChanged(Parameter* param)
    {
        const MessageManagerLock mml;
        
        updateView();
    }

    /** Implements Parameter::Listener */
    void removeParameter(Parameter* param_)
    {
        if(param == param_)
        {
            const MessageManagerLock mml;

            param = nullptr;
            updateView();
        }
    }

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() = 0;

    /** Sets the parameter corresponding to this editor*/
    void setParameter(Parameter* newParam)
    {
        if(param != nullptr)
            param->removeListener(this);

        if(newParam != nullptr)
        {
            newParam->addListener(this);
            setEnabled(newParam->isEnabled());
        }

        param = newParam;
        updateView();
    }

    void parameterEnabled(bool isParamEnabled) override
    {
        setEnabled(isParamEnabled);
    }

    /** Returns true if this editor should be disabled during acquisition*/
    bool shouldDeactivateDuringAcquisition()
    {
        if (param != nullptr)
            return param->shouldDeactivateDuringAcquisition();
        else
            return false;
    }

    /** Returns true if this editor should update when the selected stream is changed */
    bool shouldUpdateOnSelectedStreamChanged()
    {
        return m_updateOnSelectedStreamChanged;
    }

    /** Disables editor updates when the selected stream has changes 
     * @see RecordNode Sync Line editors
    */
    void disableUpdateOnSelectedStreamChanged() {
        m_updateOnSelectedStreamChanged = false;
    }

    /** Returns the name of the underlying parameter*/
    const String getParameterName() { return name; }

    /** Returns a pointer to the parameter name label, for customization */
    Label* getLabel() { return label.get(); }

    /** Returns a pointer to the parameter editor element for customization */
    Component* getEditor() { return editor; }

protected:
    Parameter* param;
    
    String name;

    std::unique_ptr<Label> label = nullptr;
    Component* editor = nullptr;

    Layout layout;

    /** Updates label and editor bounds based on layout */
    void updateBounds();

    bool m_updateOnSelectedStreamChanged;
};


/** 
    An editable label that only accepts specific characters
*/
class PLUGIN_API CustomTextBox : public Label
{
public:

    /** Constructor */
    CustomTextBox(const String& name, const String& text, const String& allowedCharacters, const String& units = "") 
        : Label(name, text),
          allowedChars(allowedCharacters),
          units(units)
    {};

    /** Destructor */
    ~CustomTextBox() {};

    /** Creates the editor component */
    TextEditor* createEditorComponent() override;

    void paint(Graphics& g) override;

private:
    String allowedChars;
    String units;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomTextBox)
};


/** 
    Allows parameters to be changed via text box.

    Only valid for IntParameter and FloatParameter

*/
class PLUGIN_API TextBoxParameterEditor : public ParameterEditor,
    public Label::Listener
{
public:

    /** Constructor */
    TextBoxParameterEditor(Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 160);

    /** Destructor */
    virtual ~TextBoxParameterEditor() { }

    /** Called when the text box contents are changed*/
    void labelTextChanged(Label* label) override;

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() override;
    
    /** Returns a pointer to the value label, for customization */
    Label* getValueLabel() { return valueTextBox.get(); }

    /** Sets sub-component locations */
    virtual void resized() override;

private:
    std::unique_ptr<CustomTextBox> valueTextBox;
};


class PLUGIN_API CustomToggleButton : public ToggleButton
{
public:
    CustomToggleButton() : ToggleButton("") {}
    ~CustomToggleButton() { }

    void paintButton (Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

/**
    Allows parameters to be changed via a check box.

    Only valid for BooleanParameter

*/
class PLUGIN_API ToggleParameterEditor : public ParameterEditor,
    public Button::Listener
{
public:

    /** Constructor */
    ToggleParameterEditor(Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 160);

    /** Destructor */
    virtual ~ToggleParameterEditor() { }

    /** Responds to checkbox clicks */
    void buttonClicked(Button* label) override;

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() override;

    /** Sets sub-component locations */
    virtual void resized() override;

private:
    std::unique_ptr<CustomToggleButton> toggleButton;
};


/**
    Allows parameters to be changed via combo box.

    Only valid for BooleanParameter, IntParameter, and CategoricalParameter

*/
class PLUGIN_API ComboBoxParameterEditor : public ParameterEditor,
    public ComboBox::Listener
{
public:

    /** Constructor */
    ComboBoxParameterEditor(Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 160);

    /** Destructor */
    virtual ~ComboBoxParameterEditor() { }

    /** Responds to combo box clicks */
    void comboBoxChanged(ComboBox* comboBox) override;

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() override;

    /** Sets sub-component locations */
    virtual void resized() override;

private:
    std::unique_ptr<ComboBox> valueComboBox;

    int offset;
};

class SliderLookAndFeel : public juce::LookAndFeel_V4
{
public:

    /** Constructor */
    SliderLookAndFeel() { }

    /** Destructor */
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
    void mouseDrag(const MouseEvent& event) override;
    
    SliderLookAndFeel sliderLookAndFeel;

    void setEnabled(bool isEnabled_) { isEnabled = isEnabled_; }

private:

    bool isEnabled;

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

    /** Constructor */
    SliderParameterEditor(Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 160);

    /** Destructor */
    virtual ~SliderParameterEditor() { }

    /** Responds to slider value changes */
    void sliderValueChanged(Slider* slider) override;

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() override;

    /** Sets sub-component locations */
    virtual void resized() override;

private:
    std::unique_ptr<CustomSlider> slider;
};

class PLUGIN_API BoundedValueEditor : public Label
{
public:

    /** Constructor (float) */
    BoundedValueEditor(float min, float max, float step, String units_ = "")
        : Label("",""), minValue(double(min)), maxValue(double(max)), stepSize(double(step)), units(units_) {
            setEditable(true, true, false);
        }

    /** Constructor (int) */
    BoundedValueEditor(int min, int max, int step, String units_ = "")
        : Label("",""), minValue(double(min)), maxValue(double(max)), stepSize(double(step)), units(units_) {
            setEditable(true, true, false);
        }

    /** Destructor */
    ~BoundedValueEditor() {};

    /** Customize text location while editing */
    TextEditor* createEditorComponent() override;

    /** Mouse event handlers */
    void mouseDrag(const MouseEvent& event) override;

    void mouseUp (const MouseEvent&) override;
    
private:

    void paint (Graphics& g) override;

    bool mouseWasDragged = false;

    double minValue;
    double maxValue;
    double stepSize;

    String units;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BoundedValueEditor)
};

/**
    Allows bounded parameters to be changed via a custom label editor

    Only valid for IntParameter and FloatParameter

*/
class PLUGIN_API BoundedValueParameterEditor : public ParameterEditor,
    public Label::Listener
{
public:

    /** Constructor */
    BoundedValueParameterEditor(Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 160);

    /** Destructor */
    virtual ~BoundedValueParameterEditor() { }
    
    /** Responds to label value changes */
    void labelTextChanged(Label* label) override;

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() override;

    /** Sets sub-component locations */
    virtual void resized() override;


private:
    std::unique_ptr<BoundedValueEditor> valueEditor;
};

/**
    Creates a special editor for a SelectedChannelsParameter

    Displays all of the channels in the currently active DataStream,
    and makes it possible to select them by clicking.

*/
class PLUGIN_API SelectedChannelsParameterEditor : 
    public ParameterEditor,
    public Button::Listener,
    public PopupChannelSelector::Listener
{
public:

    /** Constructor */
    SelectedChannelsParameterEditor(Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 160);

    /** Destructor */
    virtual ~SelectedChannelsParameterEditor() { }

    /** Displays the PopupChannelSelector*/
    void buttonClicked(Button* label) override;

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() override;

    /** Get selected channels */
    Array<int> getSelectedChannels() override;

    /** Responds to changes in the PopupChannelSelector*/
    void channelStateChanged(Array<int> selectedChannels) override;

    /** Sets sub-component locations */
    virtual void resized() override;

private:
    std::unique_ptr<TextButton> button;
};

class PLUGIN_API SelectedStreamParameterEditor : public ParameterEditor,
    public ComboBox::Listener
{
public:

    /** Constructor */
    SelectedStreamParameterEditor(Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 160);

    /** Destructor */
    virtual ~SelectedStreamParameterEditor() { }

    /** Responds to combo box clicks */
    void comboBoxChanged(ComboBox* comboBox) override;

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() override;

    /** Sets sub-component locations */
    virtual void resized() override;

private:
    std::unique_ptr<ComboBox> valueComboBox;

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

    /** Constructor */
    MaskChannelsParameterEditor(Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 160);

    /** Destructor */
    virtual ~MaskChannelsParameterEditor() { }

    /** Displays the PopupChannelSelector*/
    void buttonClicked(Button* label) override;

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() override;

    /** Get selected channels */
    Array<int> getSelectedChannels() override;

    /** Responds to changes in the PopupChannelSelector*/
    void channelStateChanged(Array<int> selectedChannels) override;

    /** Sets sub-component locations */
    virtual void resized() override;

private:
    std::unique_ptr<Button> button;
};


/**
    
    Button for selecting a sync line for a particular data stream
    Shows the synchronization status of the stream as well as whether 
    it is the main stream or not
 */
class PLUGIN_API SyncControlButton :
    public Button,
    public Timer
{
public:
    
    /** Constructor */
    SyncControlButton(SynchronizingProcessor* node,
                      const String& name,
                      String streamKey,
                      int ttlLineCount = 8);
    
    /** Destructor */
    ~SyncControlButton();

    /** Creates the sync selection interface */
    //void mouseUp(const MouseEvent &event) override;
    
    String streamKey;
    bool isPrimary;
    int ttlLineCount;

private:
    
    /** Checks whether the underlying stream is synchronized */
    void timerCallback();
    
    /** Renders the button */
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
    
    /** Called when popup selection interface is closed */
    //void componentBeingDeleted(Component &component);
    
    SynchronizingProcessor* node;

};

/**
    Creates a special popup editor for a TtlLineParameter

    Displays the currently active TTL line,
    and makes it possible to select them by clicking.

*/
class PLUGIN_API TtlLineParameterEditor : 
    public ParameterEditor,
    public Button::Listener,
    public SyncLineSelector::Listener
{
public:

    /** Constructor */
    TtlLineParameterEditor(Parameter* param,
                           Parameter* syncParam = nullptr,
                           int rowHeightPixels = 18,
                           int rowWidthPixels = 160);

    /** Destructor */
    virtual ~TtlLineParameterEditor() { }

    /** Displays the PopupChannelSelector*/
    void buttonClicked(Button* label) override;

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() override;

    /** Responds to changes in the SyncLineSelector */
    void selectedLineChanged(int selectedLine) override;

    /** Sets parameter's stream as primary */
    void primaryStreamChanged() override;

    /** Sets sub-component locations */
    virtual void resized() override;

private:
    std::unique_ptr<TextButton> textButton;
    std::unique_ptr<SyncControlButton> syncControlButton;
    Parameter* syncParam;
};


class PLUGIN_API PathParameterEditor : public ParameterEditor,
    public Button::Listener
{
public:

    /** Constructor */
    PathParameterEditor(Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 160);

    /** Destructor */
    virtual ~PathParameterEditor() { }

    /** Displays the PopupChannelSelector*/
    void buttonClicked(Button* label) override;

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() override;

    /** Sets sub-component locations */
    virtual void resized() override;

private:
    std::unique_ptr<TextButton> button;
};

class PLUGIN_API TimeParameterEditor : public ParameterEditor,
    public Button::Listener, public Timer
{
public:

    /** Constructor */
    TimeParameterEditor(Parameter* param, int rowHeightPixels = 18, int rowWidthPixels = 160);

    /** Destructor */
    virtual ~TimeParameterEditor() { }

    /** Displays the PopupChannelSelector*/
    void buttonClicked(Button* label) override;

    /** Must ensure that editor state matches underlying parameter */
    virtual void updateView() override;

    /** Sets sub-component locations */
    virtual void resized() override;

private:

    void timerCallback() override;

    std::unique_ptr<TextButton> button;
    std::unique_ptr<PopupTimeEditor> timeEditor;
};

#endif  // __PARAMETEREDITOR_H_44537DA9__