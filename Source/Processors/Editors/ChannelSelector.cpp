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

ChannelSelector::ChannelSelector(bool createButtons, Font& titleFont_) :
    eventsOnly(false), paramsToggled(true), paramsActive(true),
    recActive(true), radioStatus(false), isNotSink(createButtons),
    moveRight(false), moveLeft(false), offsetLR(0), offsetUD(0), desiredOffset(0), titleFont(titleFont_), acquisitionIsActive(false)
{
    // initialize buttons
    audioButton = new EditorButton("AUDIO", titleFont);
    audioButton->addListener(this);
    addAndMakeVisible(audioButton);
    if (!createButtons)
        audioButton->setState(false);

    recordButton = new EditorButton("REC", titleFont);
    recordButton->addListener(this);
    addAndMakeVisible(recordButton);
    if (!createButtons)
        recordButton->setState(false);

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

    selectButtonParam = new EditorButton("+", titleFont);
    selectButtonParam->addListener(this);
    addAndMakeVisible(selectButtonParam);

    deselectButtonParam = new EditorButton("-", titleFont);
    deselectButtonParam->addListener(this);
    addAndMakeVisible(deselectButtonParam);

    selectButtonRecord = new EditorButton("+", titleFont);
    selectButtonRecord->addListener(this);
    addAndMakeVisible(selectButtonRecord);

    deselectButtonRecord = new EditorButton("-", titleFont);
    deselectButtonRecord->addListener(this);
    addAndMakeVisible(deselectButtonRecord);

    selectButtonAudio = new EditorButton("+", titleFont);
    selectButtonAudio->addListener(this);
    addAndMakeVisible(selectButtonAudio);

    deselectButtonAudio = new EditorButton("-", titleFont);
    deselectButtonAudio->addListener(this);
    addAndMakeVisible(deselectButtonAudio);

    addAndMakeVisible (paramBox  = new ChannelSelectorBox);
    addAndMakeVisible (recordBox = new ChannelSelectorBox);
    addAndMakeVisible (audioBox  = new ChannelSelectorBox);

    numColumnsLessThan100 = 8;
    numColumnsGreaterThan100 = 6;

    addAndMakeVisible (audioButtonsManager);
    addAndMakeVisible (recordButtonsManager);
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

    // std::cout << difference << " buttons needed." << std::endl;

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
        int num = ( (GenericEditor*)getParentComponent())->getChannel (n)->nodeIndex;
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

    //std::cout << "offsetUD = " << offsetUD << std::endl;

    refreshButtonBoundaries();
}

