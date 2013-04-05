/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#ifndef __RHD2000EDITOR_H_2AD3C591__
#define __RHD2000EDITOR_H_2AD3C591__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"

/**

  User interface for the RHD2000 source module.

  @see SourceNode

*/

class HeadstageOptionsInterface;


class RHD2000Editor : public GenericEditor

{
public:
    RHD2000Editor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    ~RHD2000Editor();

    void resized();

private:

	OwnedArray<HeadstageOptionsInterface> headstageOptionsInterfaces;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RHD2000Editor);

};


class HeadstageOptionsInterface : public Component
{
public:
	HeadstageOptionsInterface(int hsNum);
	~HeadstageOptionsInterface();

	void paint(Graphics& g);

private:

	int hsNumber;
	String name;

};



#endif  // __RHD2000EDITOR_H_2AD3C591__
