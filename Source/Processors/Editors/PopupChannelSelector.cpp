/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#include "PopupChannelSelector.h"
#include <string>
#include <vector>

#include "../../UI/LookAndFeel/CustomLookAndFeel.h"
#include "../../Utils/Utils.h"

ChannelButton::ChannelButton (int _id, PopupChannelSelector* _parent) : Button (String (_id)),
                                                                        id (_id),
                                                                        parent (_parent)
{
    setClickingTogglesState (true);
}

void ChannelButton::mouseDown (const MouseEvent& event)
{
    parent->firstButtonSelectedState = ! this->getToggleState();
    parent->mouseDown (event);
}

void ChannelButton::mouseDrag (const MouseEvent& event)
{
    parent->mouseDrag (event);
}

void ChannelButton::mouseUp (const MouseEvent& event)
{
    parent->mouseUp (event);
}

void ChannelButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    g.setColour (findColour (ThemeColours::outline));
    g.fillRoundedRectangle (0.0f, 0.0f, getWidth(), getHeight(), 0.001 * getWidth());

    if (isMouseOver)
    {
        if (getToggleState())
            g.setColour (parent->buttonColour.brighter());
        else
            g.setColour (findColour (ThemeColours::widgetBackground).contrasting (0.3f));
    }
    else
    {
        if (getToggleState())
            g.setColour (parent->buttonColour);
        else
            g.setColour (findColour (ThemeColours::widgetBackground));
    }
    g.fillRoundedRectangle (1, 1, getWidth() - 2, getHeight() - 2, 0.001 * getWidth());

    //Draw text string in middle of button
    g.setColour (getToggleState() ? Colours::white : findColour (ThemeColours::defaultText));
    g.setFont (FontOptions ("Inter", "Medium", 10.0f));
    g.drawText (String (id + 1), 0, 0, getWidth(), getHeight(), Justification::centred);
}

SelectButton::SelectButton (const String& name) : Button (name)
{
    setClickingTogglesState (true);
}

void SelectButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (isMouseOver)
    {
        if (getToggleState())
            g.setColour (findColour (ThemeColours::highlightedFill).withAlpha (0.5f));
        else
            g.setColour (findColour (ThemeColours::widgetBackground).contrasting (0.3f));
    }
    else
    {
        if (getToggleState())
            g.setColour (findColour (ThemeColours::highlightedFill));
        else
            g.setColour (findColour (ThemeColours::widgetBackground));
    }
    g.fillRoundedRectangle (1.0f, 1.0f, (float) getWidth() - 2.0f, (float) getHeight() - 2.0f, 2.0f);

    g.setColour (findColour (ThemeColours::outline));
    g.drawRoundedRectangle (0.0f, 0.0f, (float) getWidth(), (float) getHeight(), 2.0f, 1.0f);

    g.setColour (findColour (ThemeColours::defaultText));
    g.setFont (FontOptions ("Inter", "Regular", 12.0f));
    g.drawText (String (getName()), 0, 0, getWidth(), getHeight(), Justification::centred);
}

RangeEditor::RangeEditor (const String& name, const Font& font) : TextEditor (name)
{
    setFont (font);
}

