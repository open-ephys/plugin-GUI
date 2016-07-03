
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

#include "CyclopsProcessor.h"

ofSerial  CyclopsProcessor::Serial;
string    CyclopsProcessor::port = "";
int       CyclopsProcessor::baud_rate = 9600;
const int CyclopsProcessor::BAUDRATES[12] = {300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 230400};

int       CyclopsProcessor::node_count = 0;
int       CyclopsProcessor::board_count = 0;

CyclopsProcessor::CyclopsProcessor()
    : GenericProcessor("Cyclops Stimulator")
{
    node_count++;
}

CyclopsProcessor::~CyclopsProcessor()
{
  if (--node_count == 0)
    Serial.close();
}

bool CyclopsProcessor::screenLikelyNames(const String& portName)
{
    #ifdef TARGET_OSX
        return portName.contains("cu.") || portName.contains("tty.");
    #endif
    #ifdef TARGET_LINUX
        return portName.contains("ttyUSB") || portName.contains("ttyA");
    #endif
    return true; // for TARGET_WIN32
}

StringArray CyclopsProcessor::getDevices()
{
    vector<ofSerialDeviceInfo> allDeviceInfos = Serial.getDeviceList();
    StringArray allDevices;
    String portName;
    for (int i = 0; i < allDeviceInfos.size(); i++)
    {
        portName = allDeviceInfos[i].getDeviceName();
        if (screenLikelyNames(portName))
        {
            allDevices.add(portName);
        }
    }
    return allDevices;
}

Array<int> CyclopsProcessor::getBaudrates()
{
    Array<int> allBaudrates(BAUDRATES, 12);
    return allBaudrates;
}

void CyclopsProcessor::setDevice(string port)
{
    this->port = port;
}

void CyclopsProcessor::setBaudrate(int baudrate)
{
    this->baud_rate = baudrate;
}

void CyclopsProcessor::updateSettings()
{
    ;
}


/**
    If the processor uses a custom editor, this method must be present.
*/
AudioProcessorEditor* CyclopsProcessor::createEditor()
{
    editor = new CyclopsEditor(this, true);
    //std::cout << "Creating editor." << std::endl;
    return editor;
}

void CyclopsProcessor::setParameter(int parameterIndex, float newValue)
{

    //Parameter& p =  parameters.getReference(parameterIndex);
    //p.setValue(newValue, 0);
    //threshold = newValue;
    //std::cout << float(p[0]) << std::endl;
    editor->updateParameterButtons(parameterIndex);
}

void CyclopsProcessor::process(AudioSampleBuffer& buffer,
                               MidiBuffer& events)
{
    checkForEvents(events);
}

void handleEvent(int eventType, MidiMessage& event, int samplePosition/* = 0 */)
{
    ;    
}

bool CyclopsProcessor::isReady()
{
    if (port == "" || baud_rate == 0)
    {
        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Serial Port connection error!", "Please set port and baudrate to use first!");
        return false;
    }
    if (!Serial.setup(port, baud_rate))
    {
        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Serial Port connection error!", "Could not connect to specified serial port. Check log files for details.");
        return false;
    }
    return true;
}

bool CyclopsProcessor::enable()
{
    //Serial.close();
    return true;
}

bool CyclopsProcessor::disable()
{
    //Serial.close();
    return true;
}