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
#include <UIUtilitiesHeaders.h>
#include "CyclopsCanvas.h"
#include "CyclopsProcessor.h"

#include <string>
#include <iostream>


namespace cyclops{
    namespace CyclopsColours{
    const Colour disconnected(0xffff3823);
    const Colour notResponding(0xffffa834);
    const Colour connected(0xffc1d045);
    const Colour notReady       = disconnected;
    const Colour Ready          = connected;
    }
class CyclopsProcessor;
class IndicatorLED;

class CyclopsEditor : public VisualizerEditor
                    , public ComboBox::Listener
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
    void buttonEvent(Button* button);

    /** Combobox listener callback, called when a combobox is changed. */
    void comboBoxChanged(ComboBox* box);

    void timerCallback();
    void paint(Graphics& g);

    /** Disables all input widgets on the editor. */
    void disableAllInputWidgets();
    /** Enables all input widgets on the editor. */
    void enableAllInputWidgets();

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

    ScopedPointer<IndicatorLED> serialLED;
    ScopedPointer<IndicatorLED> readinessLED;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CyclopsEditor);
};

class IndicatorLED : public Component
                   , public SettableTooltipClient
{
public:
    IndicatorLED (const Colour& fill, const Colour& line);
    void paint (Graphics& g);
    void update (const Colour& fill, String& tooltip);
    void update (const Colour& fill, const Colour& line, String& tooltip);
private:
    Colour fillColour, lineColour;
};

} // NAMESPACE cyclops
#endif
