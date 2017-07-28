/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

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

#include "CrossingDetectorEditor.h"
#include "CrossingDetector.h"

// standard component creation methods
Label* CrossingDetectorEditor::createEditable(const String& name, const String& initialValue,
    const String& tooltip, const Rectangle bounds)
{
    Label* editable = new Label(name, initialValue);
    editable->setEditable(true);
    editable->addListener(this);
    editable->setBounds(bounds);
    editable->setColour(Label::backgroundColourId, Colours::grey);
    editable->setColour(Label::textColourId, Colours::white);
    editable->setTooltip(tooltip);
    addAndMakeVisible(editable);
    return editable;
}

Label* CrossingDetectorEditor::createLabel(const String& name, const String& text, const Rectangle bounds)
{
    Label* label = new Label(name, text);
    label->setBounds(bounds);
    label->setFont(Font("Small Text", 12, Font::plain));
    label->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(label);
    return label;
}

CrossingDetectorEditor::CrossingDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors)
    : GenericEditor(parentNode, useDefaultParameterEditors)
{
    desiredWidth = 341;
    CrossingDetector* processor = static_cast<CrossingDetector*>(parentNode);

    /* ------------- CRITERIA SECTION ---------------- */

    inputLabel = createLabel("InputChanL", "Input", Rectangle(8, 36, 50, 18));

    inputBox = new ComboBox("Input channel");
    inputBox->setTooltip("Continuous channel to analyze");
    inputBox->setBounds(60, 36, 40, 18);
    inputBox->addListener(this);
    addAndMakeVisible(inputBox);

    risingButton = new UtilityButton("RISING", Font("Default", 10, Font::plain));
    risingButton->addListener(this);
    risingButton->setBounds(105, 26, 60, 18);
    risingButton->setClickingTogglesState(true);
    bool enable = processor->posOn;
    risingButton->setToggleState(enable, dontSendNotification);
    risingButton->setTooltip("Trigger events when past samples are below and future samples are above the threshold");
    addAndMakeVisible(risingButton);

    fallingButton = new UtilityButton("FALLING", Font("Default", 10, Font::plain));
    fallingButton->addListener(this);
    fallingButton->setBounds(105, 46, 60, 18);
    fallingButton->setClickingTogglesState(true);
    enable = processor->negOn;
    fallingButton->setToggleState(enable, dontSendNotification);
    fallingButton->setTooltip("Trigger events when past samples are above and future samples are below the threshold");
    addAndMakeVisible(fallingButton);

    acrossLabel = createLabel("AcrossL", "across", Rectangle(168, 36, 60, 18));

    thresholdEditable = createEditable("Threshold", String(processor->threshold),
        "Threshold voltage", Rectangle(230, 36, 50, 18));

    /* -------------- BEFORE SECTION ----------------- */

    pastSpanLabel = createLabel("PastSpanL", "Past:   Span:", Rectangle(8, 68, 100, 18));

    pastSpanEditable = createEditable("PastSpanE", String(processor->pastSpan),
        "Number of samples considered before a potential crossing", Rectangle(110, 68, 33, 18));

    pastStrictLabel = createLabel("PastStrictL", "Strictness:", Rectangle(155, 68, 110, 18));

    pastPctEditable = createEditable("PastPctE", String(100 * processor->pastStrict),
        "Percent of considered past samples required to be above/below threshold", Rectangle(250, 68, 33, 18));

    pastPctLabel = createLabel("pastPctL", "%", Rectangle(285, 68, 20, 18));

    /* --------------- AFTER SECTION ----------------- */

    futureSpanLabel = createLabel("FutureSpanL", "Future: Span:", Rectangle(8, 88, 100, 18));

    futureSpanEditable = createEditable("FutureSpanE", String(processor->futureSpan),
        "Number of samples considered after a potential crossing", Rectangle(110, 88, 33, 18));

    futureStrictLabel = createLabel("FutureStrictL", "Strictness:", Rectangle(155, 88, 110, 18));

    futurePctEditable = createEditable("FuturePctE", String(100 * processor->futureStrict),
        "Percent of considered future samples required to be above/below threshold", Rectangle(250, 88, 33, 18));

    futurePctLabel = createLabel("futurePctL", "%", Rectangle(285, 88, 20, 18));

 /*   afterLabel = createLabel("AfterL", "After:", Rectangle(8, 88, 65, 18));

    pctNextEditable = createEditable("Percent Next", String(100 * processor->fracNext),
        "Percent of considered future samples required to be above/below threshold", Rectangle(75, 88, 33, 18));

    aPctLabel = createLabel("PctNextL", "% of", Rectangle(110, 88, 40, 18));

    numNextEditable = createEditable("Num Next", String(processor->numNext),
        "Number of future samples considered", Rectangle(152, 88, 33, 18));

    aSampLabel = createLabel("SampNextL", "sample(s)", Rectangle(188, 88, 85, 18));
*/
    /* -------------- OUTPUT SECTION ----------------- */

    outputLabel = createLabel("OutL", "Output:", Rectangle(8, 108, 62, 18));

    eventBox = new ComboBox("Out event channel");
    for (int chan = 1; chan <= 8; chan++)
        eventBox->addItem(String(chan), chan);
    eventBox->setSelectedId(processor->eventChan + 1);
    eventBox->setBounds(72, 108, 35, 18);
    eventBox->setTooltip("Event channel to output on when triggered");
    eventBox->addListener(this);
    addAndMakeVisible(eventBox);

    durLabel = createLabel("DurL", "Dur:", Rectangle(112, 108, 35, 18));

    durationEditable = createEditable("Event Duration", String(processor->eventDuration),
        "Duration of each event, in samples", Rectangle(151, 108, 50, 18));

    timeoutLabel = createLabel("TimeoutL", "Timeout:", Rectangle(206, 108, 64, 18));

    timeoutEditable = createEditable("Timeout", String(processor->timeout),
        "Minimum number of samples between consecutive events", Rectangle(274, 108, 50, 18));
}

