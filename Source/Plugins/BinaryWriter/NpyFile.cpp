/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2017 Open Ephys

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

#include "NpyFile.h"

using namespace BinaryRecordingEngine;

NpyFile::NpyFile(String path, const Array<NpyType>& typeList)
{
	File file(path);
	Result res = file.create();
	if (res.failed())
	{
		std::cerr << "Error creating file " << path << ":" << res.getErrorMessage() << std::endl;
		return;
	}
	m_file = file.createOutputStream();
	if (!m_file)
		return;

	m_okOpen = true;

	String header = "{'descr': [";

	int nTypes = typeList.size();
	String name;

	for (int i = 0; i < nTypes; i++)
	{
		NpyType& type = typeList.getReference(i);
		if (i > 0) header += ", ";
		header += "('" + type.getName() + "', '" + type.getTypeString() + "', (" + String(type.getTypeLength()) + ",))";
	}
	header += "], 'fortran_order': False, 'shape': (1,), }";

	int headLength = header.length();
	int padding = (int((headLength + 30 ) / 16) + 1) * 16;
	header = header.paddedRight(' ', padding);
	header += '\n';
	
	uint8 magicNum = 0x093;
	m_file->write(&magicNum, sizeof(uint8));
	String magic = "NUMPY";
	uint16 len = header.length();
	m_file->write(magic.toUTF8(), magic.getNumBytesAsUTF8());
	uint16 ver = 0x0001;
	m_file->write(&ver, sizeof(uint16));
	m_file->write(&len, sizeof(uint16));
	m_file->write(header.toUTF8(), len);
	m_countPos = headLength + 4; //10-6
	
}

NpyFile::~NpyFile()
{
	if (m_file->setPosition(m_countPos))
	{
		String newShape = String(m_recordCount) + ",), }";
		m_file->write(newShape.toUTF8(), newShape.getNumBytesAsUTF8());
	}
	else
	{
		std::cerr << "Error. Unable to seek to update header on file " << m_file->getFile().getFullPathName() << std::endl;
	}
}

void NpyFile::writeData(const void* data, size_t size)
{
	m_file->write(data, size);
}

void NpyFile::increaseRecordCount(int count)
{
	m_recordCount += count;
}


NpyType::NpyType(String n, BaseType t, size_t l)
	: name(n), type(t), length(l)
{
}

String NpyType::getTypeString() const
{
	switch (type)
	{
	case BaseType::CHAR:
		return "U" + String(length);
	case BaseType::INT8:
		return "<i1";
	case BaseType::UINT8:
		return "<u1";
	case BaseType::INT16:
		return "<i2";
	case BaseType::UINT16:
		return "<u2";
	case BaseType::INT32:
		return "<i4";
	case BaseType::UINT32:
		return "<u4";
	case BaseType::INT64:
		return "<i8";
	case BaseType::UINT64:
		return "<u8";
	case BaseType::FLOAT:
		return "<f4";
	case BaseType::DOUBLE:
		return "<f8";
	default:
		return "<b1";
	}
}

int NpyType::getTypeLength() const
{
	if (type == BaseType::CHAR)
		return 1;
	else
		return length;
}

String NpyType::getName() const
{
	return name;
}