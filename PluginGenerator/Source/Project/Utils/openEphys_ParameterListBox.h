/*
   ------------------------------------------------------------------

   This file is part of the Open Ephys GUI
   Copyright (C) 2016 Open Ephys

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

#ifndef __OPEN_EPHYS_PARAMETERLISTBOX_H__
#define __OPEN_EPHYS_PARAMETERLISTBOX_H__

#include <JuceHeader.h>

#include "../../../../Source/Processors/Parameter/Parameter.h"

class Project;

class ParameterListBox  : public ListBox
                        , public ListBoxModel
                        , private Parameter::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {}

        /**
            This function is called when any parameter in the ParameterListBox was selected

            @param  parameterThatWasSelected    A pointer to the parameter, which was selected.
                                                If no parameter were set for the current row, returns nullptr.
            @param  row                         Number of the row which was clicked
        */
        virtual void parameterSelected (Parameter* parameterThatWasSelected, int row) = 0;
    };

    ParameterListBox (Project& project);

    // ListBoxModel methods
    // ========================================================================
    int getNumRows() override;
    void paintListBoxItem   (int row, Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked (int row, const MouseEvent& e) override;
    // ========================================================================

    // Parameter::Listener methods
    void parameterValueChanged (Value& valueThatWasChanged) override;

    String getItemText (int row) const noexcept;
    bool isExistsParameterForRow (int row) const noexcept;

    const OwnedArray<Parameter>& getAllParameters() const noexcept;

    void addListener    (Listener* listener)    { m_listeners.add (listener); }
    void removeListener (Listener* listener)    { m_listeners.remove (listener); }

    //virtual void listBoxItemDoubleClicked (int row, const MouseEvent& e);

private:
    void updateParametersFromProject();

    Project& m_project;

    OwnedArray<Parameter> m_parameters;

    ListenerList<Listener> m_listeners;

    // ========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterListBox);
};




#endif // __OPEN_EPHYS_PARAMETERLISTBOX_H__