CrossingDetectorEditor::~CrossingDetectorEditor() {}

void CrossingDetectorEditor::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == inputBox)
        getProcessor()->setParameter(pInputChan, (float)inputBox->getSelectedId() - 1);
    else if (comboBoxThatHasChanged == eventBox)
        getProcessor()->setParameter(pEventChan, (float)eventBox->getSelectedId() - 1);
}

void CrossingDetectorEditor::labelTextChanged(Label* labelThatHasChanged)
{
    CrossingDetector* processor = static_cast<CrossingDetector*>(getProcessor());

    if (labelThatHasChanged == durationEditable)
    {
        int newVal;
        bool success = updateIntLabel(labelThatHasChanged, 0, INT_MAX, processor->eventDuration, &newVal);

        if (success)
            processor->setParameter(pEventDur, (float)newVal);
    }
    else if (labelThatHasChanged == timeoutEditable)
    {
        int newVal;
        bool success = updateIntLabel(labelThatHasChanged, 0, INT_MAX, processor->timeout, &newVal);

        if (success)
            processor->setParameter(pTimeout, (float)newVal);
    }
    else if (labelThatHasChanged == thresholdEditable)
    {
        float newVal;
        bool success = updateFloatLabel(labelThatHasChanged, -FLT_MAX, FLT_MAX, processor->threshold, &newVal);

        if (success)
            processor->setParameter(pThreshold, newVal);
    }
    else if (labelThatHasChanged == pastPctEditable)
    {
        float newVal;
        bool success = updateFloatLabel(labelThatHasChanged, 0, 100, 100 * processor->pastStrict, &newVal);

        if (success)
            processor->setParameter(pPastStrict, newVal / 100);
    }
    else if (labelThatHasChanged == pastSpanEditable)
    {
        int newVal;
        bool success = updateIntLabel(labelThatHasChanged, 0, processor->MAX_PAST_SPAN, processor->pastSpan, &newVal);

        if (success)
            processor->setParameter(pPastSpan, (float)newVal);
    }
    else if (labelThatHasChanged == futurePctEditable)
    {
        float newVal;
        bool success = updateFloatLabel(labelThatHasChanged, 0, 100, 100 * processor->futureStrict, &newVal);

        if (success)
            processor->setParameter(pFutureStrict, newVal / 100);
    }
    else if (labelThatHasChanged == futureSpanEditable)
    {
        int newVal;
        bool success = updateIntLabel(labelThatHasChanged, 0, processor->MAX_FUTURE_SPAN, processor->futureSpan, &newVal);

        if (success)
            processor->setParameter(pFutureSpan, (float)newVal);
    }
}

void CrossingDetectorEditor::buttonEvent(Button* button)
{
    if (button == risingButton)
        getProcessor()->setParameter(pPosOn, static_cast<float>(button->getToggleState()));
    else if (button == fallingButton)
        getProcessor()->setParameter(pNegOn, static_cast<float>(button->getToggleState()));
}

void CrossingDetectorEditor::updateSettings()
{
    CrossingDetector* processor = static_cast<CrossingDetector*>(getProcessor());

    // update input combo box
    int numInputs = processor->settings.numInputs;
    int numBoxItems = inputBox->getNumItems();
    if (numInputs != numBoxItems)
    {
        int currId = inputBox->getSelectedId();
        inputBox->clear(dontSendNotification);
        for (int chan = 1; chan <= numInputs; chan++)
            // using 1-based ids since 0 is reserved for "nothing selected"
            inputBox->addItem(String(chan), chan);
        if (numInputs > 0 && (currId < 1 || currId > numInputs))
            inputBox->setSelectedId(1, sendNotificationAsync);
        else
            inputBox->setSelectedId(currId, dontSendNotification);
    }
    
}

