/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys
Copyright (C) 2014 Michael Borisov

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


#ifndef __ECUBEEDITOR_H_D3EC8BA8__
#define __ECUBEEDITOR_H_D3EC8BA8__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"



/**

User interface for the "eCube" source node.

@see SourceNode, eCube

*/

#ifdef ECUBE_COMPILE

class EcubeEditor : public GenericEditor,
    public Slider::Listener, public ComboBox::Listener
{
public:
    EcubeEditor(GenericProcessor* parentNode, EcubeThread* npThread, bool useDefaultParameterEditors);
    virtual ~EcubeEditor();

    //void buttonEvent(Button* button);
    virtual void sliderValueChanged(Slider* slider);
    virtual void comboBoxChanged(ComboBox* comboBoxThatHasChanged);

    void saveEditorParameters(XmlElement*);
    void loadEditorParameters(XmlElement*);

private:
    ScopedPointer<Label> volLabel;
    ScopedPointer<Slider> volSlider;
    ScopedPointer<Label> chanLabel;
    ScopedPointer<ComboBox> chanComboBox;
    ScopedPointer<Label> samplerateLabel;
    ScopedPointer<Label> samplerateValueLabel;

    EcubeThread* pThread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EcubeEditor);

};

#endif // ECUBE_COMPILE

#endif  // __ECUBEEDITOR_H_D3EC8BA8__
