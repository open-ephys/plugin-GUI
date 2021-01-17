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

#include "ChannelSelector.h"
#include <math.h>

#include "../../AccessClass.h"
#include "../RecordNode/RecordNode.h"
#include "../AudioNode/AudioNode.h"
#include "../ProcessorGraph/ProcessorGraph.h"
#include "../../UI/GraphViewer.h"
#include "../../Utils/ListSliceParser.h"
#include "../../Utils/Utils.h"


static const Colour COLOUR_DROPDOWN_BUTTON_BG   (Colour::fromRGB (48, 63, 159));

static const Font FONT_DEFAULT ("Arial", 12.f, Font::plain);

static const int SIZE_DROPDOWN_ARROW            = 16;
static const int DURATION_ANIMATION_COLLAPSE_MS = 200;


ChannelSelector::ChannelSelector(bool createButtons, Font& titleFont_) :
    eventsOnly(false)
    , parameterSlicerChannelSelector (Channels::PARAM_CHANNELS,  "Parameter slicer channel selector component")
    , audioSlicerChannelSelector     (Channels::AUDIO_CHANNELS,  "Audio slicer channel selector component")
    , recordSlicerChannelSelector    (Channels::RECORD_CHANNELS, "Record slicer channel selector component")
    , paramsToggled(true), paramsActive(true), recActive(true), radioStatus(false), isNotSink(createButtons)
    , moveRight(false), moveLeft(false), offsetLR(0), offsetUD(0), desiredOffset(0), titleFont(titleFont_), acquisitionIsActive(false)
{
    audioButton = new EditorButton("AUDIO", titleFont);
    audioButton->addListener(this);
    addAndMakeVisible(audioButton);
    if (!createButtons)
        audioButton->setState(false);

    /*
    recordButton = new EditorButton("REC", titleFont);
    recordButton->addListener(this);
    addAndMakeVisible(recordButton);
    if (!createButtons)
        recordButton->setState(false);
    */

    paramsButton = new EditorButton("PARAM", titleFont);
    paramsButton->addListener(this);
    addAndMakeVisible(paramsButton);

    paramsButton->setToggleState(true, dontSendNotification);

    // set button layout parameters
    parameterOffset = 0;
    recordOffset = getDesiredWidth();
    audioOffset = getDesiredWidth() * 2;

    allButton = new EditorButton("all", titleFont);
    allButton->addListener(this);
    addAndMakeVisible(allButton);

    noneButton = new EditorButton("none", titleFont);
    noneButton->addListener(this);
    addAndMakeVisible(noneButton);

    // Buttons managers
    // ====================================================================
    addAndMakeVisible (audioButtonsManager);
    //addAndMakeVisible (recordButtonsManager);
    addAndMakeVisible (parameterButtonsManager);

    // Enable fast mode selection for buttons
    audioButtonsManager.setFastSelectionModeEnabled     (true);
    recordButtonsManager.setFastSelectionModeEnabled    (true);
    parameterButtonsManager.setFastSelectionModeEnabled (true);

    audioButtonsManager.setMinPaddingBetweenButtons     (0);
    recordButtonsManager.setMinPaddingBetweenButtons    (0);
    parameterButtonsManager.setMinPaddingBetweenButtons (0);

    audioButtonsManager.setColour       (ButtonGroupManager::outlineColourId, Colour (0x0));
    recordButtonsManager.setColour      (ButtonGroupManager::outlineColourId, Colour (0x0));
    parameterButtonsManager.setColour   (ButtonGroupManager::outlineColourId, Colour (0x0));

    // Register listeners for buttons
    audioButtonsManager.setButtonListener      (this);
    recordButtonsManager.setButtonListener     (this);
    parameterButtonsManager.setButtonListener  (this);
    // ====================================================================

    // Slicer channels selectors
    // ====================================================================
    audioSlicerChannelSelector.setListener      (this);
    recordSlicerChannelSelector.setListener     (this);
    parameterSlicerChannelSelector.setListener  (this);

    // Set just initial y for each slicer
    const int slicerChannelSelectorY = 10;
    audioSlicerChannelSelector.setBounds        (audioSlicerChannelSelector.getBounds().withY (slicerChannelSelectorY));
    recordSlicerChannelSelector.setBounds       (recordSlicerChannelSelector.getBounds().withY (slicerChannelSelectorY));
    parameterSlicerChannelSelector.setBounds    (parameterSlicerChannelSelector.getBounds().withY (slicerChannelSelectorY));

    addAndMakeVisible (audioSlicerChannelSelector);
    addAndMakeVisible (recordSlicerChannelSelector);
    addAndMakeVisible (parameterSlicerChannelSelector);

    audioSlicerChannelSelector.toBack();
    recordSlicerChannelSelector.toBack();
    parameterSlicerChannelSelector.toBack();
    // ====================================================================


    numColumnsLessThan100 = 8;
    numColumnsGreaterThan100 = 6;
}

