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

#include "ProcessorInfo.h"

#include "../GenericProcessor/GenericProcessor.h"

ProcessorInfoObject::ProcessorInfoObject(GenericProcessor* processor) 
	: InfoObject(InfoObject::Type::PROCESSOR_INFO)
{
	m_nodeId = processor->getNodeId();
	m_name = processor->getName();

	if (processor->isSource())
		m_type = "Source";
	else if (processor->isFilter())
		m_type = "Filter";
	else if (processor->isSink())
		m_type = "Sink";
	else if (processor->isRecordNode())
		m_type = "RecordNode";
	
}

ProcessorInfoObject::~ProcessorInfoObject()
{

}

int ProcessorInfoObject::getNodeId() const
{
	return m_nodeId;
}

String ProcessorInfoObject::getType() const
{
	return m_type;
}

String ProcessorInfoObject::getName() const
{
	return m_name;
}
