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

#ifndef __SOURCENODEEDITOR_H_A1B19E1E__
#define __SOURCENODEEDITOR_H_A1B19E1E__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"

/**

  Default interface for the SourceNode.

  Ideally, DataThreads should create custom
  editors to use instead.

  @see SourceNode

*/
class SourceNodeEditor : public GenericEditor

{
public:

    /** Constructor */
    SourceNodeEditor(GenericProcessor* parentNode);

    /** Destructor */
    virtual ~SourceNodeEditor() { }

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SourceNodeEditor);
    

};



#endif  // __SOURCENODEEDITOR_H_A1B19E1E__
