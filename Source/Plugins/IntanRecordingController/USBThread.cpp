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


#include "USBThread.h"
#include "rhythm-api/rhd2000evalboardusb3.h"

using namespace IntanRecordingController;

USBThread::USBThread(Rhd2000EvalBoardUsb3* b)
	: m_board(b), Thread("USBThread")
{
}


USBThread::~USBThread()
{
}

void USBThread::startAcquisition(int nBytes)
{
    ScopedLock lock(m_lock);
	for (int i = 0; i < 2; i++)
	{
		m_lastRead[i] = 0;
		m_buffers[i].malloc(nBytes);
	}
	m_curBuffer = 0;
	m_readBuffer = 0;
	m_canRead = true;
	startThread();
}

void USBThread::stopAcquisition()
{
	std::cout << "Stopping usb thread" << std::endl;
	if (isThreadRunning())
	{
		if (!stopThread(1000))
		{
			std::cerr << "USB Thread could not stop cleanly. Force quitting it" << std::endl;
		}
	}
}

long USBThread::usbRead(unsigned char*& buffer)
{
	ScopedLock lock(m_lock);
	if (m_readBuffer == m_curBuffer)
		return 0;
	buffer = m_buffers[m_readBuffer].getData();
	long read = m_lastRead[m_readBuffer];
	m_readBuffer = ++m_readBuffer % 2;
	m_canRead = true;
	notify();
	return read;
}

void USBThread::run()
{
	while (!threadShouldExit())
	{
		m_lock.enter();
		if (m_canRead)
		{
			m_lock.exit();
			long read;
			do
			{
				if (threadShouldExit())
					break;
				read = m_board->readDataBlocksRaw(1, m_buffers[m_curBuffer].getData());
			} while (read <= 0);
			{
				ScopedLock lock(m_lock);
				m_lastRead[m_curBuffer] = read;
				m_curBuffer = ++m_curBuffer % 2;
				m_canRead = false;
			}
		}
		else
			m_lock.exit();

		if (!threadShouldExit())
			wait(100);
	}
}
