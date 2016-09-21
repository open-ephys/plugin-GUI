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

#ifndef HDF5FILEFORMAT_H_INCLUDED
#define HDF5FILEFORMAT_H_INCLUDED

#include <CommonLibHeader.h>

#define PROCESS_ERROR std::cerr << error.getCDetailMsg() << std::endl; return -1
#define CHECK_ERROR(x) if (x) std::cerr << "Error at HDFRecording " << __LINE__ << std::endl;

#ifndef CHUNK_XSIZE
#define CHUNK_XSIZE 2048
#endif

#define MAX_STR_SIZE 256

namespace H5
{
class DataSet;
class H5File;
class DataType;
}

namespace OpenEphysHDF5
{

class HDF5RecordingData;

class COMMON_LIB HDF5FileBase
{
public:
    HDF5FileBase();
    virtual ~HDF5FileBase();

    int open();
	int open(int nChans);
    void close();
    virtual String getFileName() = 0;
    bool isOpen() const;
	bool isReadyToOpen() const;
    typedef enum DataTypes { U8, U16, U32, U64, I8, I16, I32, I64, F32, F64, STR} DataTypes;

    static H5::DataType getNativeType(DataTypes type);
    static H5::DataType getH5Type(DataTypes type);

protected:

    virtual int createFileStructure() = 0;

    int setAttribute(DataTypes type, const void* data, String path, String name);
    int setAttributeStr(const String& value, String path, String name);
    int setAttributeArray(DataTypes type, const void* data, int size, String path, String name);
	int setAttributeStrArray(const StringArray& data, String path, String name);
    int createGroup(String path);
	int createGroupIfDoesNotExist(String path);

    HDF5RecordingData* getDataSet(String path);

    //aliases for createDataSet
    HDF5RecordingData* createDataSet(DataTypes type, int sizeX, int chunkX, String path);
    HDF5RecordingData* createDataSet(DataTypes type, int sizeX, int sizeY, int chunkX, String path);
    HDF5RecordingData* createDataSet(DataTypes type, int sizeX, int sizeY, int sizeZ, int chunkX, String path);
    HDF5RecordingData* createDataSet(DataTypes type, int sizeX, int sizeY, int sizeZ, int chunkX, int chunkY, String path);

    bool readyToOpen;

private:
	int setAttributeStrArray(Array<const char*>& data, int maxSize, String path, String name);

    //create an extendable dataset
    HDF5RecordingData* createDataSet(DataTypes type, int dimension, int* size, int* chunking, String path);
    int open(bool newfile, int nChans);
    ScopedPointer<H5::H5File> file;
    bool opened;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HDF5FileBase);
};

class COMMON_LIB HDF5RecordingData
{
public:
    HDF5RecordingData(H5::DataSet* data);
    ~HDF5RecordingData();

    int writeDataBlock(int xDataSize, HDF5FileBase::DataTypes type, const void* data);
    int writeDataBlock(int xDataSize, int yDataSize, HDF5FileBase::DataTypes type, const void* data);

    int writeDataRow(int yPos, int xDataSize, HDF5FileBase::DataTypes type, const void* data);

    void getRowXPositions(Array<uint32>& rows);

private:
    int xPos;
    int xChunkSize;
    int size[3];
    int dimension;
    Array<uint32> rowXPos;
    ScopedPointer<H5::DataSet> dSet;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HDF5RecordingData);
};

}


#endif  // HDF5FILEFORMAT_H_INCLUDED
