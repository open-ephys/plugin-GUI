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

#ifndef SPIKERASTER_H_INCLUDED
#define SPIKERASTER_H_INCLUDED

#ifdef _WIN32
#include <Windows.h>
#endif

#include <ProcessorHeaders.h>
#include <SpikeLib.h>

class RasterPlot;

/**

  Displays a raster and peri-stimulus time histogram for incoming spikes.

  @see GenericProcessor

*/

class SpikeRaster : public GenericProcessor

{
public:

    /** The class constructor, used to initialize any members. */
    SpikeRaster();

    /** The class destructor, used to deallocate memory */
    ~SpikeRaster();

    /** Determines whether the processor is treated as a source. */
    bool isSource()
    {
        return false;
    }

    /** Determines whether the processor is treated as a sink. */
    bool isSink()
    {
        return false;
    }

    /** Indicates if the processor has a custom editor. Defaults to false */
    bool hasEditor() const
    {
        return true;
    }

    void updateSettings();

    int getNumElectrodes();

    AudioProcessorEditor* createEditor();

    void process(AudioSampleBuffer& buffer, MidiBuffer& events);

    void handleEvent(int, MidiMessage&, int);

    void setParameter(int parameterIndex, float newValue);

    bool enable();
    bool disable();

    void setRasterPlot(RasterPlot*);

private:

    struct Electrode
    {
        String name;

        int numChannels;

        Array<float> displayThresholds;
        Array<float> detectorThresholds;

        Array<SpikeObject> mostRecentSpikes;
        int currentSpikeIndex;

        int recordIndex;

    };

    RasterPlot* canvas;


    Array<Electrode> electrodes;

    int displayBufferSize;
    bool redrawRequested;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeRaster);

};

#endif  // SPIKERASTER_H_INCLUDED
