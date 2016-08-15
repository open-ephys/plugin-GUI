/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 4.2.1

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_3B5F4A5CE75A04FC__
#define __JUCE_HEADER_3B5F4A5CE75A04FC__

//[Headers]     -- You can add your own extra header files here --
#include <JuceHeader.h>
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class OE_GUI_MoonShard  : public Component,
                          public ButtonListener,
                          public SliderListener
{
public:
    //==============================================================================
    OE_GUI_MoonShard ();
    ~OE_GUI_MoonShard();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    SharedResourcePointer<LookAndFeel_V2> defaultLookAndFeel;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<TextButton> textButton;
    ScopedPointer<Slider> slider1;
    ScopedPointer<Label> label;
    ScopedPointer<Slider> slider2;
    ScopedPointer<Label> label2;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OE_GUI_MoonShard)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_3B5F4A5CE75A04FC__