PopupChannelSelector::PopupChannelSelector (Component* parent, PopupChannelSelector::Listener* listener_, std::vector<bool> channelStates, Array<String> channelNames, const String& title_)
    : PopupComponent (parent), listener (listener_), nChannels (int (channelStates.size())), mouseDragged (false), startDragCoords (0, 0), shiftKeyDown (false), firstButtonSelectedState (false), isDragging (false), editable (true), maxSelectable (-1), title (title_)
{
    int nColumns;

    if (nChannels <= 8)
    {
        nColumns = 8;
    }
    else if (nChannels <= 512)
    {
        nColumns = 16;
    }
    else if (nChannels <= 1024)
    {
        nColumns = 32;
    }
    else if (nChannels <= 2048)
    {
        nColumns = 64;
    }
    else
    {
        nColumns = 64;
    }

    int width = 23 * nColumns;
    int nRows = nChannels / nColumns + (int) (! (nChannels % nColumns == 0));
    int buttonSize = width / nColumns;
    int height = buttonSize * nRows;

    maxSelectable = (maxSelectable == -1) ? nChannels : maxSelectable;
    maxSelectable = (maxSelectable > nChannels) ? nChannels : maxSelectable;

    buttonColour = Colours::azure;

    contentComponent = std::make_unique<Component>();

    if (channelNames.isEmpty() || channelNames.size() != nChannels)
    {
        channelNames.clear();
        for (int i = 0; i < nChannels; i++)
        {
            channelNames.add ("CH" + String (i + 1));
        }
    }

    // Create buttons for each channel. Use actual channel names as tooltips.
    for (int i = 0; i < nRows; i++)
    {
        for (int j = 0; j < nColumns; j++)
        {
            if (nColumns * i + j < nChannels)
            {
                auto* button = new ChannelButton (nColumns * i + j, this);
                button->setBounds (width / nColumns * j, height / nRows * i, buttonSize, buttonSize);
                button->setToggleState (channelStates[nColumns * i + j], NotificationType::dontSendNotification);
                button->setTooltip (channelNames[nColumns * i + j]);
                button->addListener (this);
                contentComponent->addAndMakeVisible (button);

                if (channelStates[nColumns * i + j])
                    activeChannels.add (nColumns * i + j);

                channelButtons.add (button);
            }
        }
    }

    if (editable)
    {
        float widthScaling = nChannels > 8 ? 0.25 : 0.5;

        // Add "SELECT ALL" button
        auto* selectAllButton = new SelectButton ("ALL");
        selectAllButton->setBounds (0, height, widthScaling * width, width / nColumns);
        selectAllButton->addListener (this);
        contentComponent->addAndMakeVisible (selectAllButton);
        selectButtons.add (selectAllButton);

        // Add "SELECT NONE" button
        auto* selectNoneButton = new SelectButton ("NONE");
        selectNoneButton->setBounds (widthScaling * width, height, widthScaling * width, width / nColumns);
        selectNoneButton->addListener (this);
        contentComponent->addAndMakeVisible (selectNoneButton);
        selectButtons.add (selectNoneButton);

        if (nChannels > 8)
        {
            // Add "SELECT RANGE" button
            auto* selectRangeButton = new SelectButton ("RANGE");
            selectRangeButton->setBounds (0.5 * width, height, 0.25 * width, width / nColumns);
            selectRangeButton->addListener (this);
            contentComponent->addAndMakeVisible (selectRangeButton);
            selectButtons.add (selectRangeButton);

            // Add Range Editor
            rangeEditor = std::make_unique<RangeEditor> ("Range", FontOptions (12.0f));
            rangeEditor->setInputRestrictions (0, "0123456789:");
            rangeEditor->setBounds (0.75 * width, height, 0.25 * width, width / nColumns);
            rangeEditor->addListener (this);
            contentComponent->addAndMakeVisible (rangeEditor.get());
        }
    }

    contentComponent->setSize (width, height + (editable ? buttonSize : 0));

    int scrollBarThickness = 15;

    viewport = std::make_unique<Viewport>();
    viewport->setViewedComponent (contentComponent.get(), false);
    viewport->setScrollBarsShown (true, false);
    viewport->setScrollBarThickness (scrollBarThickness);

    addAndMakeVisible (viewport.get());

    if (nChannels > 2048)
    {
        viewport->setSize (width + scrollBarThickness, buttonSize * 32);
    }
    else
    {
        viewport->setSize (width, height + (editable ? buttonSize : 0));
    }

    if (title.isNotEmpty())
    {
        setSize (viewport->getWidth(), viewport->getHeight() + 24);
    }
    else
    {
        setSize (viewport->getWidth(), viewport->getHeight());
    }

    startDragCoords = Point<int> (-1, -1);
}

void PopupChannelSelector::resized()
{
    viewport->setBounds (0, getHeight() - viewport->getHeight(), getWidth(), viewport->getHeight());
}