ChannelSelector::~ChannelSelector()
{
    // Just a temporary workaround as we don't want to delete these buttons managers by hands.
    // We will remove it after getting rid of the ugly calling of deleteAllChildren() method.
    // We should really use some RAII technuiqes to avoid calling this method.
    // TODO: refactor the code to follow RAII best principles and to avoid using raw pointers after merge with priyanjitdey94
    removeChildComponent (&audioButtonsManager);
    removeChildComponent (&recordButtonsManager);
    removeChildComponent (&parameterButtonsManager);

    removeChildComponent (&audioSlicerChannelSelector);
    removeChildComponent (&recordSlicerChannelSelector);
    removeChildComponent (&parameterSlicerChannelSelector);

    deleteAllChildren();
}

void ChannelSelector::paint(Graphics& g)
{
    ColourGradient grad1 = ColourGradient(Colours::black.withAlpha(0.8f), 0.0, 0.0,
                                          Colours::black.withAlpha(0.1f), 0.0, 25.0f,
                                          false);
    g.setGradientFill(grad1);
    g.fillRect(0, 15, getWidth(), getHeight() - 30);

    ColourGradient grad2 = ColourGradient(Colours::black.withAlpha(0.2f), 0.0, 0.0,
                                          Colours::black.withAlpha(0.0f), 5.0f, 0.0f,
                                          false);
    g.setGradientFill(grad2);
    g.fillRect(0, 15, getWidth(), getHeight() - 30);

    ColourGradient grad3 = ColourGradient(Colours::black.withAlpha(0.2f), (float)getDesiredWidth(), 0.0,
                                          Colours::black.withAlpha(0.0f), (float)getDesiredWidth() - 5.0f, 0.0f,
                                          false);
    g.setGradientFill(grad3);
    g.fillRect(0, 15, getWidth(), getHeight() - 30);
}


void ChannelSelector::setNumChannels(int numChans)
{
    int difference = numChans - parameterButtonsManager.getNumButtons();

LOGDD(difference, " buttons needed.");

    if (difference > 0)
    {
        for (int n = 0; n < difference; n++)
        {
            addButton();
        }
    }
    else if (difference < 0)
    {
        for (int n = 0; n < -difference; n++)
        {
            removeButton();
        }
    }

    const int numButtons = parameterButtonsManager.getNumButtons();
    //Reassign numbers according to the actual channels (useful for channel mapper)
    for (int n = 0; n < numButtons; ++n)
    {
        int num = ( (GenericEditor*)getParentComponent())->getChannelDisplayNumber (n);
        static_cast<ChannelSelectorButton*> (parameterButtonsManager.getButtonAt  (n))->setChannel (n + 1, num + 1);

        if (isNotSink)
        {
            static_cast<ChannelSelectorButton*> (recordButtonsManager.getButtonAt (n))->setChannel (n + 1, num + 1);
            static_cast<ChannelSelectorButton*> (audioButtonsManager.getButtonAt  (n))->setChannel (n + 1, num + 1);
        }
    }

    refreshButtonBoundaries();
}

int ChannelSelector::getNumChannels()
{
    return parameterButtonsManager.getNumButtons();
}

void ChannelSelector::shiftChannelsVertical(float amount)
{
    if (parameterButtonsManager.getNumButtons() > 16)
    {
        offsetUD -= amount * 10;
        offsetUD = jmin(offsetUD, 0.0f);
        offsetUD = jmax(offsetUD, (float)-overallHeight);
    }

LOGDD("offsetUD = ", offsetUD);

    refreshButtonBoundaries();
}

