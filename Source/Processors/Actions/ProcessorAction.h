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

#ifndef PROCESSORACTION_H_INCLUDED
#define PROCESSORACTION_H_INCLUDED

#include <JuceHeader.h>

#include <map>
#include <string>
#include <vector>

/**
  Defines an UndoableAction that is specific to a Processor.

  @see UndoableAction, GenericProcessor
*/
class PLUGIN_API ProcessorAction : public UndoableAction
{
public:
    ProcessorAction (const std::string& name_) : name (name_) {}

    /* Defines the perform action */
    bool perform() override = 0;

    /* Defines the undo action */
    bool undo() override = 0;

    /* Restores the action's owner */
    virtual void restoreOwner (GenericProcessor* p) = 0;

private:
    std::string name;
};

#endif // PROCESSORACTION_H_INCLUDED