void PopupChannelSelector::paint (Graphics& g)
{
    if (title.isNotEmpty())
    {
        g.setFont (FontOptions ("Inter", "Regular", 15.0f));
        g.setColour (findColour (ThemeColours::controlPanelText));
        g.drawFittedText (title, 5, 0, getWidth() - 10, 20, Justification::centredLeft, 1.0f);
    }
}

void PopupChannelSelector::updatePopup()
{
    if (nChannels != listener->getChannelCount())
    {
        findParentComponentOfClass<CallOutBox>()->exitModalState (0);
        return;
    }

    Array<int> selectedChannels = listener->getSelectedChannels();
    for (auto* btn : channelButtons)
    {
        if (selectedChannels.contains (btn->getId()))
            btn->setToggleState (true, NotificationType::sendNotification);
        else
            btn->setToggleState (false, NotificationType::sendNotification);
    }
}

void PopupChannelSelector::setMaximumSelectableChannels (int num)
{
    maxSelectable = num;

    maxSelectable = (maxSelectable > nChannels) ? nChannels : maxSelectable;
}

void PopupChannelSelector::setChannelButtonColour (Colour clr)
{
    buttonColour = clr;
}

ChannelButton* PopupChannelSelector::getButtonForId (int btnId)
{
    for (auto button : channelButtons)
    {
        if (button->getId() == btnId)
            return button;
    }

    return nullptr;
}

void PopupChannelSelector::mouseMove (const MouseEvent& event)
{
}

void PopupChannelSelector::mouseDown (const MouseEvent& event)
{
    if (editable)
    {
        selectedButtons.clear();
        startDragCoords = event.getEventRelativeTo (contentComponent.get()).getPosition();
    }
}

void PopupChannelSelector::mouseDrag (const MouseEvent& event)
{
    if (editable)
    {
        MouseEvent e = event.getEventRelativeTo (contentComponent.get());

        int w = e.getDistanceFromDragStartX();
        int h = e.getDistanceFromDragStartY();
        int x = startDragCoords.getX();
        int y = startDragCoords.getY();

        if (x < 0 || y < 0)
            return;

        mouseDragged = true;

        if (w < 0)
        {
            x = x + w;
            w = -w;
        }

        if (h < 0)
        {
            y = y + h;
            h = -h;
        }

        dragBox.setBounds (x, y, w > 0 ? w : 1, h > 0 ? h : 1);

        for (auto button : channelButtons)
        {
            if (button->getBounds().intersects (dragBox) && ! selectedButtons.contains (button->getId()))
            {
                selectedButtons.add (button->getId());

                if (shiftKeyDown) //toggle
                {
                    if (button->getToggleState())
                    {
                        button->triggerClick();
                        activeChannels.removeFirstMatchingValue (button->getId());
                    }
                    else
                    {
                        button->triggerClick();

                        if (activeChannels.size() == maxSelectable)
                        {
                            getButtonForId (activeChannels.getFirst())->triggerClick();
                            activeChannels.remove (0);
                        }

                        activeChannels.add (button->getId());
                    }
                }
                else //Use state of the first selected button
                {
                    if (firstButtonSelectedState)
                    {
                        button->setToggleState (firstButtonSelectedState, NotificationType::dontSendNotification);

                        if (activeChannels.size() == maxSelectable)
                        {
                            getButtonForId (activeChannels.getFirst())->triggerClick();
                            activeChannels.remove (0);
                        }

                        activeChannels.add (button->getId());
                    }
                    else
                    {
                        button->setToggleState (firstButtonSelectedState, NotificationType::dontSendNotification);
                        activeChannels.removeFirstMatchingValue (button->getId());
                    }
                }
            }
        }
    }
}

void PopupChannelSelector::modifierKeysChanged (const ModifierKeys& modifiers)
{
    shiftKeyDown = modifiers.isShiftDown();
}