void CrossingDetectorEditor::startAcquisition()
{
    inputBox->setEnabled(false);
}

void CrossingDetectorEditor::stopAcquisition()
{
    inputBox->setEnabled(true);
}

void CrossingDetectorEditor::saveCustomParameters(XmlElement* xml)
{
    xml->setAttribute("Type", "CrossingDetectorEditor");

    CrossingDetector* processor = static_cast<CrossingDetector*>(getProcessor());
    XmlElement* paramValues = xml->createNewChildElement("VALUES");

    paramValues->setAttribute("inputChanId", inputBox->getSelectedId());
    paramValues->setAttribute("bRising", risingButton->getToggleState());
    paramValues->setAttribute("bFalling", fallingButton->getToggleState());
    paramValues->setAttribute("threshold", thresholdEditable->getText());
    paramValues->setAttribute("pastPct", pastPctEditable->getText());
    paramValues->setAttribute("pastSpan", pastSpanEditable->getText());
    paramValues->setAttribute("futurePct", futurePctEditable->getText());
    paramValues->setAttribute("futureSpan", futureSpanEditable->getText());
    paramValues->setAttribute("outputChanId", eventBox->getSelectedId());
    paramValues->setAttribute("duration", durationEditable->getText());
    paramValues->setAttribute("timeout", timeoutEditable->getText());
}

void CrossingDetectorEditor::loadCustomParameters(XmlElement* xml)
{
    forEachXmlChildElementWithTagName(*xml, xmlNode, "VALUES")
    {
        inputBox->setSelectedId(xmlNode->getIntAttribute("inputChanId", inputBox->getSelectedId()), sendNotificationSync);
        risingButton->setToggleState(xmlNode->getBoolAttribute("bRising", risingButton->getToggleState()), sendNotificationSync);
        fallingButton->setToggleState(xmlNode->getBoolAttribute("bFalling", fallingButton->getToggleState()), sendNotificationSync);
        thresholdEditable->setText(xmlNode->getStringAttribute("threshold", thresholdEditable->getText()), sendNotificationSync);
        pastPctEditable->setText(xmlNode->getStringAttribute("pastPct", pastPctEditable->getText()), sendNotificationSync);
        pastSpanEditable->setText(xmlNode->getStringAttribute("pastSpan", pastSpanEditable->getText()), sendNotificationSync);
        futurePctEditable->setText(xmlNode->getStringAttribute("futurePct", futurePctEditable->getText()), sendNotificationSync);
        futureSpanEditable->setText(xmlNode->getStringAttribute("futureSpan", futureSpanEditable->getText()), sendNotificationSync);
        eventBox->setSelectedId(xmlNode->getIntAttribute("outputChanId", eventBox->getSelectedId()), sendNotificationSync);
        durationEditable->setText(xmlNode->getStringAttribute("duration", durationEditable->getText()), sendNotificationSync);
        timeoutEditable->setText(xmlNode->getStringAttribute("timeout", timeoutEditable->getText()), sendNotificationSync);
    }
}

// static utilities

/* Attempts to parse the current text of a label as an int between min and max inclusive.
*  If successful, sets "*out" and the label text to this value and and returns true.
*  Otherwise, sets the label text to defaultValue and returns false.
*/
bool CrossingDetectorEditor::updateIntLabel(Label* label, int min, int max, int defaultValue, int* out)
{
    const String& in = label->getText();
    int parsedInt;
    try
    {
        parsedInt = std::stoi(in.toRawUTF8());
    }
    catch (const std::exception& e)
    {
        label->setText(String(defaultValue), dontSendNotification);
        return false;
    }

    if (parsedInt < min)
        *out = min;
    else if (parsedInt > max)
        *out = max;
    else
        *out = parsedInt;

    label->setText(String(*out), dontSendNotification);
    return true;
}

// Like updateIntLabel, but for floats
bool CrossingDetectorEditor::updateFloatLabel(Label* label, float min, float max, float defaultValue, float* out)
{
    const String& in = label->getText();
    float parsedFloat;
    try
    {
        parsedFloat = std::stof(in.toRawUTF8());
    }
    catch (const std::exception& e)
    {
        label->setText(String(defaultValue), dontSendNotification);
        return false;
    }

    if (parsedFloat < min)
        *out = min;
    else if (parsedFloat > max)
        *out = max;
    else
        *out = parsedFloat;

    label->setText(String(*out), dontSendNotification);
    return true;
}
