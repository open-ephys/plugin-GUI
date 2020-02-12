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

#ifndef EVENTQUEUE_H_INCLUDED
#define EVENTQUEUE_H_INCLUDED

#include <JuceHeader.h>
//#include <Events.h>
#include <vector>

template <class MsgContainer>
class AsyncEventMessage :
	public ReferenceCountedObject
{
public:
	AsyncEventMessage(const MsgContainer& m, int64 t, int extra) : 
		ReferenceCountedObject(),
		m_data(m),
		m_timestamp(t),
		m_extra(extra)
	{}

	~AsyncEventMessage() {};

	const MsgContainer& getData() const { return m_data;  }
	const int64& getTimestamp() const { return m_timestamp; }
	const int& getExtra() const { return m_extra;  }

private:
	const MsgContainer m_data;
	const int64 m_timestamp;
	const int m_extra;

};

template <class EventClass>
class EventQueue
{
public:
	typedef AsyncEventMessage<EventClass> EventContainer;
	typedef ReferenceCountedObjectPtr<EventContainer> EventClassPtr;

	EventQueue(int size) :
		m_fifo(size)
	{
		m_data.resize(size);
	}

	~EventQueue()
	{}

	int getRemainingEvents() const
	{
		return m_fifo.getNumReady();
	}

	void reset()
	{
		m_data.clear();
		m_data.resize(m_fifo.getTotalSize());
	}

	void resize(int size)
	{
		m_data.clear();
		m_fifo.setTotalSize(size);
		m_data.resize(size);
	}

	void addEvent(const EventClass& ev, int64 t, int extra = 0)
	{
		int pos1, size1, pos2, size2;
		size1 = 0;
		m_fifo.prepareToWrite(1, pos1, size1, pos2, size2);

		/* This means there is a buffer overrun. Instead of overwritting the existing data and risking a collision of both threads
			we just skip the incoming samples. TODO: use this to notify of the overrun and act consequently   */
		if (size1 > 0)
		{
			m_data[pos1] = new EventContainer(ev, t, extra);
			m_fifo.finishedWrite(1);
		}
	}

	int getEvents(std::vector<EventClassPtr>& vec, int max)
	{
		int pos1, size1, pos2, size2;
		int numAvailable = m_fifo.getNumReady();
		int numToRead = ((max < numAvailable) && (max > 0)) ? max : numAvailable;
		m_fifo.prepareToRead(numToRead, pos1, size1, pos2, size2);
		vec.resize(numToRead);
		for (int i = 0; i < size1; ++i)
		{
			vec[i] = m_data[pos1 + i];
		}
		if (size2 > 0)
		{
			for (int i = 0; i < size2; ++i)
			{
				vec[size1 + i] = m_data[pos2 + i];
			}
		}
		m_fifo.finishedRead(numToRead);
		return numToRead;
	}

private:
	std::vector<EventClassPtr> m_data;
	AbstractFifo m_fifo;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EventQueue);
};
//NOTE: Events are sent as midimessages while spikes as spike objects due to the difference on how they are passed to the record node.
//Once the probe system is implemented, this will be normalized
typedef EventQueue<MidiMessage> EventMsgQueue;
typedef EventQueue<SpikeEvent> SpikeMsgQueue;
typedef ReferenceCountedObjectPtr<AsyncEventMessage<MidiMessage>> EventMessagePtr;
typedef ReferenceCountedObjectPtr<AsyncEventMessage<SpikeEvent>> SpikeMessagePtr;

#endif  // EVENTQUEUE_H_INCLUDED

