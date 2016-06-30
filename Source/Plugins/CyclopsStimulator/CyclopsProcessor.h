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

#ifndef CYCLOPS_PROCESSOR_H_INCLUDED
#define CYCLOPS_PROCESSOR_H_INCLUDED

#ifdef _WIN32
#include <Windows.h>
#endif

#include <ProcessorHeaders.h>
#include "CyclopsEditor.h"
#include <SerialLib.h>

/**

  The Cyclops Stimulator controls Cyclops boards to perform optpgenetic
  stimulation.

  @see GenericProcessor

*/

class CyclopsProcessor : public GenericProcessor

{
public:

    /** The class constructor, used to initialize any members. */
    CyclopsProcessor();

    /** The class destructor, used to deallocate memory */
    ~CyclopsProcessor();

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

    bool hasEditor() const
    {
        return true;
    }

    /**
     * @brief      Filters only relevant serial ports (by name).
     *
     * @param[in]  portName  The port name
     *
     * @return     ``true`` if a Teensy or Arduino could be connected.
     */
    bool screenLikelyNames(const String& portName);

    /**
     Returns a list of all serial devices that are available on the system.

     The list of available devices changes whenever devices are connected or removed.
     */
    StringArray getDevices();

    /**
     Returns a list of all supported baudrates.
     */
    Array<int> getBaudrates();

    /** Setter, that allows you to set the serial device that will be used during acquisition */
    void setDevice(string device);

    /** Setter, that allows you to set the baudrate that will be used during acquisition */
    void setBaudrate(int baudrate);

    AudioProcessorEditor* createEditor();

    /**
     This should only be run by the ProcessorGraph, before acquisition will be started.
     It tries to open the serial port previsouly specified by the setDevice and setBaudrate setters.
     Returns true on success, false if port could not be opened.
     */
    bool isReady();

    /** Defines the functionality of the processor.

        The process method is called every time a new data buffer is available.

        Processors can either use this method to add new data, manipulate existing
        data, or send data to an external target (such as a display or other hardware).

        Continuous signals arrive in the "buffer" variable, event data (such as TTLs
        and spikes) is contained in the "events" variable.
         */
    void process(AudioSampleBuffer& buffer, MidiBuffer& events);

    /** The method that standard controls on the editor will call.
        It is recommended that any variables used by the "process" function 
        are modified only through this method while data acquisition is active. */
    void setParameter(int parameterIndex, float newValue);

    /** Optional method called every time the signal chain is refreshed or changed in any way.
        
        Allows the processor to handle variations in the channel configuration or any other parameter
        passed down the signal chain. The processor can also modify here the settings structure, which contains
        information regarding the input and output channels as well as other signal related parameters. Said
        structure shouldn't be manipulated outside of this method.
    */
    void updateSettings();

    virtual bool enable();
    virtual bool disable();

private:
    static ofSerial Serial;
    static string port;
    static int baud_rate;
    static const int BAUDRATES[12];

    static int node_count;
    static int board_count;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CyclopsProcessor);

};

#endif  // CYCLOPS_PROCESSOR_H_INCLUDED
