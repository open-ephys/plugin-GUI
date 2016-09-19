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

#ifndef __OPEN_EPHYS_EDITOR_TEMPLATES_FACTORY_H__
#define __OPEN_EPHYS_EDITOR_TEMPLATES_FACTORY_H__

#include <JuceHeader.h>

class EditorTemplateComponent;


/** A factory which creates appropiate template components for both
    GenericEditor and VisualizerEditor classes. */
class EditorTemplatesFactory
{
public:
    static Array<EditorTemplateComponent*>* createGenericEditorTemplates();
    static Array<EditorTemplateComponent*>* createVisualizerEditorTemplates();
};


#endif // __OPEN_EPHYS_PLUGIN_TEMPLATES_PAGE_COMPONENT_H__