void ChannelSelector::refreshButtonBoundaries()
{
    const int columnWidth   = getDesiredWidth() / (numColumnsGreaterThan100 + 1) + 1;
    const int rowHeight     = 14;

    audioButtonsManager.setButtonSize      (columnWidth, rowHeight);
    recordButtonsManager.setButtonSize     (columnWidth, rowHeight);
    parameterButtonsManager.setButtonSize  (columnWidth, rowHeight);

    const int xLoc = offsetLR + 3;

    juce::Rectangle<int> slicerSelectorBounds (xLoc - 2, 0, getDesiredWidth(), 0);
    parameterSlicerChannelSelector.setBounds (slicerSelectorBounds
                                              .withY (parameterSlicerChannelSelector.getY())
                                              .withHeight (parameterSlicerChannelSelector.getHeight()));
    slicerSelectorBounds.translate (- getDesiredWidth(), 0);
    recordSlicerChannelSelector.setBounds (slicerSelectorBounds
                                           .withY (recordSlicerChannelSelector.getY())
                                           .withHeight (recordSlicerChannelSelector.getHeight()));
    slicerSelectorBounds.translate (- getDesiredWidth(), 0);
    audioSlicerChannelSelector.setBounds (slicerSelectorBounds
                                          .withY (audioSlicerChannelSelector.getY())
                                          .withHeight (audioSlicerChannelSelector.getHeight()));

    // Set bounds for buttons managers
    // ===================================================================================================
    const int headerHeight              = 25;
    const int tabButtonHeight           = 15;
    const int buttonsManagerWidth       = getDesiredWidth() - 6;
    const int defaultButtonsManagerY    = headerHeight;

    // We will use just some hacks to set initial y and height if height is zero,
    // otherwise we will use the same bounds for buttons maangers
    int buttonsManagerX = xLoc;
    parameterButtonsManager.setBounds   (buttonsManagerX,
                                         parameterButtonsManager.getHeight() == 0 ? defaultButtonsManagerY : parameterButtonsManager.getY(),
                                         buttonsManagerWidth,
                                         getHeight() - parameterButtonsManager.getY() - tabButtonHeight);
    buttonsManagerX -= getDesiredWidth();
    recordButtonsManager.setBounds      (buttonsManagerX,
                                         recordButtonsManager.getHeight() == 0 ? defaultButtonsManagerY : recordButtonsManager.getY(),
                                         buttonsManagerWidth,
                                         getHeight() - recordButtonsManager.getY() - tabButtonHeight);
    buttonsManagerX -= getDesiredWidth();
    audioButtonsManager.setBounds       (buttonsManagerX,
                                         audioButtonsManager.getHeight() == 0 ? defaultButtonsManagerY : audioButtonsManager.getY(),
                                         buttonsManagerWidth,
                                         getHeight() - audioButtonsManager.getY() - tabButtonHeight);
    // ===================================================================================================

    /*
      audio,record and param tabs
    */
    //const int tabButtonWidth = getWidth() / 3;
    const int tabButtonWidth = getWidth() / 2;

    audioButton->setBounds  (0, 0, tabButtonWidth, tabButtonHeight);
    //recordButton->setBounds (tabButtonWidth, 0, tabButtonWidth, tabButtonHeight);
    paramsButton->setBounds (tabButtonWidth, 0, tabButtonWidth, tabButtonHeight);

    /*
      All and None buttons
    */
    allButton->setBounds (0, getHeight() - 15, getWidth() / 2, tabButtonHeight);
    noneButton->setBounds (getWidth() / 2, getHeight() - 15, getWidth() / 2, tabButtonHeight);
}

void ChannelSelector::resized()
{
    refreshButtonBoundaries();
}

void ChannelSelector::timerCallback()
{
LOGDD(desiredOffset - offsetLR);

    if (offsetLR != desiredOffset)
    {
        if (desiredOffset - offsetLR > 0)
        {
            offsetLR += 25; // be careful what you set this value to!
            // if it's not a multiple of the desired
            // width, things could go badly!
        }
        else
        {
            offsetLR -= 25;
        }
    }
    else
    {
        stopTimer();
    }

    refreshButtonBoundaries();
}

void ChannelSelector::addButton()
{
    const int size = parameterButtonsManager.getNumButtons();

    ChannelSelectorButton* b = new ChannelSelectorButton (size + 1, PARAMETER, titleFont);
    parameterButtonsManager.addButton (b);

    if (paramsToggled)
        b->setToggleState(true, dontSendNotification);
    else
        b->setToggleState(false, dontSendNotification);

    if (!paramsActive)
        b->setActive(false);

    if (isNotSink)
    {
        ChannelSelectorButton* br = new ChannelSelectorButton(size + 1, RECORD, titleFont);
        recordButtonsManager.addButton (br);

        ChannelSelectorButton* ba = new ChannelSelectorButton(size + 1, AUDIO, titleFont);
        audioButtonsManager.addButton (ba);
    }
}

void ChannelSelector::removeButton()
{
    int size = parameterButtonsManager.getNumButtons();

    parameterButtonsManager.removeButton (size - 1);

    if (isNotSink)
    {
        recordButtonsManager.removeButton (size - 1);
        audioButtonsManager.removeButton  (size - 1);
    }
}

Array<int> ChannelSelector::getActiveChannels()
{
    Array<int> a;

    if (! eventsOnly)
    {
        const int numButtons = parameterButtonsManager.getNumButtons();
        for (int i = 0; i < numButtons; ++i)
        {
            if (parameterButtonsManager.getButtonAt (i)->getToggleState())
                a.add (i);
        }
    }
    else
    {
        a.add (0);
    }

    return a;
}

void ChannelSelector::setActiveChannels(Array<int> a)
{
LOGDD("Setting active channels!");

    const int numButtons = parameterButtonsManager.getNumButtons();
    for (int i = 0; i < numButtons; ++i)
    {
        parameterButtonsManager.getButtonAt (i)->setToggleState (false, dontSendNotification);
    }

    for (int i = 0; i < a.size(); i++)
    {
        if (a[i] < numButtons)
        {
            parameterButtonsManager.getButtonAt (a[i])->setToggleState (true, dontSendNotification);
        }
    }
}

void ChannelSelector::inactivateButtons()
{
    paramsActive = false;

    const int numButtons = parameterButtonsManager.getNumButtons();
    for (int i = 0; i < numButtons; ++i)
    {
        const auto& button = static_cast<ChannelSelectorButton*> (parameterButtonsManager.getButtonAt (i));
        button->setActive (false);
        button->repaint();
    }
}

