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


#include "DisplayNode.h"
#include "ResamplingNode.h"
#include <stdio.h>

DisplayNode::DisplayNode()
	: GenericProcessor("Display Node")

{
	
}

DisplayNode::~DisplayNode()
{
}

AudioProcessorEditor* DisplayNode::createEditor()
{

	Visualizer* visualizer = new Visualizer(this, viewport, dataViewport);

	GenericProcessor* source = (GenericProcessor*) getSourceNode();


	visualizer->setBuffers(source->getContinuousBuffer(),source->getEventBuffer());
	visualizer->setUIComponent(getUIComponent());
	visualizer->setConfiguration(config);

	setEditor(visualizer);
	
	std::cout << "Creating visualizer." << std::endl;
	return visualizer;

}
