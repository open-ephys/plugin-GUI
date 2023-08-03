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

#include "ParameterEditorOwner.h"


//ParameterEditorOwnerCommon
ParameterEditorOwner::ParameterEditorOwner(Component* ownerComponent_)
    : ownerComponent(ownerComponent_)
{
}

ParameterEditorOwner::~ParameterEditorOwner()
{}

ParameterEditorOwner::ParameterEditorOwner(const ParameterEditorOwner& other)
{
}

void ParameterEditorOwner::addParameterEditor(ParameterEditor* p, int xPos, int yPos)
{
    LOGD("@@@@@@@@ Adding parameter editor: ", p->getParameterName());
    parameterEditors.add(p);
    ownerComponent->addAndMakeVisible(p);
    p->setBounds(xPos, yPos, p->getWidth(), p->getHeight());
}

Array<ParameterEditor*> ParameterEditorOwner::getParameterEditors()
{
    Array<ParameterEditor*> editors;

    for(auto editor :  parameterEditors)
        editors.add(editor);
    
    return editors;
}

ParameterEditor* ParameterEditorOwner::getParameterEditor(String name) const
{
    for(auto editor :  parameterEditors)
    {
        if(editor->getParameterName().compare(name) == 0)
            return editor;
    }

    return nullptr;
}

bool ParameterEditorOwner::hasParameterEditor(String name) const
{
    for(auto editor :  parameterEditors)
    {
        if(editor->getParameterName().compare(name) == 0)
            return true;
    }

    return false;
}