void ChannelSelector::activateButtons()
{
    paramsActive = true;

    const int numButtons = parameterButtonsManager.getNumButtons();
    for (int i = 0; i < numButtons; ++i)
    {
        const auto& button = static_cast<ChannelSelectorButton*> (parameterButtonsManager.getButtonAt (i));
        button->setActive (true);
        button->repaint();
    }
}

void ChannelSelector::inactivateRecButtons()
{
    recActive = false;

    const int numButtons = recordButtonsManager.getNumButtons();
    for (int i = 0; i < numButtons; ++i)
    {
        const auto& button = static_cast<ChannelSelectorButton*> (recordButtonsManager.getButtonAt (i));
        button->setActive (false);
        button->repaint();
    }
}

void ChannelSelector::activateRecButtons()
{
    recActive = true;

    const int numButtons = recordButtonsManager.getNumButtons();
    for (int i = 0; i < numButtons; ++i)
    {
        const auto& button = static_cast<ChannelSelectorButton*> (recordButtonsManager.getButtonAt (i));
        button->setActive (true);
        button->repaint();
    }
}

void ChannelSelector::refreshParameterColors()
{
    GenericEditor* p = dynamic_cast<GenericEditor*>(getParentComponent());
    p->updateParameterButtons(-1);
}

void ChannelSelector::paramButtonsToggledByDefault(bool t)
{
    paramsToggled = t;
}

void ChannelSelector::startAcquisition()
{
    acquisitionIsActive = true;
}

void ChannelSelector::stopAcquisition()
{
    acquisitionIsActive = false;
}

void ChannelSelector::setRadioStatus(bool radioOn)
{
    if (radioStatus != radioOn)
    {
        radioStatus = radioOn;

        const int numButtons = parameterButtonsManager.getNumButtons();
        for (int i = 0; i < numButtons; ++i)
        {
            parameterButtonsManager.getButtonAt (i)->setToggleState (false, dontSendNotification);
        }

        parameterButtonsManager.setRadioButtonMode (radioStatus);
    }
}

bool ChannelSelector::getParamStatus(int chan)
{
    if (chan >= 0 && chan < parameterButtonsManager.getNumButtons())
        return parameterButtonsManager.getButtonAt (chan)->getToggleState();
    else
        return false;
}

bool ChannelSelector::getRecordStatus(int chan)
{
    if (chan >= 0 && chan < recordButtonsManager.getNumButtons())
        return recordButtonsManager.getButtonAt (chan)->getToggleState();
    else
        return false;
}

bool ChannelSelector::getAudioStatus(int chan)
{
    if (chan >= 0 && chan < audioButtonsManager.getNumButtons())
        return audioButtonsManager.getButtonAt (chan)->getToggleState();
    else
        return false;
}

void ChannelSelector::setParamStatus(int chan, bool b)
{
    if (chan >= 0 && chan < parameterButtonsManager.getNumButtons())
        parameterButtonsManager.getButtonAt (chan)->setToggleState(b, sendNotification);
}

void ChannelSelector::setRecordStatus(int chan, bool b)
{
    if (chan >= 0 && chan < recordButtonsManager.getNumButtons())
        recordButtonsManager.getButtonAt (chan)->setToggleState(b, sendNotification);
}

void ChannelSelector::setAudioStatus(int chan, bool b)
{
    if (chan >= 0 && chan < audioButtonsManager.getNumButtons())
        audioButtonsManager.getButtonAt (chan)->setToggleState (b, sendNotification);
}

void ChannelSelector::clearAudio()
{
    const int numButtons = audioButtonsManager.getNumButtons();
    for (int chan = 0; chan < numButtons; ++chan)
        audioButtonsManager.getButtonAt (chan)->setToggleState (false, sendNotification);
}

int ChannelSelector::getDesiredWidth()
{
    return 150;
}

