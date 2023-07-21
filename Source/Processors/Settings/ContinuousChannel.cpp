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

#include "ContinuousChannel.h"
#include "DataStream.h"

ContinuousChannel::ContinuousChannel(ContinuousChannel::Settings settings) :
	ChannelInfoObject(InfoObject::Type::CONTINUOUS_CHANNEL, settings.stream),
	ParameterOwner(ParameterOwner::Type::CONTINUOUS_CHANNEL),
	m_type(settings.type)
{
	setName(settings.name);
	setDescription(settings.description);
	setBitVolts(settings.bitVolts);
}


ContinuousChannel::~ContinuousChannel()
{
}

void ContinuousChannel::setBitVolts(float bitVolts)
{
	m_bitVolts = bitVolts;
}

float ContinuousChannel::getBitVolts() const
{
	return m_bitVolts;
}

void ContinuousChannel::setUnits(String unit)
{
	m_units = unit;
}

String ContinuousChannel::getUnits() const
{
	return m_units;
}

const ContinuousChannel::Type ContinuousChannel::getChannelType() const
{
	return m_type;
}


/*bool DataChannel::checkEqual(const InfoObjectCommon& other, bool similar) const
{
	const DataChannel& o = dynamic_cast<const DataChannel&>(other);
	if (m_bitVolts != o.m_bitVolts) return false;
	if (!similar && m_unitName != o.m_unitName) return false;
	if (similar)
		return hasSimilarMetadata(o);
	else return hasSameMetadata(o);
}*/
