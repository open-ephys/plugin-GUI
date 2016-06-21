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


#include <stdio.h>
#include <iostream>
#include "TestPlugin0706sinkProcessor.h"
#include "TestPlugin0706sinkProcessorEditor.h"

//If the processor uses a custom editor, it needs its header to instantiate it
//#include "ExampleEditor.h"


TestPlugin0706sinkProcessor::TestPlugin0706sinkProcessor()
    : GenericProcessor ("TestPlugin0706-Sink") //, threshold(200.0), state(true)

{
    setProcessorType (PROCESSOR_TYPE_FILTER);
    //Without a custom editor, generic parameter controls can be added
    //auto parameter2 = new Parameter ("threshold", 0.0, 500.0, 200.0, 1);
    //parameter2->setEditorDesiredBounds (70, 15, 50, 20);
    //parameters.add (parameter2);

    //auto parameter1 = new Parameter("threshold22", 0.0, 500.0, 200.0, 0);
    //parameter1->setEditorDesiredBounds (0, 0, 200, 70);
    //parameters.add (parameter1);

    //parameters.add (new Parameter("threshold22", 0.0, 500.0, 200.0, 0));

    //// BOOLEANS
    //auto boolParam = new Parameter ("boolean", false, 0);
    //boolParam->setEditorDesiredBounds (0, 0, 100, 30);
    //parameters.add (boolParam);

    //auto boolParam2 = new Parameter ("boolean2", false, 0);
    //boolParam2->setEditorDesiredBounds (0, 20, 50, 15);
    //parameters.add (boolParam2);

    //auto boolParam3 = new Parameter ("boolean3", false, 0);
    //boolParam3->setEditorDesiredBounds (0, 40, 100, 20);
    //parameters.add (boolParam3);

    //auto boolParam4 = new Parameter ("boolean4", false, 0);
    //boolParam4->setEditorDesiredBounds (0, 60, 100, 60);
    //parameters.add (boolParam4);

    //parameters.add(new Parameter ("boolean", false, 0));
    //parameters.add(new Parameter ("gdfgdf", false, 1));
    //

    // DISCRETE
    Array<var> someValues;
    someValues.add (1);
    someValues.add (3);
    someValues.add (26);

    auto discreteParam1 = new Parameter ("discrete", someValues, 2, 0);
    parameters.add (discreteParam1);

    auto discreteParam2 = new Parameter ("discretecopy", someValues, 2, 0);
    discreteParam2->setEditorDesiredBounds (20, 50, discreteParam2->getEditorRecommendedWidth(), discreteParam2->getEditorRecommendedHeight());
    parameters.add (discreteParam2);

    //auto discreteParam3 = new Parameter ("discretecopy2", someValues, 2, 0);
    //discreteParam3->setEditorDesiredBounds (20, 50, 50, 20);
    //parameters.add (discreteParam3);
}


TestPlugin0706sinkProcessor::~TestPlugin0706sinkProcessor()
{
}


/**
  If the processor uses a custom editor, this method must be present.
*/
AudioProcessorEditor* TestPlugin0706sinkProcessor::createEditor()
{
    editor = new TestPlugin0706sinkProcessorEditor (this, true);

    //std::cout << "Creating editor." << std::endl;

    return editor;
}


void TestPlugin0706sinkProcessor::setParameter (int parameterIndex, float newValue)
{
    //Parameter& p =  parameters.getReference(parameterIndex);
    //p.setValue(newValue, 0);

    //threshold = newValue;

    //std::cout << float(p[0]) << std::endl;
    editor->updateParameterButtons (parameterIndex);
}


void TestPlugin0706sinkProcessor::process (AudioSampleBuffer& buffer, MidiBuffer& events)
{
    /**
      Generic structure for processing buffer data
    */
    int nChannels = buffer.getNumChannels();
    for (int chan = 0; chan < nChannels; ++chan)
    {
        int nSamples = getNumSamples (chan);
        /* =============================================================================
          Do something here.

          To obtain a read-only pointer to the n sample of a channel:
          float* samplePtr = buffer.getReadPointer(chan,n);

          To obtain a read-write pointer to the n sample of a channel:
          float* samplePtr = buffer.getWritePointer(chan,n);

          All the samples in a channel are consecutive, so this example is valid:
          float* samplePtr = buffer.getWritePointer(chan,0);
          for (i=0; i < nSamples; i++)
          {
         *(samplePtr+i) = (*samplePtr+i)+offset;
         }

         See also documentation and examples for buffer.copyFrom and buffer.addFrom to operate on entire channels at once.

         To add a TTL event generated on the n-th sample:
         addEvents(events, TTL, n);
         =============================================================================== */
    }

    /** Simple example that creates an event when the first channel goes under a negative threshold

      for (int i = 0; i < getNumSamples(channels[0]->sourceNodeId); i++)
      {
      if ((*buffer.getReadPointer(0, i) < -threshold) && !state)
      {

    // generate midi event
    addEvent(events, TTL, i);

    state = true;

    } else if ((*buffer.getReadPointer(0, i) > -threshold + bufferZone)  && state)
    {
    state = false;
    }

    }
    */
}