void ChannelSelector::buttonClicked(Button* button)
{
    //checkChannelSelectors();
    if (button == paramsButton)
    {
        // make sure param buttons are visible
        allButton->setState(true);
        desiredOffset = parameterOffset;
        startTimer(20);
        return;
    }
    else if (button == audioButton)
    {
        // make sure audio buttons are visible

        if (audioButton->getState())
        {
            allButton->setState(false);

            desiredOffset = audioOffset;
            startTimer(20);
        }
        else
        {
            paramsButton->setToggleState(true, dontSendNotification);
        }
        return;
    }
    else if (button == recordButton)
    {
        // make sure record buttons are visible;
        if (recordButton->getState())
        {
            allButton->setState(true);
            desiredOffset = recordOffset;
            startTimer(20);
        }
        else
        {
            paramsButton->setToggleState(true, dontSendNotification);
        }
        return;
    }
    else if (button == allButton)
    {
        // select all active buttons
        if (offsetLR == recordOffset)
        {
            for (int i = 0; i < recordButtonsManager.getNumButtons(); ++i)
            {
                recordButtonsManager.getButtonAt (i)->setToggleState (true, sendNotification);
            }

        }
        else if (offsetLR == parameterOffset)
        {
            for (int i = 0; i < parameterButtonsManager.getNumButtons(); ++i)
            {
                parameterButtonsManager.getButtonAt (i)->setToggleState (true, sendNotification);
            }
        }
        else if (offsetLR == audioOffset)
        {
            // do nothing--> button is disabled
        }
    }
    else if (button == noneButton)
    {
        // deselect all active buttons
        if (offsetLR == recordOffset)
        {
            for (int i = 0; i < recordButtonsManager.getNumButtons(); ++i)
            {
                recordButtonsManager.getButtonAt (i)->setToggleState (false, sendNotification);
            }
        }
        else if (offsetLR == parameterOffset)
        {
            for (int i = 0; i < parameterButtonsManager.getNumButtons(); ++i)
            {
                parameterButtonsManager.getButtonAt (i)->setToggleState (false, sendNotification);
            }
            
            if (radioStatus) // if radio buttons are active
            {
                // send a message to parent
                GenericEditor* editor = (GenericEditor*) getParentComponent();
                editor->channelChanged (-1, false);
            }
        }
        else if (offsetLR == audioOffset)
        {
            for (int i = 0; i < audioButtonsManager.getNumButtons(); ++i)
            {
                audioButtonsManager.getButtonAt (i)->setToggleState (false, sendNotification);
            }
        }
    }
    else
    {
        ChannelSelectorButton* b = (ChannelSelectorButton*)button;

        if (b->getType() == AUDIO)
        {
            // get audio node, and inform it of the change
            GenericEditor* editor = (GenericEditor*)getParentComponent();

            const DataChannel* ch = editor->getChannel(b->getChannel() - 1);
            //int channelNum = editor->getStartChannel() + b->getChannel() - 1;
            bool status = b->getToggleState();

//LOGDD("Requesting audio monitor for channel ", ch->nodeIndex + 1);
            
            // change parameter directly on editor
            //     This is another of those ugly things that will go away once the
            //     probe audio system is implemented, but is needed to maintain compatibility
            //     between the older recording system and the newer channel objects.
            const_cast<DataChannel*>(ch)->setMonitored(status);

            
            if (acquisitionIsActive) // use setParameter to change audio node's copy of parameter safely, if running
            {
                AccessClass::getProcessorGraph()->
                getAudioNode()->setChannelStatus(ch, status);
            }
        }
        else if (b->getType() == RECORD)
        {
			
            // get record node, and inform it of the change
            GenericEditor* editor = (GenericEditor*)getParentComponent();

            const DataChannel* ch = editor->getChannel(b->getChannel() - 1);
            //int channelNum = editor->getStartChannel() + b->getChannel() - 1;
            bool status = b->getToggleState();

            if (acquisitionIsActive) // use setParameter to change parameter safely
            {
                               
                // disable toggling when acquisition is active
                b->setToggleState(const_cast<DataChannel*>(ch)->getRecordState(), dontSendNotification);
            }
            else     // change parameter directly
            {
LOGDD("Setting record status for channel ", b->getChannel());

				//This is another of those ugly things that will go away once the
				//probe recording system is implemented, but is needed to maintain compatibility
				//between the older recording system and the newer channel objects.
                const_cast<DataChannel*>(ch)->setRecordState(status);
            }

            AccessClass::getGraphViewer()->repaint();

        }
        else // parameter type
        {
            GenericEditor* editor = (GenericEditor*) getParentComponent();
            editor->channelChanged (b->getChannel() - 1, b->getToggleState());

            // do nothing
            if (radioStatus) // if radio buttons are active
            {
                // send a message to parent
                GenericEditor* editor = (GenericEditor*) getParentComponent();
                editor->channelChanged (b->getChannel(), b->getToggleState());
            }
        }

    }
    refreshParameterColors();
}


void ChannelSelector::changeChannelsSelectionButtonClicked (SlicerChannelSelectorComponent* sender,
                                                            Button* buttonThatWasClicked,
                                                            bool isSelect)
{
    const Channels::ChannelsType channelsType = sender->getChannelsType();

    TiledButtonGroupManager* buttonsManager = nullptr;
    if (channelsType == Channels::AUDIO_CHANNELS)
        buttonsManager = &audioButtonsManager;
    else if (channelsType == Channels::RECORD_CHANNELS)
        buttonsManager = &recordButtonsManager;
    else if (channelsType == Channels::PARAM_CHANNELS)
        buttonsManager = &parameterButtonsManager;

    jassert (buttonsManager != nullptr);

    Array<int> getBoxList = ListSliceParser::parseStringIntoRange (sender->getText(), audioButtonsManager.getNumButtons());
    if (getBoxList.size() < 3)
        return;

    int i = 0;
    while (i <= getBoxList.size() - 3)
    {
        const int lim = getBoxList[i + 1];
        const int comd = getBoxList[i + 2];
        for (int fa = getBoxList[i]; fa <= lim; fa += comd)
        {
            buttonsManager->getButtonAt (fa)->setToggleState (isSelect, sendNotification);
        }
        i += 3;
    }
}


