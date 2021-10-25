/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2021 Open Ephys

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

#ifndef __FILTEREDITOR_H_969BDB5__
#define __FILTEREDITOR_H_969BDB5__

#include <EditorHeaders.h>

/**

  User interface for the FilterNode processor.

  @see FilterNode

*/

class FilterEditor : public GenericEditor
{
public:

    /** Constructor */
    FilterEditor(GenericProcessor* parentNode);
    
    /** Destructor */
    ~FilterEditor() { }

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterEditor);

};



#endif  // __FILTEREDITOR_H_969BDB5__
