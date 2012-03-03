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
#include "FilterNode.h"
#include "Editors/FilterEditor.h"

FilterNode::FilterNode()
	: GenericProcessor("Bandpass Filter"), filter(0),
	  highCut(6000.0), lowCut(1.0)
	
{
	//setNumInputs(16);
	//setSampleRate(25000.0);
	// set up default configuration
	//setPlayConfigDetails(16, 16, 44100.0, 128);

		// each family of filters is given its own namespace
		// RBJ: filters from the RBJ cookbook
		// Butterworth
		// ChebyshevI: ripple in the passband
		// ChebyshevII: ripple in the stop band
		// Elliptic: ripple in both the passband and stopband
		// Bessel: theoretically with linear phase
		// Legendre: "Optimum-L" filters with steepest transition and monotonic passband
		// Custom: Simple filters that allow poles and zeros to be specified directly

		// within each namespace exists a set of "raw filters"
		// Butterworth::LowPass
		//				HighPass
		// 				BandPass
		//				BandStop
		//				LowShelf
		// 				HighShelf
		//				BandShelf
		//
		//	class templates (such as SimpleFilter) which require FilterClasses
		//    expect an identifier of a raw filter
		//  raw filters do not support introspection, or the Params style of changing
		//    filter settings; they only offer a setup() function for updating the IIR
		//    coefficients to a given set of parameters
		//

		// each filter family namespace also has the nested namespace "Design"
		// here we have all of the raw filter names repeated, except these classes
		//  also provide the Design interface, which adds introspection, polymorphism,
		//  the Params style of changing filter settings, and in general all fo the features
		//  necessary to interoperate with the Filter virtual base class and its derived classes


	


}

FilterNode::~FilterNode()
{
	filter = 0;
}

AudioProcessorEditor* FilterNode::createEditor()
{
	FilterEditor* filterEditor = new FilterEditor(this);
	setEditor(filterEditor);
	
	std::cout << "Creating editor." << std::endl;

	return filterEditor;
}

void FilterNode::setNumInputs(int inputs)
{		

	numInputs = inputs;
	setNumOutputs(inputs);

	if (filter != 0)
	{
		delete filter;
		filter = 0;
	}

	const int nChans = inputs;

	if (nChans == 16) {

	filter = new Dsp::SmoothedFilterDesign 
			<Dsp::Butterworth::Design::BandPass 	// design type
			<4>,								 	// order
			16,										// number of channels (must be const)
			Dsp::DirectFormII>						// realization
			(1024);									// number of samples over which to fade 
		
	} else if (nChans == 64) {
	
	filter = new Dsp::SmoothedFilterDesign 
			<Dsp::Butterworth::Design::BandPass 	// design type
			<4>,								 	// order
			64	,									// number of channels (must be const)
			Dsp::DirectFormII>						// realization
			(1024);									// number of samples over which to fade 
													//   parameter changes

	} else if (nChans == 10) {
		
		filter = new Dsp::SmoothedFilterDesign 
			<Dsp::Butterworth::Design::BandPass 	// design type
			<4>,								 	// order
			10	,									// number of channels (must be const)
			Dsp::DirectFormII>						// realization
			(1024);									// number of samples over which to fade 
													//   parameter changes
	} else {
		// send a message saying this is not implemented

	}									

	//std::cout << "Filter created with " << getNumInputs() << " channels." << std::endl;

	setFilterParameters();

}

//AudioProcessorEditor* FilterNode::createEditor(AudioProcessorEditor* const editor)
//{
	
//	return editor;
//}

void FilterNode::setSampleRate(float r)
{
	sampleRate = r;
	setFilterParameters();
}

void FilterNode::setFilterParameters()
{

	Dsp::Params params;
	params[0] = getSampleRate(); // sample rate
	params[1] = 4; // order
	params[2] = (highCut + lowCut)/2; // center frequency
	params[3] = highCut - lowCut; // bandwidth

	if (filter != 0)
		filter->setParams (params);

}

void FilterNode::setParameter (int parameterIndex, float newValue)
{
	//std::cout << "Message received." << std::endl;

	if (parameterIndex == 0) {
		lowCut = newValue;
	} else {
		highCut = newValue;
	}

	setFilterParameters();

}

void FilterNode::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &midiMessages,
                            int& nSamples)
{
	//std::cout << "Filter node processing." << std::endl;
	//std::cout << buffer.getNumChannels() << std::endl;
	//::cout << buffer.getNumSamples() << std::endl;

	//int nSamps = getNumSamples(midiMessages);
	//std::cout << nSamples << std::endl;
    filter->process (nSamples, buffer.getArrayOfChannels());

    //std::cout << "Filter node:" << *buffer.getSampleData(0,0);


    //int ts = checkForMidiEvents(midiMessages);

    //std::cout << "Timestamp = " << ts << std::endl;
}
