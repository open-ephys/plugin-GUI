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

#include "PlaceholderProcessor.h"
#include "PlaceholderProcessorEditor.h"

PlaceholderProcessor::PlaceholderProcessor(String pName, String lName, int lVer, bool pSource, bool pSink) :
					GenericProcessor(pName), processorName(pName), libName(lName), libVersion(lVer), 
					processorSource(pSource), processorSink(pSink)
{

}

PlaceholderProcessor::~PlaceholderProcessor()
{

}

bool PlaceholderProcessor::hasEditor() const
{
	return true;
}

AudioProcessorEditor* PlaceholderProcessor::createEditor()
{
	editor = new PlaceholderProcessorEditor(this, processorName, libName, libVersion);
	return editor;
}

void PlaceholderProcessor::process(AudioSampleBuffer& continuousBuffer,	MidiBuffer& eventBuffer)
{

}

bool PlaceholderProcessor::isSource()
{
	return processorSource;
}
bool PlaceholderProcessor::isSink()
{
	return processorSink;
}

bool PlaceholderProcessor::isReady()
{
	CoreServices::sendStatusMessage("Cannot acquire with placeholder nodes");
	return false; //This processor never allows processing
}