void ChannelSelector::channelSelectorCollapsedStateChanged (SlicerChannelSelectorComponent* sender,
                                                            bool isCollapsed)
{
    const Channels::ChannelsType channelsType = sender->getChannelsType();

    TiledButtonGroupManager* buttonsManager = nullptr;
    if (channelsType == Channels::AUDIO_CHANNELS)
        buttonsManager = &audioButtonsManager;
    else if (channelsType == Channels::RECORD_CHANNELS)
        buttonsManager = &recordButtonsManager;
    else if (channelsType == Channels::PARAM_CHANNELS)
        buttonsManager = &parameterButtonsManager;

    jassert (buttonsManager != nullptr);

    const int headerHeight      = 25;
    const int tabButtonHeight   = 15;

    int yPos = headerHeight;
    if (! isCollapsed)
        yPos += SlicerChannelSelectorComponent::MAX_HEIGHT - 20;

    const int height = getHeight() - yPos - tabButtonHeight;
    const juce::Rectangle<int> finalBounds (buttonsManager->getX(), yPos, buttonsManager->getWidth(), height);

    auto& componentAnimator = Desktop::getInstance().getAnimator();
    componentAnimator.animateComponent (buttonsManager, finalBounds, 1.f, DURATION_ANIMATION_COLLAPSE_MS, false, 1.0, 1.0);
}

///////////// BUTTONS //////////////////////


EditorButton::EditorButton(const String& name, const Font& f) : Button(name)
{
    isEnabled = true;

    buttonFont = f;

    buttonFont.setHeight(10);

    if (!getName().equalsIgnoreCase("all") && !getName().equalsIgnoreCase("none"))
    {
        setRadioGroupId(999);
        setClickingTogglesState(true);
    }

    selectedGrad = ColourGradient(Colour(240, 179, 12), 0.0, 0.0,
                                  Colour(207, 160, 33), 0.0, 20.0f,
                                  false);
    selectedOverGrad = ColourGradient(Colour(209, 162, 33), 0.0, 5.0f,
                                      Colour(190, 150, 25), 0.0, 0.0f,
                                      false);
    neutralGrad = ColourGradient(Colour(220, 220, 220), 0.0, 0.0,
                                 Colour(170, 170, 170), 0.0, 20.0f,
                                 false);
    neutralOverGrad = ColourGradient(Colour(180, 180, 180), 0.0, 5.0f,
                                     Colour(150, 150, 150), 0.0, 0.0,
                                     false);
}

EditorButton::~EditorButton() {}

bool EditorButton::getState()
{
    return isEnabled;
}

void EditorButton::setState(bool state)
{
    isEnabled = state;

    if (!state)
    {
        removeListener((Button::Listener*) getParentComponent());
    }
    else
    {
        addListener((Button::Listener*) getParentComponent());
    }

    repaint();
}

void EditorButton::resized()
{
    // float radius = 5.0f;
    float width = (float)getWidth();
    float height = (float)getHeight();

    if (getName().equalsIgnoreCase("AUDIO"))
    {
        //outlinePath.startNewSubPath(0, height);
        outlinePath.lineTo(0, 0);//radius);
        //outlinePath.addArc(0, 0, radius*2, radius*2, 1.5*double_Pi, 2.0*double_Pi );

        outlinePath.lineTo(width, 0);//getHeight());

        outlinePath.lineTo(width, height);

        outlinePath.lineTo(0, height);
        //outlinePath.addArc(0, getHeight()-radius*2, radius*2, radius*2, double_Pi, 1.5*double_Pi);
        //outlinePath.lineTo(0, radius);
        outlinePath.closeSubPath();

    }
    else if (getName().equalsIgnoreCase("PARAM"))
    {
        //outlinePath.startNewSubPath(0, 0);

        outlinePath.lineTo(width, 0);

        //outlinePath.addArc(width-radius*2, 0, radius*2, radius*2, 0, 0.5*double_Pi);

        outlinePath.lineTo(getWidth(), height);

        //outlinePath.addArc(getWidth()-radius*2, getHeight()-radius*2, radius*2, radius*2, 0.5*double_Pi, double_Pi);

        outlinePath.lineTo(0, height);
        outlinePath.lineTo(0, 0);
        //outlinePath.closeSubPath();


    }
    else if (getName().equalsIgnoreCase("REC"))
    {

        outlinePath.addRectangle(0, 0, getWidth(), getHeight());

    }
    else if (getName().equalsIgnoreCase("all"))
    {

        //outlinePath.startNewSubPath(0, 0);

        outlinePath.lineTo(width, 0);

        //outlinePath.addArc(width-radius*2, 0, radius*2, radius*2, 0, 0.5*double_Pi);

        outlinePath.lineTo(width, height);

        //outlinePath.addArc(getWidth()-radius*2, getHeight()-radius*2, radius*2, radius*2, 0.5*double_Pi, double_Pi);

        outlinePath.lineTo(0, height);
        //outlinePath.addArc(0, height-radius*2, radius*2, radius*2, double_Pi, 1.5*double_Pi);

        outlinePath.lineTo(0, 0);
        //outlinePath.closeSubPath();

    }
    else if (getName().equalsIgnoreCase("none"))
    {

        //outlinePath.startNewSubPath(0, 0);

        outlinePath.lineTo(width, 0);

        //outlinePath.addArc(width-radius*2, 0, radius*2, radius*2, 0, 0.5*double_Pi);

        outlinePath.lineTo(width, height);

        //outlinePath.addArc(width-radius*2, height-radius*2, radius*2, radius*2, 0.5*double_Pi, double_Pi);

        outlinePath.lineTo(0, height);

        outlinePath.lineTo(0, 0);
        //outlinePath.closeSubPath();

    }
    else if (getName().equalsIgnoreCase("+") )
    {
        outlinePath.lineTo(width, 0);
        outlinePath.lineTo(width, height);
        outlinePath.lineTo(0, height);
        outlinePath.lineTo(0, 0);
    }
    else if (getName().equalsIgnoreCase("-") )
    {
        outlinePath.lineTo(width, 0);
        outlinePath.lineTo(width, height);
        outlinePath.lineTo(0, height);
        outlinePath.lineTo(0, 0);
    }

}


void EditorButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{

    g.setColour(Colours::grey);
    g.fillPath(outlinePath);

    if (getToggleState())
    {
        if (isMouseOver && isEnabled)
            g.setGradientFill(selectedOverGrad);
        else
            g.setGradientFill(selectedGrad);
    }
    else
    {
        if (isMouseOver && isEnabled)
            g.setGradientFill(neutralOverGrad);
        else
            g.setGradientFill(neutralGrad);
    }


    AffineTransform a = AffineTransform::scale(0.98f, 0.94f, float(getWidth()) / 2.0f,
                        float(getHeight()) / 2.0f);
    g.fillPath(outlinePath, a);

    buttonFont.setHeight(10.0f);
    int stringWidth = buttonFont.getStringWidth(getName());

    g.setFont(buttonFont);

    if (isEnabled)
        g.setColour(Colours::darkgrey);
    else
        g.setColour(Colours::lightgrey);


    g.drawSingleLineText(getName(), getWidth() / 2 - stringWidth / 2, 11);

}


ChannelSelectorButton::ChannelSelectorButton(int num_, int type_, Font& f) : Button("name")
{
    isActive = true;
    num = num_;
    displayNum = num_;
    type = type_;

    setClickingTogglesState(true);

    buttonFont = f;
    buttonFont.setHeight (11);
}

ChannelSelectorButton::~ChannelSelectorButton() {}

int ChannelSelectorButton::getType()
{
    return type;
}

int ChannelSelectorButton::getChannel()
{
    return num;
}

void ChannelSelectorButton::setChannel(int n)
{
    num = n;
    displayNum = n;
}
void ChannelSelectorButton::setChannel(int n, int d)
{
    num = n;
    displayNum = d;
}

void ChannelSelectorButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (isActive)
    {
        if (getToggleState() == true)
            g.setColour(Colours::orange);
        else
            g.setColour(Colours::darkgrey);

        if (isMouseOver)
            g.setColour(Colours::white);
    }
    else
    {
        if (getToggleState() == true)
            g.setColour(Colours::yellow);
        else
            g.setColour(Colours::lightgrey);
    }

    // g.fillRect(0,0,getWidth(),getHeight());

    g.setFont(buttonFont);

    // g.drawRect(0,0,getWidth(),getHeight(),1.0);

    g.drawText(String(displayNum), 0, 0, getWidth(), getHeight(), Justification::centred, true);
}

void ChannelSelectorButton::setActive(bool t)
{
    isActive = t;
    setClickingTogglesState(t);
}


SlicerChannelSelectorComponent::SlicerChannelSelectorComponent (Channels::ChannelsType channelsType,
                                                                const String& componentName)
    : m_channelsType                (channelsType)
    , m_isCollapsed                 (true)
    , m_dropdownArrowImage          (ImageCache::getFromMemory (BinaryData::dropdown_arrow_rotated_png,
                                                                BinaryData::dropdown_arrow_rotated_pngSize))
    , m_dropdownArrowImageCollapsed (ImageCache::getFromMemory (BinaryData::dropdown_arrow_png,
                                                                BinaryData::dropdown_arrow_pngSize))

