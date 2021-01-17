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

#ifndef NPYFILE_H
#define NPYFILE_H

#include "../RecordEngine.h"
#include "../../../Utils/Utils.h"


class NpyType
{
public:
    NpyType(String, BaseType, size_t);
    NpyType(BaseType, size_t);
    NpyType();
    String getName() const;
    String getTypeString() const;
    int getTypeLength() const;
    BaseType getType() const;
private:
    String name;
    BaseType type;
    size_t length;
};

class NpyFile
{
public:
    NpyFile(String path, const Array<NpyType>& typeList);
    NpyFile(String path, NpyType type, unsigned int dim = 1);
    ~NpyFile();
    void writeData(const void* data, size_t size);
    void increaseRecordCount(int count = 1);
private:
    bool openFile(String path);
    String getShapeString();
    void writeHeader(const Array<NpyType>& typeList);
    void updateHeader();
    ScopedPointer<FileOutputStream> m_file;
    int64 m_headerLen; // total header length
    bool m_okOpen{ false };
    int64 m_recordCount{ 0 };
    size_t m_shapePos;
    unsigned int m_dim1;
    unsigned int m_dim2;

    // Compile-time constants

    // flush file buffer to disk and update the .npy header every this many records:
    const int recordBufferSize{ 1024 };

};

#endif
