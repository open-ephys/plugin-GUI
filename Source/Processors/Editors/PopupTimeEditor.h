/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2023 Open Ephys

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

#ifndef ___POPUPTIMEEDITOR_H_E47DE5C__
#define ___POPUPTIMEEDITOR_H_E47DE5C__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"
#include "../../Utils/Utils.h"

#include <regex>

class TimeParameter;
/*
    A popup editor for time parameters
*/
class PLUGIN_API PopupTimeEditor :
    public Component,
    public TextEditor::Listener
{
public:

    class CustomTextEditor : public TextEditor
    {
    public:
        CustomTextEditor(PopupTimeEditor* popupEditor) : popupEditor(popupEditor) {
            setJustification(Justification::centred); 
        }

        bool keyPressed(const KeyPress& key) override
        {
            if (key == KeyPress::escapeKey)
            {
                popupEditor->closePopup();
                return true;
            }

            return TextEditor::keyPressed(key);
        }

    private:
        PopupTimeEditor* popupEditor;
    };

    PopupTimeEditor(TimeParameter* p_) : p(p_),
        hourEditor(this),
        minuteEditor(this),
        secondEditor(this)
    {

        hourLabel.setText("H", dontSendNotification);
        hourLabel.setJustificationType(Justification::centred);
        hourLabel.setColour(Label::textColourId, Colours::white);
        addAndMakeVisible(hourLabel);

        minuteLabel.setText("M", dontSendNotification);
        minuteLabel.setJustificationType(Justification::centred);
        minuteLabel.setColour(Label::textColourId, Colours::white);
        addAndMakeVisible(minuteLabel);

        secondLabel.setText("S", dontSendNotification);
        secondLabel.setJustificationType(Justification::centred);
        secondLabel.setColour(Label::textColourId, Colours::white);
        addAndMakeVisible(secondLabel);

        int hour = p->getTimeValue()->getHours();
        hourEditor.setInputRestrictions(2, "0123456789");
        hourEditor.setText(String(hour));
        hourEditor.addListener(this);
        addAndMakeVisible(hourEditor);

        int minute = p->getTimeValue()->getMinutes();
        minuteEditor.setInputRestrictions(2, "0123456789");
        minuteEditor.setText(String(minute));
        minuteEditor.addListener(this);
        addAndMakeVisible(minuteEditor);

        double second = p->getTimeValue()->getSeconds();
        secondEditor.setInputRestrictions(6, "0123456789.");
        secondEditor.setText(String(second));
        secondEditor.addListener(this);
        addAndMakeVisible(secondEditor);

        setSize(115, 35);
    }

    ~PopupTimeEditor() {
        p->setNextValue(((TimeParameter*)p)->getTimeValue()->toString());
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(1);
        auto editorArea = area.removeFromTop(20);
        auto labelArea = area.removeFromTop(22);  // Area for the labels
        int padding = 2;

        int editorWidth = 30;
        int editorHeight = 20;
        int labelWidth = 30;
        int labelHeight = 20;

        hourEditor.setBounds(editorArea.removeFromLeft(editorWidth).reduced(padding));
        hourLabel.setBounds(labelArea.removeFromLeft(labelWidth).withSizeKeepingCentre(labelWidth, labelHeight));
        minuteEditor.setBounds(editorArea.removeFromLeft(editorWidth).reduced(padding));
        minuteLabel.setBounds(labelArea.removeFromLeft(labelWidth).withSizeKeepingCentre(labelWidth, labelHeight));
        secondEditor.setBounds(editorArea.removeFromLeft(2*editorWidth).reduced(padding));
        secondLabel.setBounds(labelArea.removeFromLeft(2*labelWidth).withSizeKeepingCentre(labelWidth, labelHeight));
    }

    void closePopup()
    {
        Component* parent = getParentComponent();
        while (parent != nullptr)
        {
            if (parent->isCurrentlyModal())
            {
                parent->exitModalState(0);
                break;
            }
            parent = parent->getParentComponent();
        }
    }

private:

    void textEditorTextChanged(TextEditor& editor) override
    {
        if (&editor == &hourEditor)
            p->getTimeValue()->setHours(editor.getText().getIntValue());
        else if (&editor == &minuteEditor)
            p->getTimeValue()->setMinutes(editor.getText().getIntValue());
        else if (&editor == &secondEditor)
        {
            String text = secondEditor.getText();

            StringArray split;
            split.addTokens(text, ".", "\"");
            if ((split.size() > 2) || 
                (split[0].length() > 2) || 
                ((split.size() > 1) && (split[1].length() > 3)) || 
                ((split.size() == 1) && (text.length() > 2)))
            {
                editor.setText(String(p->getTimeValue()->getSeconds()));
                return;
            }
            p->getTimeValue()->setSeconds(editor.getText().getDoubleValue());
        }
            
    }

    Label hourLabel;
    Label minuteLabel;
    Label secondLabel;

    CustomTextEditor hourEditor;
    CustomTextEditor minuteEditor;
    CustomTextEditor secondEditor;

    TimeParameter* p;

};

#endif  // ___POPUPTIMEEDITOR_H_E47DE5C__
