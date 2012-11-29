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

#include <stdio.h>
#include "ReferenceNode.h"
#include "Editors/ReferenceNodeEditor.h"



ReferenceNode::ReferenceNode()
	: GenericProcessor("Digital Reference")

{


}

ReferenceNode::~ReferenceNode()
{

}

AudioProcessorEditor* ReferenceNode::createEditor()
{
	editor = new ReferenceNodeEditor(this);
	
	std::cout << "Creating editor." << std::endl;

	return editor;
}



void ReferenceNode::updateSettings()
{		


				
}

void ReferenceNode::setParameter (int parameterIndex, float newValue)
{



}

void ReferenceNode::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &midiMessages,
                            int& nSamples)
{


}

