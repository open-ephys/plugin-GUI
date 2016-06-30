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

#ifndef CYCLOPS_EDITOR_H_INCLUDED
#define CYCLOPS_EDITOR_H_INCLUDED

#include <VisualizerEditorHeaders.h>
#include "CyclopsCanvas.h"
#include "CyclopsProcessor.h"

#include <string>
#include <iostream>

/**

Yo mama

@see GenericEditor
*/

class CyclopsProcessor;

class CyclopsEditor : public VisualizerEditor, public ComboBox::Listener
{
public:
    
    /** The class constructor, used to initialize any members. */
    CyclopsEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=false);

    /** The class destructor, used to deallocate memory */
    ~CyclopsEditor();

    Visualizer* createNewCanvas();

    /** 
    Unlike processors, which have a minimum set of required methods,
    editor are completely customized. There are still a couple of
    sometimes useful overloaded methods, which will appear here
    */

    /** This method executes whenever a custom button is pressed */
    virtual void buttonCallback(Button* button);

    /** Combobox listener callback, called when a combobox is changed. */
    void comboBoxChanged(ComboBox* box);

    /** Called to inform the editor that acquisition is about to start*/
    void startAcquisition();

    /** Called to inform the editor that acquisition has just stopped*/
    void stopAcquisition();

    /** Called whenever there is a change in the signal chain or it refreshes.
        It's called after the processors' same named method.
    */
    void updateSettings();

    void saveEditorParameters(XmlElement* xmlNode);
    void loadEditorParameters(XmlElement* xmlNode);
private:

    // Button that reloads device list
    ScopedPointer<UtilityButton> refreshButton;
    // List of all available dvices
    ScopedPointer<ComboBox> portList;
    // List of all available baudrates.
    ScopedPointer<ComboBox> baudrateList;
    // Parent node
    CyclopsProcessor* node;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CyclopsEditor);
};

#endif