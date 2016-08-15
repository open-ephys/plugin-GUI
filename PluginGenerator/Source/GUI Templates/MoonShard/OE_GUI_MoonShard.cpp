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

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "OE_GUI_MoonShard.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
OE_GUI_MoonShard::OE_GUI_MoonShard ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    setLookAndFeel (defaultLookAndFeel);
    //[/Constructor_pre]

    addAndMakeVisible (textButton = new TextButton ("Example button"));
    textButton->setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight | Button::ConnectedOnTop | Button::ConnectedOnBottom);
    textButton->addListener (this);
    textButton->setColour (TextButton::buttonColourId, Colour (0xff2aa2e4));
    textButton->setColour (TextButton::buttonOnColourId, Colour (0xff005075));

    addAndMakeVisible (slider1 = new Slider ("new slider"));
    slider1->setRange (0, 10, 0);
    slider1->setSliderStyle (Slider::LinearHorizontal);
    slider1->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider1->addListener (this);

    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("Slider1\n")));
    label->setFont (Font (15.10f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (slider2 = new Slider ("new slider"));
    slider2->setRange (0, 10, 0);
    slider2->setSliderStyle (Slider::LinearHorizontal);
    slider2->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    slider2->addListener (this);

    addAndMakeVisible (label2 = new Label ("new label",
                                           TRANS("Slider2\n")));
    label2->setFont (Font (15.10f, Font::plain));
    label2->setJustificationType (Justification::centredLeft);
    label2->setEditable (false, false, false);
    label2->setColour (TextEditor::textColourId, Colours::black);
    label2->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (150, 140);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

OE_GUI_MoonShard::~OE_GUI_MoonShard()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    textButton = nullptr;
    slider1 = nullptr;
    label = nullptr;
    slider2 = nullptr;
    label2 = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void OE_GUI_MoonShard::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xffcfecf5));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void OE_GUI_MoonShard::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    textButton->setBounds (proportionOfWidth (0.1067f), proportionOfHeight (0.6286f), proportionOfWidth (0.8000f), proportionOfHeight (0.2857f));
    slider1->setBounds (proportionOfWidth (0.4667f), proportionOfHeight (0.1429f), proportionOfWidth (0.4667f), 24);
    label->setBounds (proportionOfWidth (0.0533f), proportionOfHeight (0.1143f), 56, 32);
    slider2->setBounds (proportionOfWidth (0.4667f), proportionOfHeight (0.3429f), proportionOfWidth (0.4667f), 24);
    label2->setBounds (proportionOfWidth (0.0533f), proportionOfHeight (0.3214f), 56, 32);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void OE_GUI_MoonShard::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == textButton)
    {
        //[UserButtonCode_textButton] -- add your button handler code here..
        //[/UserButtonCode_textButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void OE_GUI_MoonShard::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == slider1)
    {
        //[UserSliderCode_slider1] -- add your slider handling code here..
        //[/UserSliderCode_slider1]
    }
    else if (sliderThatWasMoved == slider2)
    {
        //[UserSliderCode_slider2] -- add your slider handling code here..
        //[/UserSliderCode_slider2]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="OE_GUI_MoonShard" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="150" initialHeight="140">
  <BACKGROUND backgroundColour="ffcfecf5"/>
  <TEXTBUTTON name="Example button" id="25ca35aa5a01f1e6" memberName="textButton"
              virtualName="" explicitFocusOrder="0" pos="10.667% 62.857% 80% 28.571%"
              bgColOff="ff2aa2e4" bgColOn="ff005075" buttonText="Example button"
              connectedEdges="15" needsCallback="1" radioGroupId="0"/>
  <SLIDER name="new slider" id="339671929170a5c2" memberName="slider1"
          virtualName="" explicitFocusOrder="0" pos="46.667% 14.286% 46.667% 24"
          min="0" max="10" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"
          needsCallback="1"/>
  <LABEL name="new label" id="c0213f1981531a0d" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="5.333% 11.429% 56 32" edTextCol="ff000000"
         edBkgCol="0" labelText="Slider1&#10;" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.099999999999999645" bold="0" italic="0" justification="33"/>
  <SLIDER name="new slider" id="791588b5e0859f71" memberName="slider2"
          virtualName="" explicitFocusOrder="0" pos="46.667% 34.286% 46.667% 24"
          min="0" max="10" int="0" style="LinearHorizontal" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"
          needsCallback="1"/>
  <LABEL name="new label" id="8675a1fd47da165e" memberName="label2" virtualName=""
         explicitFocusOrder="0" pos="5.333% 32.143% 56 32" edTextCol="ff000000"
         edBkgCol="0" labelText="Slider2&#10;" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.099999999999999645" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
