/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2022 Open Ephys

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

#ifndef NPYFILE_H
#define NPYFILE_H

#include "../RecordEngine.h"
#include "../../../Utils/Utils.h"

#include "../../PluginManager/PluginClass.h"
#include "../../Settings/Metadata.h"

/**

 Represents the data type (e.g. <i8) of a particular file
 
 */
class PLUGIN_API NpyType
{
public:
    
    /** Constructor 1*/
    NpyType(String, BaseType, size_t);
    
    /** Constructor 2*/
    NpyType(BaseType, size_t);
    
    /** Default constructor */
    NpyType();
    
    /** Returns the name of this type */
    String getName() const;
    
    /** Returns the type in numpy syntax */
    String getTypeString() const;
    
    /** Returns the length (in bytes) of this type */
    int getTypeLength() const;
    
    /** Returns the BaseType */
    BaseType getType() const;
private:
    String name;
    BaseType type;
    size_t length;
};

/**
    
    Writes array data to a file in numpy (.npy) format.
 
    These files can be easily opening in Python using the numpy library (https://numpy.org ),
 or in Matlab using the npy_matlab library (https://github.com/cortex-lab/npy_matlab)
 
 */
class PLUGIN_API NpyFile
{
public:
    
    /** Constructor for an array of types */
    NpyFile(String path, const Array<NpyType>& typeList);
    
    /** Constructor for a 1-dimensional file with a single type */
    NpyFile(String path, NpyType type, unsigned int dim = 1);
    
    /** Destructor */
    ~NpyFile();
    
    /** Writes nSamples of data to the file  */
    void writeData(const void* data, size_t nSamples);
    
    /** Increases the count of the number of records in the file (must match the number of samples written) */
    void increaseRecordCount(int count = 1);
private:
    
    /** Opens the file at a specified path */
    bool openFile(String path);
    
    /** Returns a string describing the underlying array shape */
    String getShapeString();
    
    /** Writes the initial file header */
    void writeHeader(const Array<NpyType>& typeList);
    
    /** Updates the header with the total number of samples */
    void updateHeader();
    
    std::unique_ptr<FileOutputStream> m_file;
    int64 m_headerLen;
    bool m_okOpen{ false };
    int64 m_recordCount{ 0 };
    size_t m_shapePos;
    unsigned int m_dim1;
    unsigned int m_dim2;

    /** flush file buffer to disk and update the .npy header every this many records: */
    const int recordBufferSize{ 1024 };

};

#endif
