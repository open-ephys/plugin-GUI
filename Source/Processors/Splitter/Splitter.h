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

#ifndef __SPLITTER_H_A75239F7__
#define __SPLITTER_H_A75239F7__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"

#include <stdio.h>


/**

  Allows the user to split the signal chain.

  This processor doesn't modify the data passing through it. In fact,
  it has no incoming or outgoing connections. It just allows the outputs from
  its source node to be connected to TWO destination nodes.

  @see GenericProcessor, ProcessorGraph

*/


class Splitter : public GenericProcessor
{
public:

    Splitter();
    ~Splitter();

    AudioProcessorEditor* createEditor();

    /** Nothing happens here, because Splitters are not part of the ProcessorGraph. */
    void process(AudioSampleBuffer& buffer) override {}

    bool isSplitter()
    {
        return true;
    }

    void switchIO(int);
    void switchIO();
    void setSplitterDestNode(GenericProcessor* dn);
    GenericProcessor* getDestNode(int);

    void setPathToProcessor(GenericProcessor* processor);

    int getPath();

private:

    GenericProcessor* destNodeA;
    GenericProcessor* destNodeB;
    int activePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Splitter);

};


#endif  // __SPLITTER_H_A75239F7__
