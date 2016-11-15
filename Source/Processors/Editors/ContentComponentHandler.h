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

#ifndef __CONTENTCOMPONENTHANDLER_H__
#define __CONTENTCOMPONENTHANDLER_H__

#include <JuceHeader.h>
#include "../PluginManager/OpenEphysPlugin.h"
#include "../Parameter/Parameter.h"

class GenericProcessor;

class PLUGIN_API ContentComponentHandler
{
public:
    virtual Component& getContentComponent() = 0;

    /** Configures parameter editors from content component (if it exists) to set the same values
        accordingly to the parameters and configure needed options for them. */
    virtual void configureParameterEditors (GenericProcessor* processor);

    virtual void updateParameterComponent (Parameter* parameterToUpdate);

    void setContentComponentHandlerOwner (Component* parent);


private:
    Button::Listener*       m_buttonListener;
    Slider::Listener*       m_sliderListener;
    TextEditor::Listener*   m_textEditorListener;
};



#endif // CONTENTCOMPONENTHANDLER_H__
