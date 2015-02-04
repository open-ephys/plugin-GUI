/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_CDBC0626D3EB7016__
#define __JUCE_HEADER_CDBC0626D3EB7016__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class EcubeDialogComponent  : public Component,
                              public ButtonListener,
                              public ComboBoxListener
{
public:
    //==============================================================================
    EcubeDialogComponent ();
    ~EcubeDialogComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    String GetAddressValue();
    String GetModuleName();
    void SetDeviceNames(const StringArray& names);
    void GetHeadstageSelection(bool hs[10]);
    double GetSampleRate(void);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> laAddressLabel;
    ScopedPointer<TextButton> bnOK;
    ScopedPointer<TextButton> bnCancel;
    ScopedPointer<ComboBox> comboModule;
    ScopedPointer<Label> labelModule;
    ScopedPointer<ComboBox> cbNetworkAddress;
    ScopedPointer<GroupComponent> groupComponent;
    ScopedPointer<ToggleButton> toggleHeadstage1;
    ScopedPointer<ToggleButton> toggleHeadstage2;
    ScopedPointer<ToggleButton> toggleHeadstage3;
    ScopedPointer<ToggleButton> toggleHeadstage4;
    ScopedPointer<ToggleButton> toggleHeadstage5;
    ScopedPointer<ToggleButton> toggleHeadstage6;
    ScopedPointer<ToggleButton> toggleHeadstage7;
    ScopedPointer<ToggleButton> toggleHeadstage8;
    ScopedPointer<ToggleButton> toggleHeadstage9;
    ScopedPointer<ToggleButton> toggleHeadstage10;
    ScopedPointer<Label> labelSamplerate;
    ScopedPointer<ComboBox> comboSamplerate;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EcubeDialogComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_CDBC0626D3EB7016__