void ChannelSelector::refreshButtonBoundaries()
{
    //int column = 0;
    //int row = 0;
    //int nColumns;

    //const int numButtons    = parameterButtonsManager.getNumButtons();
    const int columnWidth   = getDesiredWidth() / (numColumnsGreaterThan100 + 1) + 1;
    const int rowHeight     = 14;

    audioButtonsManager.setButtonSize      (columnWidth, rowHeight);
    recordButtonsManager.setButtonSize     (columnWidth, rowHeight);
    parameterButtonsManager.setButtonSize  (columnWidth, rowHeight);

    const int headerHeight = 25;
    //const int xLoc = columnWidth / 2 + offsetLR;
    //const int yLoc = offsetUD + headerHeight;
    const int xLoc = offsetLR + 3;
    const int yLoc = offsetUD + headerHeight * 2 - 5;

    const int tabButtonHeight = 15;
    juce::Rectangle<int> buttonManagerBounds (xLoc, yLoc, getDesiredWidth() - 6, getHeight() - yLoc - tabButtonHeight);
    parameterButtonsManager.setBounds (buttonManagerBounds);
    buttonManagerBounds.translate (- getDesiredWidth(), 0);
    recordButtonsManager.setBounds    (buttonManagerBounds);
    buttonManagerBounds.translate (- getDesiredWidth(), 0);
    audioButtonsManager.setBounds    (buttonManagerBounds);

    juce::Rectangle<int> textEditBounds (xLoc, yLoc - headerHeight, 90, 20);
    paramBox->setBounds     (textEditBounds);
    textEditBounds.translate (- getDesiredWidth(), 0);
    recordBox->setBounds    (textEditBounds);
    textEditBounds.translate (- getDesiredWidth(), 0);
    audioBox->setBounds     (textEditBounds);

    /*
      audio,record and param tabs
    */
    const int tabButtonWidth = getWidth() / 3;
    audioButton->setBounds  (0, 0, tabButtonWidth, tabButtonHeight);
    recordButton->setBounds (tabButtonWidth, 0, tabButtonWidth, tabButtonHeight);
    paramsButton->setBounds (tabButtonWidth * 2, 0, tabButtonWidth, tabButtonHeight);

    /*
      select and deselect button under each tab
    */
    juce::Rectangle<int> selectionControlBounds (xLoc + 95, 20, 20, 20);
    selectButtonParam->setBounds    (selectionControlBounds);
    deselectButtonParam->setBounds  (selectionControlBounds.translated (22, 0));

    selectionControlBounds.translate (- getDesiredWidth(), 0);
    selectButtonRecord->setBounds    (selectionControlBounds);
    deselectButtonRecord->setBounds  (selectionControlBounds.translated (22, 0));

    selectionControlBounds.translate (- getDesiredWidth(), 0);
    selectButtonAudio->setBounds    (selectionControlBounds);
    deselectButtonAudio->setBounds  (selectionControlBounds.translated (22, 0));

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
    //std::cout << desiredOffset - offsetLR << std::endl;

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
    //std::cout << "Setting active channels!" << std::endl;

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
        }
        else if (offsetLR == audioOffset)
        {
            for (int i = 0; i < audioButtonsManager.getNumButtons(); ++i)
            {
                audioButtonsManager.getButtonAt (i)->setToggleState (false, sendNotification);
            }
        }

        if (radioStatus) // if radio buttons are active
        {
            // send a message to parent
            GenericEditor* editor = (GenericEditor*) getParentComponent();
            editor->channelChanged (-1, false);
        }
    }
    else if (button == selectButtonParam)
    {   // select channels in parameter tab
        selectButtonParam->removeListener(this);
        deselectButtonParam->removeListener(this);
        std::vector<int> getBoxList;
        int fa, lim, comd, i;
        getBoxList = paramBox->getBoxInfo (parameterButtonsManager.getNumButtons());
        if (getBoxList.size() < 3)
        {
            selectButtonParam->addListener(this);
            deselectButtonParam->addListener(this);
            return;
        }
        i = 0;
        while (i <= getBoxList.size() - 3)
        {
            fa = getBoxList[i];
            lim = getBoxList[i + 1];
            comd = getBoxList[i + 2];
            for (; fa <= lim; fa += comd)
            {
                parameterButtonsManager.getButtonAt (fa)->setToggleState (true, sendNotification);
            }
            i += 3;
        }
        selectButtonParam->addListener(this);
        deselectButtonParam->addListener(this);
    }
    else if (button == selectButtonRecord)
    {   // select channels in record tab
        selectButtonRecord->removeListener(this);
        deselectButtonRecord->removeListener(this);
        std::vector<int> getBoxList;
        int fa, lim, comd, i;
        getBoxList = recordBox->getBoxInfo (recordButtonsManager.getNumButtons());
        if (getBoxList.size() < 3)
        {
            selectButtonRecord->addListener(this);
            deselectButtonRecord->addListener(this);
            return;
        }
        i = 0;
        while (i <= getBoxList.size() - 3)
        {
            fa = getBoxList[i];
            lim = getBoxList[i + 1];
            comd = getBoxList[i + 2];
            for (; fa <= lim; fa += comd)
            {
                recordButtonsManager.getButtonAt (fa)->setToggleState (true, sendNotification);
            }
            i += 3;
        }
        selectButtonRecord->addListener(this);
        deselectButtonRecord->addListener(this);
    }
    else if (button == selectButtonAudio)
    {   // select channels in audio tab
        selectButtonAudio->removeListener(this);
        deselectButtonAudio->removeListener(this);
        std::vector<int> getBoxList;
        int fa, lim, comd, i;
        getBoxList = audioBox->getBoxInfo (audioButtonsManager.getNumButtons());
        if (getBoxList.size() < 3)
        {
            selectButtonAudio->addListener(this);
            deselectButtonAudio->addListener(this);
            return;
        }
        i = 0;
        while (i <= getBoxList.size() - 3)
        {
            fa = getBoxList[i];
            lim = getBoxList[i + 1];
            comd = getBoxList[i + 2];
            for (; fa <= lim; fa += comd)
            {
                audioButtonsManager.getButtonAt (fa)->setToggleState(true, sendNotification);
            }
            i += 3;
        }
        selectButtonAudio->addListener(this);
        deselectButtonAudio->addListener(this);
    }
    else if (button == deselectButtonParam)
    {   // deselect channels in param tab
        selectButtonParam->removeListener(this);
        deselectButtonParam->removeListener(this);
        std::vector<int> getBoxList;
        int fa, lim, comd, i;
        getBoxList = paramBox->getBoxInfo (parameterButtonsManager.getNumButtons());
        if (getBoxList.size() < 3)
        {
            selectButtonParam->addListener(this);
            deselectButtonParam->addListener(this);
            return;
        }
        i = 0;
        while (i <= getBoxList.size() - 3)
        {
            fa = getBoxList[i];
            lim = getBoxList[i + 1];
            comd = getBoxList[i + 2];
            for (; fa <= lim; fa += comd)
            {
                parameterButtonsManager.getButtonAt (fa)->setToggleState (false, sendNotification);
            }
            i += 3;
        }
        selectButtonParam->addListener(this);
        deselectButtonParam->addListener(this);
    }
    else if (button == deselectButtonRecord)
    {   // deselect channels in record tab
        selectButtonRecord->removeListener(this);
        deselectButtonRecord->removeListener(this);
        std::vector<int> getBoxList;
        int fa, lim, comd, i;
        getBoxList = recordBox->getBoxInfo (recordButtonsManager.getNumButtons());
        if (getBoxList.size() < 3)
        {
            selectButtonRecord->addListener(this);
            deselectButtonRecord->addListener(this);
            return;
        }
        i = 0;
        while (i <= getBoxList.size() - 3)
        {
            fa = getBoxList[i];
            lim = getBoxList[i + 1];
            comd = getBoxList[i + 2];
            for (; fa <= lim; fa += comd)
            {
                recordButtonsManager.getButtonAt (fa)->setToggleState (false, sendNotification);
            }
            i += 3;
        }
        selectButtonRecord->addListener(this);
        deselectButtonRecord->addListener(this);
    }
    else if (button == deselectButtonAudio)
    {   // deselect channels in audio tab
        selectButtonAudio->removeListener(this);
        deselectButtonAudio->removeListener(this);
        std::vector<int> getBoxList;
        int fa, lim, comd, i;
        getBoxList = audioBox->getBoxInfo (audioButtonsManager.getNumButtons());
        if (getBoxList.size() < 3)
        {
            selectButtonAudio->addListener(this);
            deselectButtonAudio->addListener(this);
            return;
        }
        i = 0;
        while (i <= getBoxList.size() - 3)
        {
            fa = getBoxList[i];
            lim = getBoxList[i + 1];
            comd = getBoxList[i + 2];
            for (; fa <= lim; fa += comd)
            {
                audioButtonsManager.getButtonAt (fa)->setToggleState(false, sendNotification);
            }
            i += 3;
        }
        selectButtonAudio->addListener(this);
        deselectButtonAudio->addListener(this);
    }
    else
    {

        ChannelSelectorButton* b = (ChannelSelectorButton*)button;

        if (b->getType() == AUDIO)
        {
            // get audio node, and inform it of the change
            GenericEditor* editor = (GenericEditor*)getParentComponent();

            Channel* ch = editor->getChannel(b->getChannel() - 1);
            //int channelNum = editor->getStartChannel() + b->getChannel() - 1;
            bool status = b->getToggleState();

            std::cout << "Requesting audio monitor for channel " << ch->nodeIndex + 1 << std::endl;

            if (acquisitionIsActive) // use setParameter to change parameter safely
            {
                AccessClass::getProcessorGraph()->
                getAudioNode()->setChannelStatus(ch, status);
            }
            else     // change parameter directly
            {
                ch->isMonitored = status;
            }


        }
        else if (b->getType() == RECORD)
        {
            // get record node, and inform it of the change
            GenericEditor* editor = (GenericEditor*)getParentComponent();

            Channel* ch = editor->getChannel(b->getChannel() - 1);
            //int channelNum = editor->getStartChannel() + b->getChannel() - 1;
            bool status = b->getToggleState();

            if (acquisitionIsActive) // use setParameter to change parameter safely
            {
                AccessClass::getProcessorGraph()->
                getRecordNode()->
                setChannelStatus(ch, status);
            }
            else     // change parameter directly
            {
                //std::cout << "Setting record status for channel " << b->getChannel() << std::endl;
                ch->setRecordState(status);
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


///////////// BUTTONS //////////////////////


EditorButton::EditorButton(const String& name, Font& f) : Button(name)
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


/*
  Constructor and Destructor of ChannelSelectorBox.
*/
ChannelSelectorBox::ChannelSelectorBox()
{
    setMultiLine(false, true);                   // No multi lines.
    setReturnKeyStartsNewLine(false);            // Return key donot start a new line.
    setTabKeyUsedAsCharacter(false);
    setTooltip("General Format: [a:b:c]->to select all channels from a to c at intervals of b");
}

ChannelSelectorBox::~ChannelSelectorBox()
{

}

/*
  convert a string to integer.
*/
int ChannelSelectorBox::convertToInteger(std::string s)
{
    char ar[20];
    int i, k = 0;
    for (i = 0; i < s.size(); i++)
    {
        if (s[i] >= 48 && s[i] <= 57)
        {
            ar[k] = s[i];
            k++;
        }
    }
    if (k>7)
    {
        return 1000000;
    }
    ar[k] = '\0';
    k = atoi(ar);
    return k;
}


/*
   TextBox to take input. Valid formats:
   1. [ : ]  -> select/deselect all channels
   2. [ a : b]  -> select/deselect all channels from a to b.
   3. [ a : c : b] -> select/deselect all channels from a to b such that the difference between in each consecutive selected channel is c.
*/
std::vector<int> ChannelSelectorBox::getBoxInfo(int len)
{
    std::string s = ",";
    s += getText().toStdString();
    std::vector<int> finalList,separator,rangeseparator;
    int i, j, a, b, k, openb, closeb, otherchar,x,y;
    s += ",";
    for (i = 0; i < s.size(); i++)      //split string by ' , ' or ' ; ' 
    {
        if (s[i] == ';' || s[i] == ',')
        {
            separator.push_back(i);
        }
    }
    for (i = 0; i < separator.size()-1; i++)  // split ranges by ' : ' or ' - '
    {
        j = k = separator[i] + 1;
        openb = closeb = otherchar = 0;
        rangeseparator.clear();
        for (; j < separator[i + 1]; j++)
        {
            if (s[j] == '-' || s[j] == ':')
            {
                rangeseparator.push_back(j);
            }
            else if (((int)s[j] == 32))
            {
                continue;
            }
            else if (s[j] == '[' || s[j] == '{' || s[j] == '(')
            {
                openb++;
            }
            else if (s[j] == ']' || s[j] == '}' || s[j] == ')')
            {
                closeb++;
            }
            else if ( (int)s[j] > 57 || (int)s[j] < 48)
            {
                otherchar++;
            }
        }

        if (openb != closeb || openb > 1 || closeb > 1 || otherchar > 0)  //Invalid input
        {
            continue;
        }
        
        
        for (x = separator[i] + 1; x < separator[i + 1]; x++)       //trim whitespace and brackets from front
        {
            if (((int)s[x] >= 48 && (int)s[x] <= 57) || s[x] == ':' || s[x] == '-')
            {
                break;
            }
        }
        for (y = separator[i + 1] - 1; y > separator[i]; y--)       //trim whitespace and brackets from end
        {
            if (((int)s[y] >= 48 && (int)s[y] <= 57) || s[y] == ':' || s[y] == '-')
            {
                break;
            }
        }
        if (x > y)
        {
            continue;
        }


        if (rangeseparator.size() == 0)   //syntax of form - x or [x]
        {
            a = convertToInteger(s.substr(x, y - x + 1));
            if (a == 0||a>len)
            {
                continue;
            }
            finalList.push_back(a - 1);
            finalList.push_back(a - 1);
            finalList.push_back(1);
        }
        else if (rangeseparator.size() == 1) // syntax of type - x-y or [x-y]
        {
            a = convertToInteger(s.substr(x, rangeseparator[0] - x + 1));
            b = convertToInteger(s.substr(rangeseparator[0], y - rangeseparator[0] + 1));
            if (a == 0)
            {
                a = 1;
            }
            if (b == 0)
            {
                b = len;
            }
            if (a > b || a > len || b > len)
            {
                continue;
            }
            finalList.push_back(a - 1);
            finalList.push_back(b - 1);
            finalList.push_back(1);
        }
        else if (rangeseparator.size() == 2)   // syntax of type [x:y:z] or x-y-z
        {
            a = convertToInteger(s.substr(x, rangeseparator[0] - x + 1));
            k = convertToInteger(s.substr(rangeseparator[0], rangeseparator[1] - rangeseparator[0] + 1));
            b = convertToInteger(s.substr(rangeseparator[1], y - rangeseparator[1] + 1));
            if (a == 0)
            {
                a = 1;
            }
            if (b == 0)
            {
                b = len;
            }
            if (k == 0)
            {
                k = 1;
            }
            if (a > b || a > len || b > len)
            {
                continue;
            }
            finalList.push_back(a - 1);
            finalList.push_back(b - 1);
            finalList.push_back(k);
        }
    }
    return finalList;
}

