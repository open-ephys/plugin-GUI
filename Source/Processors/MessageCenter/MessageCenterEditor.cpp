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

#include "MessageCenterEditor.h"
#include "MessageCenter.h"

#include "../Editors/GenericEditor.h" // for UtilityButton class

MessageCenterEditor::MessageCenterEditor (MessageCenter* owner) : AudioProcessorEditor (owner),
                                                                  messageCenter (owner),
                                                                  incomingBackground (Colours::black.withAlpha (0.0f)),
                                                                  outgoingBackground (Colours::black.withAlpha (0.0f)),
                                                                  backgroundColour (Colours::grey.withAlpha (0.0f))
{
    incomingMessageDisplayArea = std::make_unique<MessageLabel> ("Message Display Area", "No new messages");
    editableMessageDisplayArea = std::make_unique<MessageLabel> ("Message Display Area", "Type a new message here.");
    editableMessageDisplayArea->setEditable (true);
    editableMessageDisplayArea->addListener (this);

    incomingMessageViewport = std::make_unique<Viewport> ("Incoming message viewport.");
    outgoingMessageViewport = std::make_unique<Viewport> ("Outgoing message viewport.");

    incomingMessageLog = std::make_unique<MessageLog> (incomingMessageViewport.get());
    outgoingMessageLog = std::make_unique<MessageLog> (outgoingMessageViewport.get());

    incomingMessageViewport->setViewedComponent (incomingMessageLog.get(), false);
    outgoingMessageViewport->setViewedComponent (outgoingMessageLog.get(), false);
    incomingMessageViewport->setScrollBarsShown (true, false);
    outgoingMessageViewport->setScrollBarsShown (true, false);

    addAndMakeVisible (incomingMessageDisplayArea.get());
    addAndMakeVisible (editableMessageDisplayArea.get());

    addAndMakeVisible (incomingMessageViewport.get());
    addAndMakeVisible (outgoingMessageViewport.get());

    sendMessageButton = std::make_unique<TextButton> ("Send Message Button");
    sendMessageButton->setButtonText ("Send");
    sendMessageButton->addListener (this);
    sendMessageButton->setEnabled (false);
    addAndMakeVisible (sendMessageButton.get());
}

MessageCenterEditor::~MessageCenterEditor() {}

void MessageCenterEditor::startAcquisition()
{
    sendMessageButton->setEnabled (true);
}

void MessageCenterEditor::stopAcquisition()
{
    sendMessageButton->setEnabled (false);
}

void MessageCenterEditor::buttonClicked (Button* button)
{
    if (editableMessageDisplayArea->getText().length() > 0)
    {
        messageCenter->addOutgoingMessage (editableMessageDisplayArea->getText(), CoreServices::getSystemTime());
    }
}

void MessageCenterEditor::editorShown (Label* label, TextEditor& textEditor)
{
    textEditor.setInputRestrictions (490);
}

void MessageCenterEditor::timerCallback()
{
    bool shouldRepaint = false;

    float incomingAlpha = incomingBackground.getFloatAlpha();
    float outgoingAlpha = outgoingBackground.getFloatAlpha();
    float backgroundAlpha = backgroundColour.getFloatAlpha();

    if (incomingAlpha > 0)
    {
        incomingAlpha -= 0.05;

        if (incomingAlpha < 0)
            incomingAlpha = 0;

        incomingBackground = incomingBackground.withAlpha (incomingAlpha);

        shouldRepaint = true;
    }

    if (outgoingAlpha > 0)
    {
        outgoingAlpha -= 0.05;

        if (outgoingAlpha < 0)
            outgoingAlpha = 0;

        outgoingBackground = outgoingBackground.withAlpha (outgoingAlpha);

        shouldRepaint = true;
    }

    if (backgroundAlpha > 0)
    {
        backgroundAlpha -= 0.05;

        if (backgroundAlpha < 0)
            backgroundAlpha = 0;

        backgroundColour = backgroundColour.withAlpha (backgroundAlpha);

        shouldRepaint = true;
    }

    if (shouldRepaint)
        repaint();
    else
        stopTimer();
}

