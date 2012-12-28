/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"
#include "ImageIcon.h"

class FilterViewport;
class ImageIcon;

/**

  User interface for the SourceNode.

  @see SourceNode

*/


class SourceNodeEditor : public GenericEditor

{
public:
	SourceNodeEditor (GenericProcessor* parentNode);
	virtual ~SourceNodeEditor();

private:	

	ImageIcon* icon;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceNodeEditor);

};



#endif  // __SOURCENODEEDITOR_H_A1B19E1E__