void PopupChannelSelector::mouseUp (const MouseEvent& event)
{
    bool mouseClicked = false;

    if (! mouseDragged && editable)
    {
        for (auto button : channelButtons)
        {
            if (button->getBounds().contains (startDragCoords))
            {
                if (button->getToggleState())
                {
                    button->triggerClick();
                    LOGA ("Deselecting channel ", button->getId() + 1);
                    activeChannels.removeFirstMatchingValue (button->getId());
                }
                else
                {
                    button->triggerClick();

                    if (activeChannels.size() == maxSelectable && maxSelectable != nChannels)
                    {
                        getButtonForId (activeChannels.getFirst())->triggerClick();
                        activeChannels.remove (0);
                    }

                    LOGA ("Selecting channel ", button->getId() + 1);
                    activeChannels.add (button->getId());
                }

                mouseClicked = true;

                break;
            }
        }
    }

    if (mouseClicked || mouseDragged)
        listener->channelStateChanged (activeChannels);

    startDragCoords = Point<int> (-1, -1);
    mouseDragged = false;
}

void PopupChannelSelector::textEditorReturnKeyPressed (TextEditor& editor)
{
    LOGA ("RANGE select requested: ", editor.getText());

    if (editable)
    {
        channelStates = parseStringIntoRange (nChannels);

        if (channelStates.size() < 3)
            return;

        activeChannels.clear();

        for (auto* btn : channelButtons)
            btn->setToggleState (false, NotificationType::dontSendNotification);

        for (int i = 0; i < channelStates.size(); i += 3)
        {
            int startIdx = channelStates[i];
            int endIdx = channelStates[i + 1];
            int step = channelStates[i + 2];

            int ch = startIdx;
            while (ch < endIdx)
            {
                channelButtons[ch]->setToggleState (true, NotificationType::dontSendNotification);

                if (activeChannels.size() == maxSelectable)
                {
                    getButtonForId (activeChannels.getFirst())->triggerClick();
                    activeChannels.remove (0);
                }

                activeChannels.add (channelButtons[ch]->getId());

                ch += step;
            }
        }

        listener->channelStateChanged (activeChannels);
    }
}

void PopupChannelSelector::buttonClicked (Button* button)
{
    if (editable)
    {
        for (auto* btn : selectButtons)
            btn->setToggleState (false, NotificationType::dontSendNotification);

        if (button->getButtonText() == String ("ALL"))
        {
            activeChannels.clear();

            for (auto* btn : channelButtons)
                btn->setToggleState (false, NotificationType::dontSendNotification);

            for (int i = 0; i < maxSelectable; i++)
            {
                channelButtons[i]->setToggleState (true, NotificationType::dontSendNotification);
                activeChannels.add (channelButtons[i]->getId());
            }

            listener->channelStateChanged (activeChannels);
            button->setToggleState (true, NotificationType::dontSendNotification);
        }
        else if (button->getButtonText() == String ("NONE"))
        {
            for (auto* btn : channelButtons)
                btn->setToggleState (false, NotificationType::dontSendNotification);

            button->setToggleState (true, NotificationType::dontSendNotification);
            activeChannels.clear();
            listener->channelStateChanged (activeChannels);
        }
        else if (button->getButtonText() == String ("RANGE"))
        {
            button->setToggleState (true, NotificationType::dontSendNotification);
            this->textEditorReturnKeyPressed (*rangeEditor);
        }
        else //channel button was manually selected
        {
            //TODO: Update text box with range string
        }

        //rangeEditor->setText(rangeString);
    }
}

void PopupChannelSelector::updateRangeString()
{
    rangeString = "";

    int startIdx = 0;
    int endIdx = 0;

    bool inRange = false;

    for (int i = 0; i < nChannels; i++)
    {
        if (channelButtons[i]->getToggleState())
        {
            if (inRange)
            {
                if (i == nChannels - 1)
                {
                    rangeString += (rangeString == "" ? "" : ",");
                    rangeString += String (startIdx) + ":" + String (nChannels);
                }
            }
            else
            {
                if (i == nChannels - 1)
                {
                    rangeString += (rangeString == "" ? "" : ",");
                    rangeString += String (nChannels);
                }
                startIdx = i + 1;
            }
            inRange = true;
            endIdx = i + 1;
        }
        else
        {
            if (inRange)
            {
                rangeString += (rangeString == "" ? "" : ",");
                if (startIdx < endIdx)
                    rangeString += String (startIdx) + ":" + String (endIdx);
                else
                    rangeString += String (startIdx);
            }
            else
            {
            }
            inRange = false;
        }
    }
}