void MessageCenterEditor::addIncomingMessage (const String& message)
{
    if (firstMessageReceived)
        incomingMessageLog->addMessage (new MessageLabel ("message",
                                                          incomingMessageDisplayArea->getText(),
                                                          incomingMessageDisplayArea->getTimeString()));
    else
        firstMessageReceived = true;

    incomingMessageDisplayArea->setText (message, sendNotification);
    incomingMessageDisplayArea->setTimeString (Time::getCurrentTime().toString (false, true, true, true));

    incomingBackground = Colour (206, 172, 202);
    startTimer (75);
    resized();
}

void MessageCenterEditor::addOutgoingMessage (const String& message, const int64 systemTime)
{
    Time time (systemTime);
    String timeString = time.toString (false, true, true, true);
    outgoingMessageLog->addMessage (new MessageLabel ("message", message, timeString));

    editableMessageDisplayArea->setText (message, dontSendNotification);
    editableMessageDisplayArea->setTimeString (timeString);

    outgoingBackground = Colour (244, 208, 80);

    resized();
    startTimer (75);
}

void MessageCenterEditor::expand()
{
    if (! isExpanded)

    {
        isExpanded = true;

        backgroundColour = findColour (ThemeColours::widgetBackground);

        resized();

        startTimer (15);
    }
}

void MessageCenterEditor::collapse()
{
    if (isExpanded)
    {
        isExpanded = false;

        backgroundColour = findColour (ThemeColours::componentBackground);

        resized();

        startTimer (40);
    }
}

void MessageCenterEditor::paint (Graphics& g)
{
    g.setColour (findColour (ThemeColours::outline).withAlpha (0.5f)); // edge colour

    g.drawRoundedRectangle (1, 1, getWidth() - 2, getHeight() - 2, 6.0f, 2.0f);

    if (isExpanded)
        g.setColour (findColour (ThemeColours::widgetBackground).withAlpha (0.85f));
    else
        g.setColour (findColour (ThemeColours::componentBackground));

    g.fillRoundedRectangle (2, 2, getWidth() - 4, getHeight() - 4, 6.0f);

    g.setColour (backgroundColour);
    g.fillRoundedRectangle (2, 2, getWidth() - 4, getHeight() - 4, 6.0f);

    g.setColour (findColour (ThemeColours::outline).withAlpha (0.5f)); // dividing line

    if (isExpanded)
        g.fillRect (getWidth() / 2 + 9, 10, 2, getHeight() - 30);

    g.setColour (incomingBackground); // incoming background

    g.fillRoundedRectangle (4,
                            getHeight() - 25,
                            getWidth() / 2 - 12,
                            20,
                            5.0f);

    g.setColour (outgoingBackground); // outgoing background

    g.fillRoundedRectangle (getWidth() / 2 + 26,
                            getHeight() - 25,
                            getWidth() / 2 - 82,
                            20,
                            5.0f);

    if (isExpanded)
    {
        g.setFont (FontOptions ("CP Mono", "Bold", 80));
        g.setColour (findColour (ThemeColours::defaultFill).withAlpha (0.25f));
        g.drawText ("INCOMING", 4, 25, getWidth() / 2 - 11, getHeight() - 60, Justification::centred);
        g.drawText ("OUTGOING", getWidth() / 2 + 26, 25, getWidth() / 2 - 34, getHeight() - 60, Justification::centred);
    }

    g.setColour (findColour (ThemeColours::outline).withAlpha (0.5f));
    g.drawRect (getWidth() / 2 + 26, getHeight() - 25, getWidth() / 2 - 80, 20);

    sendMessageButton->setColour (TextButton::buttonColourId, findColour (ThemeColours::highlightedFill));
}

