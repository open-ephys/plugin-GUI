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

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "EcubeDialogComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
EcubeDialogComponent::EcubeDialogComponent ()
{
    addAndMakeVisible (laAddressLabel = new Label ("Address Label",
                                                   TRANS("Network Address:")));
    laAddressLabel->setFont (Font (15.00f, Font::plain));
    laAddressLabel->setJustificationType (Justification::centredLeft);
    laAddressLabel->setEditable (false, false, false);
    laAddressLabel->setColour (TextEditor::textColourId, Colours::black);
    laAddressLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (bnOK = new TextButton ("OK button"));
    bnOK->setButtonText (TRANS("Connect"));
    bnOK->addListener (this);

    addAndMakeVisible (bnCancel = new TextButton ("Cancel Button"));
    bnCancel->setButtonText (TRANS("Cancel"));
    bnCancel->addListener (this);

    addAndMakeVisible (comboModule = new ComboBox ("Module selection combobox"));
    comboModule->setTooltip (TRANS("Choose eCube module"));
    comboModule->setEditableText (false);
    comboModule->setJustificationType (Justification::centredLeft);
    comboModule->setTextWhenNothingSelected (String::empty);
    comboModule->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    comboModule->addItem (TRANS("Headstage(s)"), 1);
    comboModule->addItem (TRANS("Panel Analog Input"), 2);
    comboModule->addItem (TRANS("Panel Digital Input"), 3);
    comboModule->addListener (this);

    addAndMakeVisible (labelModule = new Label ("Module selection label",
                                                TRANS("Module")));
    labelModule->setFont (Font (15.00f, Font::plain));
    labelModule->setJustificationType (Justification::centredLeft);
    labelModule->setEditable (false, false, false);
    labelModule->setColour (TextEditor::textColourId, Colours::black);
    labelModule->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (cbNetworkAddress = new ComboBox ("Network Address ComboBox"));
    cbNetworkAddress->setEditableText (true);
    cbNetworkAddress->setJustificationType (Justification::centredLeft);
    cbNetworkAddress->setTextWhenNothingSelected (String::empty);
    cbNetworkAddress->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    cbNetworkAddress->addListener (this);

    addAndMakeVisible (groupComponent = new GroupComponent ("headstage group",
                                                            TRANS("Headstage selection")));

    addAndMakeVisible (toggleHeadstage1 = new ToggleButton ("Headstage1 button"));
    toggleHeadstage1->setButtonText (TRANS("Headstage 1"));
    toggleHeadstage1->addListener (this);

    addAndMakeVisible (toggleHeadstage2 = new ToggleButton ("Headstage2 button"));
    toggleHeadstage2->setButtonText (TRANS("Headstage 2"));
    toggleHeadstage2->addListener (this);

    addAndMakeVisible (toggleHeadstage3 = new ToggleButton ("Headstage3 button"));
    toggleHeadstage3->setButtonText (TRANS("Headstage 3"));
    toggleHeadstage3->addListener (this);

    addAndMakeVisible (toggleHeadstage4 = new ToggleButton ("Headstage4 button"));
    toggleHeadstage4->setButtonText (TRANS("Headstage 4"));
    toggleHeadstage4->addListener (this);

    addAndMakeVisible (toggleHeadstage5 = new ToggleButton ("Headstage5 button"));
    toggleHeadstage5->setButtonText (TRANS("Headstage 5"));
    toggleHeadstage5->addListener (this);

    addAndMakeVisible (toggleHeadstage6 = new ToggleButton ("Headstage6 button"));
    toggleHeadstage6->setButtonText (TRANS("Headstage 6"));
    toggleHeadstage6->addListener (this);

    addAndMakeVisible (toggleHeadstage7 = new ToggleButton ("Headstage7 button"));
    toggleHeadstage7->setButtonText (TRANS("Headstage 7"));
    toggleHeadstage7->addListener (this);

    addAndMakeVisible (toggleHeadstage8 = new ToggleButton ("Headstage8 button"));
    toggleHeadstage8->setButtonText (TRANS("Headstage 8"));
    toggleHeadstage8->addListener (this);

    addAndMakeVisible (toggleHeadstage9 = new ToggleButton ("Headstage9 button"));
    toggleHeadstage9->setButtonText (TRANS("Headstage 9"));
    toggleHeadstage9->addListener (this);

    addAndMakeVisible (toggleHeadstage10 = new ToggleButton ("Headstage10 button"));
    toggleHeadstage10->setButtonText (TRANS("Headstage 10"));
    toggleHeadstage10->addListener (this);
    toggleHeadstage10->setToggleState (true, dontSendNotification);

    addAndMakeVisible (labelSamplerate = new Label ("Samplerate Label",
                                                    TRANS("Sample Rate")));
    labelSamplerate->setFont (Font (15.00f, Font::plain));
    labelSamplerate->setJustificationType (Justification::centredLeft);
    labelSamplerate->setEditable (false, false, false);
    labelSamplerate->setColour (TextEditor::textColourId, Colours::black);
    labelSamplerate->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (comboSamplerate = new ComboBox ("Samplerate Combobox"));
    comboSamplerate->setEditableText (true);
    comboSamplerate->setJustificationType (Justification::centredLeft);
    comboSamplerate->setTextWhenNothingSelected (String::empty);
    comboSamplerate->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    comboSamplerate->addItem (TRANS("20000"), 1);
    comboSamplerate->addItem (TRANS("25000"), 2);
    comboSamplerate->addItem (TRANS("32000"), 3);
    comboSamplerate->addItem (TRANS("40000"), 4);
    comboSamplerate->addItem (TRANS("50000"), 5);
    comboSamplerate->addItem (TRANS("62500"), 6);
    comboSamplerate->addItem (TRANS("64000"), 7);
    comboSamplerate->addListener (this);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (310, 360);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

EcubeDialogComponent::~EcubeDialogComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    laAddressLabel = nullptr;
    bnOK = nullptr;
    bnCancel = nullptr;
    comboModule = nullptr;
    labelModule = nullptr;
    cbNetworkAddress = nullptr;
    groupComponent = nullptr;
    toggleHeadstage1 = nullptr;
    toggleHeadstage2 = nullptr;
    toggleHeadstage3 = nullptr;
    toggleHeadstage4 = nullptr;
    toggleHeadstage5 = nullptr;
    toggleHeadstage6 = nullptr;
    toggleHeadstage7 = nullptr;
    toggleHeadstage8 = nullptr;
    toggleHeadstage9 = nullptr;
    toggleHeadstage10 = nullptr;
    labelSamplerate = nullptr;
    comboSamplerate = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void EcubeDialogComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void EcubeDialogComponent::resized()
{
    laAddressLabel->setBounds (16, 8, 150, 24);
    bnOK->setBounds (24, 328, 112, 24);
    bnCancel->setBounds (160, 328, 87, 24);
    comboModule->setBounds (80, 72, 192, 24);
    labelModule->setBounds (16, 72, 72, 24);
    cbNetworkAddress->setBounds (24, 40, 248, 24);
    groupComponent->setBounds (8, 152, 288, 160);
    toggleHeadstage1->setBounds (24, 176, 112, 24);
    toggleHeadstage2->setBounds (24, 200, 112, 24);
    toggleHeadstage3->setBounds (24, 224, 112, 24);
    toggleHeadstage4->setBounds (24, 248, 112, 24);
    toggleHeadstage5->setBounds (24, 272, 112, 24);
    toggleHeadstage6->setBounds (160, 176, 112, 24);
    toggleHeadstage7->setBounds (160, 200, 112, 24);
    toggleHeadstage8->setBounds (160, 224, 112, 24);
    toggleHeadstage9->setBounds (160, 248, 112, 24);
    toggleHeadstage10->setBounds (160, 272, 112, 24);
    labelSamplerate->setBounds (16, 112, 88, 24);
    comboSamplerate->setBounds (120, 112, 152, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void EcubeDialogComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == bnOK)
    {
        //[UserButtonCode_bnOK] -- add your button handler code here..
        DialogWindow* dw = findParentComponentOfClass<DialogWindow>();
        if (dw != nullptr)
        {
            dw->exitModalState(1);
        }
        //[/UserButtonCode_bnOK]
    }
    else if (buttonThatWasClicked == bnCancel)
    {
        //[UserButtonCode_bnCancel] -- add your button handler code here..
        DialogWindow* dw = findParentComponentOfClass<DialogWindow>();
        if (dw != nullptr)
        {
            dw->exitModalState(0);
        }
        //[/UserButtonCode_bnCancel]
    }
    else if (buttonThatWasClicked == toggleHeadstage1)
    {
        //[UserButtonCode_toggleHeadstage1] -- add your button handler code here..
        //[/UserButtonCode_toggleHeadstage1]
    }
    else if (buttonThatWasClicked == toggleHeadstage2)
    {
        //[UserButtonCode_toggleHeadstage2] -- add your button handler code here..
        //[/UserButtonCode_toggleHeadstage2]
    }
    else if (buttonThatWasClicked == toggleHeadstage3)
    {
        //[UserButtonCode_toggleHeadstage3] -- add your button handler code here..
        //[/UserButtonCode_toggleHeadstage3]
    }
    else if (buttonThatWasClicked == toggleHeadstage4)
    {
        //[UserButtonCode_toggleHeadstage4] -- add your button handler code here..
        //[/UserButtonCode_toggleHeadstage4]
    }
    else if (buttonThatWasClicked == toggleHeadstage5)
    {
        //[UserButtonCode_toggleHeadstage5] -- add your button handler code here..
        //[/UserButtonCode_toggleHeadstage5]
    }
    else if (buttonThatWasClicked == toggleHeadstage6)
    {
        //[UserButtonCode_toggleHeadstage6] -- add your button handler code here..
        //[/UserButtonCode_toggleHeadstage6]
    }
    else if (buttonThatWasClicked == toggleHeadstage7)
    {
        //[UserButtonCode_toggleHeadstage7] -- add your button handler code here..
        //[/UserButtonCode_toggleHeadstage7]
    }
    else if (buttonThatWasClicked == toggleHeadstage8)
    {
        //[UserButtonCode_toggleHeadstage8] -- add your button handler code here..
        //[/UserButtonCode_toggleHeadstage8]
    }
    else if (buttonThatWasClicked == toggleHeadstage9)
    {
        //[UserButtonCode_toggleHeadstage9] -- add your button handler code here..
        //[/UserButtonCode_toggleHeadstage9]
    }
    else if (buttonThatWasClicked == toggleHeadstage10)
    {
        //[UserButtonCode_toggleHeadstage10] -- add your button handler code here..
        //[/UserButtonCode_toggleHeadstage10]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void EcubeDialogComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == comboModule)
    {
        //[UserComboBoxCode_comboModule] -- add your combo box handling code here..
        if (comboModule->getSelectedId() != 2)
        {
            comboSamplerate->setSelectedId(2); // Force 25k sample rate for non-PAI sources
            comboSamplerate->setEnabled(false); // Also disable the samplerate combo box
        }
        else
        {
            comboSamplerate->setEnabled(true);
        }
        //[/UserComboBoxCode_comboModule]
    }
    else if (comboBoxThatHasChanged == cbNetworkAddress)
    {
        //[UserComboBoxCode_cbNetworkAddress] -- add your combo box handling code here..
        //[/UserComboBoxCode_cbNetworkAddress]
    }
    else if (comboBoxThatHasChanged == comboSamplerate)
    {
        //[UserComboBoxCode_comboSamplerate] -- add your combo box handling code here..
        //[/UserComboBoxCode_comboSamplerate]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
String EcubeDialogComponent::GetAddressValue()
{
    return cbNetworkAddress->getText();
}

String EcubeDialogComponent::GetModuleName()
{
    return comboModule->getText();
}

void EcubeDialogComponent::SetDeviceNames(const StringArray& names)
{
    for (int i = 0; i < names.size(); i++)
    {
        cbNetworkAddress->addItem(names[i], i+1);
    }
}

void EcubeDialogComponent::GetHeadstageSelection(bool hs[10])
{
    for (int i = 0; i < 10; i++)
        hs[i] = false;
    if (toggleHeadstage1->getToggleState())
        hs[0] = true;
    if (toggleHeadstage2->getToggleState())
        hs[1] = true;
    if (toggleHeadstage3->getToggleState())
        hs[2] = true;
    if (toggleHeadstage4->getToggleState())
        hs[3] = true;
    if (toggleHeadstage5->getToggleState())
        hs[4] = true;
    if (toggleHeadstage6->getToggleState())
        hs[5] = true;
    if (toggleHeadstage7->getToggleState())
        hs[6] = true;
    if (toggleHeadstage8->getToggleState())
        hs[7] = true;
    if (toggleHeadstage9->getToggleState())
        hs[8] = true;
    if (toggleHeadstage10->getToggleState())
        hs[9] = true;
}

double EcubeDialogComponent::GetSampleRate(void)
{
    return comboSamplerate->getText().getDoubleValue();

}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="EcubeDialogComponent" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="310" initialHeight="360">
  <BACKGROUND backgroundColour="ffffffff"/>
  <LABEL name="Address Label" id="598e4f972ee19e11" memberName="laAddressLabel"
         virtualName="" explicitFocusOrder="0" pos="16 8 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Network Address:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTBUTTON name="OK button" id="d46371ace0e2731f" memberName="bnOK" virtualName=""
              explicitFocusOrder="0" pos="24 328 112 24" buttonText="Connect"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="Cancel Button" id="e57f56c355a79a42" memberName="bnCancel"
              virtualName="" explicitFocusOrder="0" pos="160 328 87 24" buttonText="Cancel"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <COMBOBOX name="Module selection combobox" id="937bd3c2684c1901" memberName="comboModule"
            virtualName="" explicitFocusOrder="0" pos="80 72 192 24" tooltip="Choose eCube module"
            editable="0" layout="33" items="Headstage(s)&#10;Panel Analog Input&#10;Panel Digital Input"
            textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <LABEL name="Module selection label" id="8db8f9471da84061" memberName="labelModule"
         virtualName="" explicitFocusOrder="0" pos="16 72 72 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Module" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <COMBOBOX name="Network Address ComboBox" id="40e24a76fea3653c" memberName="cbNetworkAddress"
            virtualName="" explicitFocusOrder="0" pos="24 40 248 24" editable="1"
            layout="33" items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <GROUPCOMPONENT name="headstage group" id="8dee2bbe073f0f63" memberName="groupComponent"
                  virtualName="" explicitFocusOrder="0" pos="8 152 288 160" title="Headstage selection"/>
  <TOGGLEBUTTON name="Headstage1 button" id="c735dad1514586" memberName="toggleHeadstage1"
                virtualName="" explicitFocusOrder="0" pos="24 176 112 24" buttonText="Headstage 1"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="Headstage2 button" id="24f215ce96874781" memberName="toggleHeadstage2"
                virtualName="" explicitFocusOrder="0" pos="24 200 112 24" buttonText="Headstage 2"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="Headstage3 button" id="f035e57b5cc87d27" memberName="toggleHeadstage3"
                virtualName="" explicitFocusOrder="0" pos="24 224 112 24" buttonText="Headstage 3"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="Headstage4 button" id="6abc75f398861c5f" memberName="toggleHeadstage4"
                virtualName="" explicitFocusOrder="0" pos="24 248 112 24" buttonText="Headstage 4"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="Headstage5 button" id="3c5bb9c9db62f1bb" memberName="toggleHeadstage5"
                virtualName="" explicitFocusOrder="0" pos="24 272 112 24" buttonText="Headstage 5"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="Headstage6 button" id="306515c3b3ea7522" memberName="toggleHeadstage6"
                virtualName="" explicitFocusOrder="0" pos="160 176 112 24" buttonText="Headstage 6"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="Headstage7 button" id="704c88cd419204b0" memberName="toggleHeadstage7"
                virtualName="" explicitFocusOrder="0" pos="160 200 112 24" buttonText="Headstage 7"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="Headstage8 button" id="ba85df2f98d869d9" memberName="toggleHeadstage8"
                virtualName="" explicitFocusOrder="0" pos="160 224 112 24" buttonText="Headstage 8"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="Headstage9 button" id="16256502b9f60cd4" memberName="toggleHeadstage9"
                virtualName="" explicitFocusOrder="0" pos="160 248 112 24" buttonText="Headstage 9"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="Headstage10 button" id="fab4cbe0a6d27a36" memberName="toggleHeadstage10"
                virtualName="" explicitFocusOrder="0" pos="160 272 112 24" buttonText="Headstage 10"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="1"/>
  <LABEL name="Samplerate Label" id="35c4f52cb7d6144f" memberName="labelSamplerate"
         virtualName="" explicitFocusOrder="0" pos="16 112 88 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Sample Rate" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <COMBOBOX name="Samplerate Combobox" id="e855d5ea6005570b" memberName="comboSamplerate"
            virtualName="" explicitFocusOrder="0" pos="120 112 152 24" editable="1"
            layout="33" items="20000&#10;25000&#10;32000&#10;40000&#10;50000&#10;62500&#10;64000"
            textWhenNonSelected="" textWhenNoItems="(no choices)"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