int PopupChannelSelector::convertStringToInteger (String s)
{
    char ar[20];
    int i, k = 0;
    for (i = 0; i < s.length(); i++)
    {
        if (s[i] >= 48 && s[i] <= 57)
        {
            ar[k] = s[i];
            k++;
        }
    }
    if (k > 7)
    {
        return 1000000;
    }
    ar[k] = '\0';
    k = atoi (ar);
    return k;
}

Array<int> PopupChannelSelector::parseStringIntoRange (int rangeValue)
{
    String s = ",";
    s += rangeEditor->getText();

    Array<int> finalList, separator, rangeseparator;
    int i, j, a, b, k, openb, closeb, otherchar, x, y;
    s += ",";
    for (i = 0; i < s.length(); i++) //split string by ' , ' or ' ; '
    {
        if (s[i] == ';' || s[i] == ',')
        {
            separator.add (i);
        }
    }
    for (i = 0; i < separator.size() - 1; i++) // split ranges by ' : ' or ' - '
    {
        j = k = separator[i] + 1;
        openb = closeb = otherchar = 0;
        rangeseparator.clear();
        for (; j < separator[i + 1]; j++)
        {
            if (s[j] == '-' || s[j] == ':')
            {
                rangeseparator.add (j);
            }
            else if (((int) s[j] == 32))
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
            else if ((int) s[j] > 57 || (int) s[j] < 48)
            {
                otherchar++;
            }
        }

        if (openb != closeb || openb > 1 || closeb > 1 || otherchar > 0) //Invalid input
        {
            continue;
        }

        for (x = separator[i] + 1; x < separator[i + 1]; x++) //trim whitespace and brackets from front
        {
            if (((int) s[x] >= 48 && (int) s[x] <= 57) || s[x] == ':' || s[x] == '-')
            {
                break;
            }
        }
        for (y = separator[i + 1] - 1; y > separator[i]; y--) //trim whitespace and brackets from end
        {
            if (((int) s[y] >= 48 && (int) s[y] <= 57) || s[y] == ':' || s[y] == '-')
            {
                break;
            }
        }
        if (x > y)
        {
            continue;
        }

        if (rangeseparator.size() == 0) //syntax of form - x or [x]
        {
            a = convertStringToInteger (s.substring (x, y + 1));
            if (a == 0 || a > rangeValue)
            {
                continue;
            }
            finalList.add (a - 1);
            finalList.add (a - 1);
            finalList.add (1);
        }
        else if (rangeseparator.size() == 1) // syntax of type - x-y or [x-y]
        {
            a = convertStringToInteger (s.substring (x, rangeseparator[0]));
            b = convertStringToInteger (s.substring (rangeseparator[0] + 1, y + 1));
            if (a == 0)
            {
                a = 1;
            }
            if (b == 0)
            {
                b = rangeValue;
            }
            if (a > b || a > rangeValue || b > rangeValue)
            {
                continue;
            }
            finalList.add (a - 1);
            finalList.add (b - 1);
            finalList.add (1);
        }
        else if (rangeseparator.size() == 2) // syntax of type [x:y:z] or x-y-z
        {
            a = convertStringToInteger (s.substring (x, rangeseparator[0] + 1));
            k = convertStringToInteger (s.substring (rangeseparator[0] + 1, rangeseparator[1]));
            b = convertStringToInteger (s.substring (rangeseparator[1] + 1, y + 1));

            if (a == 0)
            {
                a = 1;
            }
            if (b == 0)
            {
                b = rangeValue;
            }
            if (k == 0)
            {
                k = 1;
            }
            if (a > b || a > rangeValue || b > rangeValue)
            {
                continue;
            }
            finalList.add (a - 1);
            finalList.add (b - 1);
            finalList.add (k);
        }
    }
    return finalList;
}