void MessageCenterEditor::resized()
{
    if (incomingMessageDisplayArea != 0)
        incomingMessageDisplayArea->setBounds (4, getHeight() - 25, getWidth() / 2 - 11, 20);

    if (editableMessageDisplayArea != 0)
        editableMessageDisplayArea->setBounds (getWidth() / 2 + 26, getHeight() - 25, getWidth() / 2 - 80, 20);

    if (incomingMessageLog != 0)
    {
        float h = incomingMessageLog->getDesiredHeight();

        if (h < 265)
        {
            incomingMessageViewport->setBounds (4,
                                                getHeight() - h - 35,
                                                getWidth() / 2 - 11,
                                                h);
        }
        else
        {
            incomingMessageViewport->setBounds (4,
                                                25,
                                                getWidth() / 2 - 11,
                                                getHeight() - 60);
        }

        incomingMessageLog->setBounds (0,
                                       0,
                                       getWidth() / 2 - 11,
                                       h);
    }

    if (outgoingMessageLog != 0)
    {
        float h = outgoingMessageLog->getDesiredHeight();

        if (h < 265)
        {
            outgoingMessageViewport->setBounds (getWidth() / 2 + 26,
                                                getHeight() - h - 35,
                                                getWidth() / 2 - 34,
                                                h);
        }
        else
        {
            outgoingMessageViewport->setBounds (getWidth() / 2 + 26,
                                                25,
                                                getWidth() / 2 - 34,
                                                getHeight() - 60);
        }

        outgoingMessageLog->setBounds (0,
                                       0,
                                       getWidth() / 2 - 10,
                                       h);
    }

    if (sendMessageButton != 0)
        sendMessageButton->setBounds (getWidth() - 50, getHeight() - 25, 45, 20);

    incomingMessageViewport->setViewPositionProportionately (0, 1);
    outgoingMessageViewport->setViewPositionProportionately (0, 1);
}

// #################################################################

MessageLabel::MessageLabel (const String& componentName, const String& labelText, const String& timeString_)
    : Label (componentName, labelText)
{
    setJustificationType (Justification::bottomLeft);
    setFont (FontOptions ("CP Mono", "Plain", 16));
    setBorderSize (BorderSize<int> (0, 7, 2, 0));
    setMinimumHorizontalScale (1.0f);

    if (timeString_.isNotEmpty())
        timestring = timeString_;
    else
        timestring = Time::getCurrentTime().toString (false, true, true, true);
}

String MessageLabel::getTooltip()
{
    return "Time of message: " + timestring;
}

void MessageLabel::prependText (String text)
{
    setText (text + getText(), dontSendNotification);
}

void MessageLabel::setTimeString (const String& timeString_)
{
    timestring = timeString_;
}

String MessageLabel::getTimeString() const
{
    return timestring;
}

MessageLog::MessageLog (Viewport* vp) : viewport (vp),
                                        messageHeight (20)
{
}

void MessageLog::addMessage (MessageLabel* message)
{
    messages.add (message);
    message->prependText (getMessageNumberAsString() + ": ");
    addAndMakeVisible (message);
}

String MessageLog::getMessageNumberAsString()
{
    int num_messages = messages.size();

    if (num_messages < 10)
    {
        return "000" + String (num_messages);
    }
    else if (num_messages >= 10 && num_messages < 100)
    {
        return "00" + String (num_messages);
    }
    else if (num_messages >= 100 && num_messages < 1000)
    {
        return "0" + String (num_messages);
    }
    else
    {
        return String (num_messages);
    }
}

int MessageLog::getDesiredHeight()
{
    return messages.size() * messageHeight + messageHeight;
}

void MessageLog::resized()
{
    int index = 0;
    for (auto& message : messages)
    {
        message->setBounds (0, messageHeight * ++index, getWidth(), messageHeight);
    }
}

void MessageLog::copyText()
{
    String text = "";

    for (auto& message : messages)
    {
        text += message->getText() + "\n";
    }

    SystemClipboard::copyTextToClipboard (text);
}
