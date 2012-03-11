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

#include "Visualizer.h"

Visualizer::Visualizer()
{

	nChans = processor->getNumInputs();
	sampleRate = processor->getSampleRate();
	std::cout << "Setting num inputs on Visualizer to " << nChans << std::endl;

	displayBuffer = processor->getDisplayBufferAddress();
	displayBufferSize = displayBuffer->getNumSamples();
		std::cout << "Setting displayBufferSize on Visualizer to " << displayBufferSize << std::endl;


	totalHeight = (plotHeight+yBuffer)*nChans + yBuffer;

	screenBuffer = new AudioSampleBuffer(nChans, 10000);
	
}

Visualizer::~Visualizer()
{
}


