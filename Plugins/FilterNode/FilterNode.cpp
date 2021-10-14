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

BandpassFilterSettings::BandpassFilterSettings()
{
    lowCut = 300.0f;
    highCut = 6000.0f;

    isEnabled = true;
}

void BandpassFilterSettings::createFilters(int numChannels, float sampleRate_)
{

    sampleRate = sampleRate_;

    filters.clear();

    for (int n = 0; n < numChannels; ++n)
    {
        filters.add(new Dsp::SmoothedFilterDesign
            <Dsp::Butterworth::Design::BandPass    // design type
            <2>,                                   // order
            1,                                     // number of channels (must be const)
            Dsp::DirectFormII>(1));               // realization

        if (channelMask.size() < n + 1)
            channelMask.add(true);

        setFilterParameters(lowCut, highCut, n);
    }

    std::cout << "Channel mask size: " << channelMask.size() << std::endl;

}

void BandpassFilterSettings::updateFilters()
{
    for (int n = 0; n < filters.size(); n++)
    {
        setFilterParameters(lowCut, highCut, n);
    }
}

void BandpassFilterSettings::setFilterParameters(double lowCut, double highCut, int channel)
{
    Dsp::Params params;
    params[0] = sampleRate;                 // sample rate
    params[1] = 2;                          // order
    params[2] = (highCut + lowCut) / 2;     // center frequency
    params[3] = highCut - lowCut;           // bandwidth

    filters[channel]->setParams(params);
}

void BandpassFilterSettings::toXml(XmlElement* xml)
{
    xml->setAttribute("lowCut", lowCut);
    xml->setAttribute("highCut", highCut);
    xml->setAttribute("isEnabled", isEnabled);
    xml->setAttribute("maskChannels", channelMaskToString(channelMask));
}

void BandpassFilterSettings::fromXml(XmlElement* xml)
{
    lowCut = xml->getDoubleAttribute("lowCut", lowCut);
    highCut = xml->getDoubleAttribute("highCut", highCut);
    isEnabled = xml->getBoolAttribute("isEnabled", isEnabled);
    channelMaskFromString(xml->getStringAttribute("maskChannels", ""));
}

String BandpassFilterSettings::channelMaskToString(Array<bool> channelMask)
{

    std::cout << " Channel mask to string..." << std::endl;
    std::cout << " Channel mask size: " << channelMask.size() << std::endl;

    String result = "";

    for (int i = 0; i < channelMask.size(); i++)
    {
        if (!channelMask[i])
            result += String(i + 1) + ",";
    }

    return result.substring(0, result.length()-1);
}

void BandpassFilterSettings::channelMaskFromString(String input)
{
    std::cout << "maskChannels input: " << input << std::endl;

    StringArray channels = StringArray::fromTokens(input, ",", "");

    std::cout << "Found " << channels.size() << " tokens" << std::endl;

    for (int i = 0; i < channels.size(); i++)
    {
        std::cout << channels[i] << std::endl;
        int ch = channels[i].getIntValue() - 1;

        if (ch < channelMask.size())
            channelMask.set(ch, false);
    }

}

FilterNode::FilterNode()
    : GenericProcessor  ("Bandpass Filter")
{
    setProcessorType (PROCESSOR_TYPE_FILTER);

}


FilterNode::~FilterNode()
{
}


AudioProcessorEditor* FilterNode::createEditor()
{
    editor = std::make_unique<FilterEditor> (this, true);

    FilterEditor* ed = (FilterEditor*) getEditor();
    ed->setDefaults (300.0f, 6000.0f);

    return editor.get();
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

    settings.update(getDataStreams());

    for (auto stream : getDataStreams())
    {
        settings[stream->getStreamId()]->createFilters(stream->getChannelCount(), stream->getSampleRate());
    }
}


double FilterNode::getLowCutValue(uint16 streamId) 
{
    return settings[streamId]->lowCut;
}

void FilterNode::setLowCutValue(uint16 streamId, double value)
{
    currentStream = streamId;

    setParameter(0, value);
}

double FilterNode::getHighCutValue(uint16 streamId)
{
    return settings[streamId]->highCut;
}

void FilterNode::setHighCutValue(uint16 streamId, double value)
{
    currentStream = streamId;

    setParameter(1, value);
}

void FilterNode::setEnabledState(uint16 streamId, bool isEnabled)
{
    currentStream = streamId;

    if (isEnabled)
        setParameter(2, 1.0f);
    else
        setParameter(2, 0.0f);
}

bool FilterNode::getEnabledState(uint16 streamId)
{
    return settings[streamId]->isEnabled;
}

void FilterNode::setChannelMask(uint16 streamId, Array<int> channels)
{
    currentStream = streamId;

    for (int i = 0; i < getDataStream(currentStream)->getChannelCount(); i++)
    {
        currentChannel = i;

        if (channels.contains(i))
            setParameter(3, 1.0f);
        else
            setParameter(3, 0.0f);
    }
}

Array<bool> FilterNode::getChannelMask(uint16 streamId)
{
    return settings[streamId]->channelMask;
}



void FilterNode::setParameter (int parameterIndex, float newValue)
{
    if (parameterIndex == 0) // change low cut for current stream
    {
        if (newValue <= 0.01 || newValue >= 10000.0f)
            return;

        settings[currentStream]->lowCut = newValue;

        settings[currentStream]->updateFilters();
    }
    else if (parameterIndex == 1) // change high cut for current stream
    {
        if (newValue <= 0.01 || newValue >= 10000.0f)
            return;

        settings[currentStream]->highCut = newValue;

        settings[currentStream]->updateFilters();

    }
    else if (parameterIndex == 2) // filter is enabled

    {
        if (newValue == 1.0f)
            settings[currentStream]->isEnabled = true;
        else
            settings[currentStream]->isEnabled = false;
    }
    else     // change channel bypass state
    {
        if (newValue == 0)
        {
            settings[currentStream]->channelMask.set(currentChannel, false);
        }
        else
        {
            settings[currentStream]->channelMask.set(currentChannel, true);
        }
    }
}


void FilterNode::process (AudioSampleBuffer& buffer)
{
    for (auto stream : getDataStreams())
    {
        BandpassFilterSettings* settings_ = settings[stream->getStreamId()];

        for (int n = 0; n < stream->getChannelCount(); ++n)
        {
            if (settings_->channelMask[n])
            {
                int globalIndex = stream->getContinuousChannels()[n]->getGlobalIndex();

                float* ptr = buffer.getWritePointer(globalIndex);

                settings_->filters[n]->process(getNumSamples(globalIndex), &ptr);
            }
        }
    }

    
}


void FilterNode::saveCustomParametersToXml(XmlElement* xml)
{
    for (auto stream : getDataStreams())
    {

        XmlElement* streamParams = xml->createNewChildElement ("STREAM");

        settings[stream->getStreamId()]->toXml(streamParams);
    }
}


void FilterNode::loadCustomParametersFromXml()
{

    std::cout << "Filter node loading custom parameters" << std::endl;
    int streamIndex = 0;
    Array<const DataStream*> availableStreams = getDataStreams();

    forEachXmlChildElement (*parametersAsXml, streamParams)
    {
        if (streamParams->hasTagName ("STREAM"))
        {

            std::cout << "STREAM " << streamIndex << std::endl;
            if (availableStreams.size() > streamIndex)
            {
                std::cout << "FOUND IT!" << std::endl;
                settings[availableStreams[streamIndex]->getStreamId()]->fromXml(streamParams);
            }
            else {
                std::cout << "DID NOT FIND IT!" << std::endl;
            }

            streamIndex++;
        }
    }

}
