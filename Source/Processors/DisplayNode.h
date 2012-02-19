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

#ifndef __DISPLAYNODE_H_C7B28CA4__
#define __DISPLAYNODE_H_C7B28CA4__


#include "../../JuceLibraryCode/JuceHeader.h"
#include "Editors/Visualizer.h"
#include "GenericProcessor.h"

class DataViewport;

class DisplayNode : public GenericProcessor

{
public:
	// real member functions:
	DisplayNode();
	~DisplayNode();

	AudioProcessorEditor* createEditor();

	bool isSink() {return true;}

	void setDestNode(GenericProcessor* sn) {destNode = 0;}

		void process(AudioSampleBuffer &buffer, MidiBuffer &midiMessages, int& nSamples) {}

	
private:

	DataViewport* dataViewport;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisplayNode);

};



#endif  // __DISPLAYNODE_H_C7B28CA4__
