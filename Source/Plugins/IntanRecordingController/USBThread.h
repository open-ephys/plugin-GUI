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

#ifndef USBTHREAD_H
#define USBTHREAD_H

#include <BasicJuceHeader.h>
#include <atomic>
class Rhd2000EvalBoardUsb3;

class USBThread : Thread
{
public:
	USBThread(Rhd2000EvalBoardUsb3*);
	~USBThread();
	void run() override;
	void startAcquisition(int nBytes);
	void stopAcquisition();
	long usbRead(unsigned char*&);
private:
	Rhd2000EvalBoardUsb3* const m_board;
	HeapBlock<unsigned char> m_buffers[2];
	long m_lastRead[2];
	unsigned short m_curBuffer{ 0 };
	unsigned short m_readBuffer{ 0 };
	bool m_canRead{ false };
	CriticalSection m_lock;
};

#endif
