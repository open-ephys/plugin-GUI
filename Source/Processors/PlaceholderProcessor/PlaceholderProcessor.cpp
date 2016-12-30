/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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


PlaceholderProcessor::PlaceholderProcessor (String pName, String lName, int lVer, bool pSource, bool pSink) 
    : GenericProcessor      (pName)
    , m_processorName       (pName)
    , m_libName             (lName)
    , m_libVersion          (lVer)
    , m_isSourceProcessor   (pSource)
    , m_isSinkProcessor     (pSink)
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
    editor = new PlaceholderProcessorEditor (this, m_processorName, m_libName, m_libVersion);
    return editor;
}


void PlaceholderProcessor::process (AudioSampleBuffer& continuousBuffer)
{
}


bool PlaceholderProcessor::isSource() const
{
    return m_isSourceProcessor;
}


bool PlaceholderProcessor::isSink() const
{
    return m_isSinkProcessor;
}


bool PlaceholderProcessor::isReady()
{
    CoreServices::sendStatusMessage ("Cannot acquire with placeholder nodes");
    return false; //This processor never allows processing
}
