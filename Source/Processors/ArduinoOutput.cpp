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

#include "ArduinoOutput.h"

#include <stdio.h>
#include <unistd.h>  /*UNIX standard function definitions */
#include <termios.h> /*POSIX terminal control definitions */
#include <fcntl.h>   /*File control definitions */


ArduinoOutput::ArduinoOutput()
	: GenericProcessor("Arduino Output"), serialport("/dev/ttyACM0")
{

}

ArduinoOutput::~ArduinoOutput()
{

}

// AudioProcessorEditor* ArduinoOutput::createEditor()
// {
// 	editor = new ArduinoOutputEditor(this);
// 	return editor;
// }

void ArduinoOutput::handleEvent(int eventType, MidiMessage& event)
{
    if (eventType == TTL)
    {
    	std::cout << "Received event!" << std::endl;

    	const char byte = 0;
    	write(handle, &byte, 1);
        //startTimer((int) float(event.getTimeStamp())/getSampleRate()*1000.0);
    }
    
}

void ArduinoOutput::setParameter (int parameterIndex, float newValue)
{

}

bool ArduinoOutput::enable()
{
	struct termios toptions;
	int fd;

	handle = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);

	if (handle == -1)
	{
		std::cout << "Arduino Output unable to open port." << std::endl;
		return false;
	}

	if (tcgetattr(handle, &toptions) < 0)
	{
		std::cout << "Arduino Output couldn't get term attributes" << std::endl;
		return false;
	}

	speed_t brate = B9600;

	cfsetispeed(&toptions, brate);
	cfsetospeed(&toptions, brate);

	  // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    toptions.c_cflag &= ~CRTSCTS;

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw

    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 20;
    
    if( tcsetattr(handle, TCSANOW, &toptions) < 0) {
        std::cout << "Arduino Output couldn't set term attributes" << std::endl;
        return false;
    }
}

bool ArduinoOutput::disable()
{



}

void ArduinoOutput::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &events,
                            int& nSamples)
{
	

	checkForEvents(events);
	

}