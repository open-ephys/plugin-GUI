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
#include "FilterNode.h"
#include "FilterEditor.h"


FilterNode::FilterNode()
    : GenericProcessor  ("Bandpass Filter")
    , defaultLowCut     (300.0f)
    , defaultHighCut    (6000.0f)
{
    setProcessorType (PROCESSOR_TYPE_FILTER);

    // // Deprecated "parameters" class // //
    // Array<var> lowCutValues;
    // lowCutValues.add(1.0f);
    // lowCutValues.add(4.0f);
    // lowCutValues.add(100.0f);
    // lowCutValues.add(600.0f);

    // parameters.add(Parameter("low cut",lowCutValues, 3, 0));

    // Array<var> highCutValues;
    // highCutValues.add(12.0f);
    // highCutValues.add(3000.0f);
    // highCutValues.add(6000.0f);
    // highCutValues.add(9000.0f);

    // parameters.add(Parameter("high cut",highCutValues, 2, 1));
    applyOnADC = false;
}


FilterNode::~FilterNode()
{
}


AudioProcessorEditor* FilterNode::createEditor()
{
    editor = new FilterEditor (this, true);

    FilterEditor* ed = (FilterEditor*) getEditor();
    ed->setDefaults (defaultLowCut, defaultHighCut);

    return editor;
}

// ----------------------------------------------------
// From the filter library documentation:
// ----------------------------------------------------
//
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

// available methods:
//
// filter->getKind()
// filter->getName()
// filter->getNumParams()
// filter->getParamInfo()
// filter->getDefaultParams()
// filter->getParams()
// filter->getParam()

// filter->setParam()
// filter->findParamId()
// filter->setParamById()
// filter->setParams()
// filter->copyParamsFrom()

// filter->getPoleZeros()
// filter->response()
// filter->getNumChannels()
// filter->reset()
// filter->process()

void FilterNode::updateSettings()
{
    //int id = nodeId;
    int numInputs = getNumInputs();
    int numfilt = filters.size();
    if (numInputs < 1024 && numInputs != numfilt)
    {
        // SO fixed this. I think values were never restored correctly because you cleared lowCuts.
        Array<double> oldlowCuts;
        Array<double> oldhighCuts;
        oldlowCuts = lowCuts;
        oldhighCuts = highCuts;

        filters.clear();
        lowCuts.clear();
        highCuts.clear();
        shouldFilterChannel.clear();

        for (int n = 0; n < getNumInputs(); ++n)
        {
            filters.add (new Dsp::SmoothedFilterDesign
                         <Dsp::Butterworth::Design::BandPass    // design type
                         <2>,                                   // order
                         1,                                     // number of channels (must be const)
                         Dsp::DirectFormII> (1));               // realization


            //Parameter& p1 =  parameters.getReference(0);
            //p1.setValue(600.0f, n);
            //Parameter& p2 =  parameters.getReference(1);
            //p2.setValue(6000.0f, n);

            // restore defaults

            shouldFilterChannel.add (true);

            float newLowCut  = 0.f;
            float newHighCut = 0.f;

            if (oldlowCuts.size() > n)
            {
                newLowCut  = oldlowCuts[n];
                newHighCut = oldhighCuts[n];
            }
            else
            {
                newLowCut  = defaultLowCut;
                newHighCut = defaultHighCut;
            }

            lowCuts.add  (newLowCut);
            highCuts.add (newHighCut);

            setFilterParameters (newLowCut, newHighCut, n);
        }
    }

    setApplyOnADC (applyOnADC);
}


double FilterNode::getLowCutValueForChannel (int chan) const
{
    return lowCuts[chan];
}


double FilterNode::getHighCutValueForChannel (int chan) const
{
    return highCuts[chan];
}


bool FilterNode::getBypassStatusForChannel (int chan) const
{
    return shouldFilterChannel[chan];
}


void FilterNode::setFilterParameters (double lowCut, double highCut, int chan)
{
    if (dataChannelArray.size() - 1 < chan)
        return;

    Dsp::Params params;
    params[0] = dataChannelArray[chan]->getSampleRate(); // sample rate
    params[1] = 2;                          // order
    params[2] = (highCut + lowCut) / 2;     // center frequency
    params[3] = highCut - lowCut;           // bandwidth

    if (filters.size() > chan)
        filters[chan]->setParams (params);
}


void FilterNode::setParameter (int parameterIndex, float newValue)
{
    if (parameterIndex < 2) // change filter settings
    {
        if (newValue <= 0.01 || newValue >= 10000.0f)
            return;

        if (parameterIndex == 0)
        {
            lowCuts.set (currentChannel,newValue);
        }
        else if (parameterIndex == 1)
        {
            highCuts.set (currentChannel,newValue);
        }

        setFilterParameters (lowCuts[currentChannel],
                             highCuts[currentChannel],
                             currentChannel);

        editor->updateParameterButtons (parameterIndex);
    }
    // change channel bypass state
    else
    {
        if (newValue == 0)
        {
            shouldFilterChannel.set (currentChannel, false);
        }
        else
        {
            shouldFilterChannel.set (currentChannel, true);
        }
    }
}


void FilterNode::process (AudioSampleBuffer& buffer)
{
    for (int n = 0; n < getNumOutputs(); ++n)
    {
        if (shouldFilterChannel[n])
        {
            float* ptr = buffer.getWritePointer (n);
            filters[n]->process (getNumSamples (n), &ptr);
        }
    }
}


void FilterNode::setApplyOnADC (bool state)
{
    for (int n = 0; n < dataChannelArray.size(); ++n)
    {
        if (dataChannelArray[n]->getChannelType() == DataChannel::ADC_CHANNEL
            || dataChannelArray[n]->getChannelType() == DataChannel::AUX_CHANNEL)
        {
            setCurrentChannel (n);

            if (state)
                setParameter (2,1.0);
            else
                setParameter (2,0.0);
        }
    }
}


void FilterNode::saveCustomChannelParametersToXml(XmlElement* channelInfo, int channelNumber, InfoObjectCommon::InfoObjectType channelType)
{
    if (channelType == InfoObjectCommon::DATA_CHANNEL
        && channelNumber > -1
        && channelNumber < highCuts.size())
    {
        //std::cout << "Saving custom parameters for filter node." << std::endl;

        XmlElement* channelParams = channelInfo->createNewChildElement ("PARAMETERS");
        channelParams->setAttribute ("highcut",         highCuts[channelNumber]);
        channelParams->setAttribute ("lowcut",          lowCuts[channelNumber]);
        channelParams->setAttribute ("shouldFilter",    shouldFilterChannel[channelNumber]);
    }
}


void FilterNode::loadCustomChannelParametersFromXml(XmlElement* channelInfo, InfoObjectCommon::InfoObjectType channelType)
{
    int channelNum = channelInfo->getIntAttribute ("number");

    if (channelType == InfoObjectCommon::DATA_CHANNEL)
    {
        // restore high and low cut text in case they were changed by channelChanged
        static_cast<FilterEditor*>(getEditor())->resetToSavedText();

        forEachXmlChildElement (*channelInfo, subNode)
        {
            if (subNode->hasTagName ("PARAMETERS"))
            {
                highCuts.set (channelNum, subNode->getDoubleAttribute ("highcut", defaultHighCut));
                lowCuts.set  (channelNum, subNode->getDoubleAttribute ("lowcut",  defaultLowCut));
                shouldFilterChannel.set (channelNum, subNode->getBoolAttribute ("shouldFilter", true));

                setFilterParameters (lowCuts[channelNum], highCuts[channelNum], channelNum);
            }
        }
    }
}
