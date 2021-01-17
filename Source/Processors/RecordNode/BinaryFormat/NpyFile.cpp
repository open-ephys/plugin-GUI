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

Specification of the .npy file format is at:

http://www.numpy.org/neps/nep-0001-npy-format.html

Python implementation is at:

https://github.com/numpy/numpy/blob/master/numpy/lib/format.py

*/

#include "NpyFile.h"

NpyFile::NpyFile(String path, const Array<NpyType>& typeList)
{
    m_dim1 = 1;
    m_dim2 = 1;

    /*If there is only one element on the list but is
    an array, make this a multidimensional file.
    */
    if (typeList.size() == 1)
    {
        NpyType type = typeList[0];
        if (type.getType() != BaseType::CHAR) //strings work different
            m_dim1 = type.getTypeLength();
    }

    if (!openFile(path))
        return;
    writeHeader(typeList);
}

NpyFile::NpyFile(String path, NpyType type, unsigned int dim)
{
    if (!openFile(path))
        return;

    Array<NpyType> typeList;
    typeList.add(type);
    m_dim1 = dim;
    m_dim2 = type.getTypeLength();
    writeHeader(typeList);
}

bool NpyFile::openFile(String path)
{
    File file(path);
    Result res = file.create();
    if (res.failed())
    {
        std::cerr << "Error creating file " << path << ":" << res.getErrorMessage() << std::endl;
        file.deleteFile();
        Result res = file.create();
        LOGD("Re-creating file: ", path);
    }
    
    //file.deleteFile(); // overwrite, never append a new .npy file to end of an existing one
    // output stream buffer size defaults to 32768 bytes, but is irrelevant because
    // each updateHeader() call triggers a m_file->flush() to disk:
    m_file = file.createOutputStream();

    /*
    if (m_file == nullptr)
    {
        LOGD("FAILED to open file @", path);
    }
    else
    {
        String pad = "";
        for (int i = 0; i < 162 - path.length(); i++)
            pad += " ";
        LOGD("Successfully opened file @", path, pad, m_file);
    }
    */
    
    if (!m_file)
        return false;

    m_okOpen = true;
    return true;
}

String NpyFile::getShapeString()
{
    String shape;
    shape.preallocateBytes(32);
    shape = "(";
    shape += String(m_recordCount) + ",";
    if (m_dim1 > 1)
    {
        shape += " " + String(m_dim1) + ",";
    }
    if (m_dim2 > 1)
        shape += " " + String(m_dim2);
    shape += "), }";
    return shape;
}

void NpyFile::writeHeader(const Array<NpyType>& typeList)
{
    uint8 magicNum = 0x93;
    String magicStr = "NUMPY";
    uint16 ver = 0x0001;
    // magic = magic number + magic string + magic version
    int magicLen = sizeof(uint8) + magicStr.getNumBytesAsUTF8() + sizeof(uint16);
    int nbytesAlign = 64; // header should use an integer multiple of this many bytes

    bool multiValue = typeList.size() > 1;
    String strHeader;
    strHeader.preallocateBytes(128);
    strHeader = "{'descr': ";

    if (multiValue)
        strHeader += "[";

    int nTypes = typeList.size();

    for (int i = 0; i < nTypes; i++)
    {
        NpyType& type = typeList.getReference(i);
        if (i > 0) strHeader += ", ";
        if (multiValue)
            strHeader += "('" + type.getName() + "', '" + type.getTypeString()
                       + "', (" + String(type.getTypeLength()) + ",))";
        else
            strHeader += "'" + type.getTypeString() + "'";
    }
    if (multiValue)
        strHeader += "]";
    strHeader += ", 'fortran_order': False, 'shape': ";

    // save byte offset of shape field in .npy file
    // magic + header length field + current string header length:
    m_shapePos = magicLen + sizeof(uint16) + strHeader.length();
    strHeader += getShapeString(); // inits to 0 records, i.e. 1st dim has length 0
    int baseHeaderLen = magicLen + sizeof(uint16) + strHeader.length() + 1; // +1 for newline
    int padlen = nbytesAlign - (baseHeaderLen % nbytesAlign);
    strHeader = strHeader.paddedRight(' ', strHeader.length() + padlen);
    strHeader += '\n';
    uint16 strHeaderLen = strHeader.length();

    m_file->write(&magicNum, sizeof(uint8));
    m_file->write(magicStr.toUTF8(), magicStr.getNumBytesAsUTF8());
    m_file->write(&ver, sizeof(uint16));
    m_file->write(&strHeaderLen, sizeof(uint16));
    m_file->write(strHeader.toUTF8(), strHeaderLen);
    m_headerLen = m_file->getPosition(); // total header length
    m_file->flush();
}

void NpyFile::updateHeader()
{

    if (true)
    {
        // overwrite the shape part of the header - even without explicitly calling
        // m_file->flush(), overwriting seems to trigger a flush to disk,
        // while appending to end of file does not
        int64 currentPos = m_file->getPosition(); // returns int64, necessary for big files
        if (m_file->setPosition(m_shapePos))
        {
            String newShape = getShapeString();
            if (m_shapePos + newShape.getNumBytesAsUTF8() + 1 > m_headerLen) // +1 for newline
            {
                std::cerr << "Error. Header has grown too big to update in-place " << std::endl;
            }
            m_file->write(newShape.toUTF8(), newShape.getNumBytesAsUTF8());
            m_file->flush(); // not necessary, already flushed due to overwrite? do it anyway
            m_file->setPosition(currentPos); // restore position to end of file
        }
        else
        {
            std::cerr << "Error. Unable to seek to update file header"
                << m_file->getFile().getFullPathName() << std::endl;
        }
    }

}

NpyFile::~NpyFile()
{
    updateHeader();
}

void NpyFile::writeData(const void* data, size_t size)
{
    m_file->write(data, size);
}

void NpyFile::increaseRecordCount(int count)
{
    int64 old_recordCount = m_recordCount;
    m_recordCount += count;
    if ((old_recordCount / recordBufferSize) != (m_recordCount / recordBufferSize))
        updateHeader(); // crossed recordBufferSize threshold, update header
}

NpyType::NpyType(String n, BaseType t, size_t l)
    : name(n), type(t), length(l)
{
}

NpyType::NpyType(BaseType t, size_t l)
    : name(String::empty), type(t), length(l)
{
}

NpyType::NpyType()
    : name(String::empty), type(BaseType::INT8), length(1)
{

}

String NpyType::getTypeString() const
{
    switch (type)
    {
    case BaseType::CHAR:
        return "|S" + String(length + 1); // null-terminated bytes, account for null separator
    case BaseType::INT8:
        return "|i1";
    case BaseType::UINT8:
        return "|u1";
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
        return "|i1"; // signed byte
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

BaseType NpyType::getType() const
{
    return type;
}