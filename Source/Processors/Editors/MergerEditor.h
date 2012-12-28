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

#ifndef __MERGEREDITOR_H_33F644A8__
#define __MERGEREDITOR_H_33F644A8__


#ifdef WIN32
#include <Windows.h>
#endif
#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"

/**

  User interface for the Merger utility.

  @see Merger

*/

class MergerEditor : public GenericEditor

{
public:
	MergerEditor (GenericProcessor* parentNode);
	virtual ~MergerEditor();

	virtual void buttonEvent (Button* button);

    void switchSource(int);
    void switchSource ();

    void switchIO(int);

private:	
	
	ImageButton* pipelineSelectorA;
	ImageButton* pipelineSelectorB;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MergerEditor);

};


#endif  // __MERGEREDITOR_H_33F644A8__