{
    m_channelSelectorTextEditor = new TextEditor;
    m_channelSelectorTextEditor->setMultiLine (false, true);
    m_channelSelectorTextEditor->setReturnKeyStartsNewLine (false);
    m_channelSelectorTextEditor->setTabKeyUsedAsCharacter (false);
    m_channelSelectorTextEditor->setTooltip ("General Format: [a:b:c]->to select all channels from a to c at intervals of b");
    m_channelSelectorTextEditor->addKeyListener (this);
    addAndMakeVisible (m_channelSelectorTextEditor);

    m_selectChannelsButton = new EditorButton ("+", FONT_DEFAULT);
    m_selectChannelsButton->setComponentID ("Select channels button");
    m_selectChannelsButton->setClickingTogglesState (false);
    m_selectChannelsButton->addListener (this);
    addAndMakeVisible (m_selectChannelsButton);

    m_deselectChannelsButton = new EditorButton ("-", FONT_DEFAULT);
    m_deselectChannelsButton->setComponentID ("Deselect channels button");
    m_deselectChannelsButton->setClickingTogglesState (false);
    m_deselectChannelsButton->addListener (this);
    addAndMakeVisible (m_deselectChannelsButton);

    m_showComponentButton = new ImageButton;
    m_showComponentButton->setImages (false, true, true,
                                      m_dropdownArrowImageCollapsed, 1.f, Colours::white,
                                      Image::null, 0.8f,  Colours::blue.withAlpha (0.5f),
                                      Image::null, 1.f, Colours::white);
    m_showComponentButton->addListener (this);
    addAndMakeVisible (m_showComponentButton);

    addKeyListener (this);

    setSize (0, SIZE_DROPDOWN_ARROW);
}


void SlicerChannelSelectorComponent::paint (Graphics& g)
{
    const int width  = getWidth();
    const int height = getHeight();

    // Draw horizontal line at the bottom of component at the center of arrow
    if (! m_isCollapsed)
    {
        g.setColour (Colours::black);
        g.drawHorizontalLine (height - SIZE_DROPDOWN_ARROW + 1, 0, width);
    }
}


void SlicerChannelSelectorComponent::resized()
{
    const int width  = getWidth();
    const int height = getHeight();
    const int margin = 5;
    const int textEditorHeight = 15;

    m_channelSelectorTextEditor->setBounds (margin, height - SIZE_DROPDOWN_ARROW - margin - textEditorHeight,
                                            95, textEditorHeight);

    juce::Rectangle<int> selectionControlBounds (110, m_channelSelectorTextEditor->getY(), 15, 15);
    m_selectChannelsButton->setBounds    (selectionControlBounds);
    m_deselectChannelsButton->setBounds  (selectionControlBounds.translated (20, 0));

    m_showComponentButton->setBounds ( (width - SIZE_DROPDOWN_ARROW) / 2 - 3, height - SIZE_DROPDOWN_ARROW,
                                       25, SIZE_DROPDOWN_ARROW);
}


void SlicerChannelSelectorComponent::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == m_showComponentButton)
    {
        setCollapsed (! m_isCollapsed);
    }
    else if (buttonThatWasClicked == m_selectChannelsButton)
    {
        m_controlsButtonListener->changeChannelsSelectionButtonClicked (this, buttonThatWasClicked, true);
    }
    else if (buttonThatWasClicked == m_deselectChannelsButton)
    {
        m_controlsButtonListener->changeChannelsSelectionButtonClicked (this, buttonThatWasClicked, false);
    }
}


bool SlicerChannelSelectorComponent::keyPressed (const KeyPress& key, Component* originatingComponent)
{
    // Collapse component by clicking "ESC" key either on component or in the TextEditor
    if ( (dynamic_cast<TextEditor*> (originatingComponent) != nullptr
            || originatingComponent == this)
        && key.isKeyCode (KeyPress::escapeKey))
    {
        setCollapsed (true);
    }

    return false;
}


void SlicerChannelSelectorComponent::setCollapsed (bool isCollapsed)
{
    m_isCollapsed = isCollapsed;

    auto& componentAnimator = Desktop::getInstance().getAnimator();

    // Show full component
    if (! m_isCollapsed)
    {
        juce::Rectangle<int> finalBounds (getX(), getY(), getWidth(), MAX_HEIGHT);
        componentAnimator.animateComponent (this, finalBounds, 1.f, DURATION_ANIMATION_COLLAPSE_MS, true, 1.0, 1.0);
    }
    // Collapse component
    else
    {
        juce::Rectangle<int> finalBounds (getX(), getY(), getWidth(), SIZE_DROPDOWN_ARROW);
        componentAnimator.animateComponent (this, finalBounds, 1.f, DURATION_ANIMATION_COLLAPSE_MS, true, 1.0, 1.0);
    }

    // Change buttons image (just a quick hack)
    m_showComponentButton->setImages (false, true, true,
                                      m_isCollapsed
                                      ? m_dropdownArrowImageCollapsed
                                      : m_dropdownArrowImage, 1.f, Colours::white,
                                      Image::null, 0.8f,  Colours::blue.withAlpha (0.5f),
                                      Image::null, 1.f, Colours::white);

    m_controlsButtonListener->channelSelectorCollapsedStateChanged (this, m_isCollapsed);
}


String SlicerChannelSelectorComponent::getText() const
{
    return m_channelSelectorTextEditor->getText();
}


Channels::ChannelsType SlicerChannelSelectorComponent::getChannelsType() const
{
    return m_channelsType;
}


void SlicerChannelSelectorComponent::setListener (SlicerChannelSelectorComponent::Listener* newListener)
{
    m_controlsButtonListener = newListener;
}
