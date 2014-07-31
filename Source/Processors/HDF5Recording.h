/*
==============================================================================

HDF5Recording.h
Created: 21 Jul 2014 1:03:17am
Author:  Kether

==============================================================================
*/

#ifndef HDF5RECORDING_H_INCLUDED
#define HDF5RECORDING_H_INCLUDED


#include "../../JuceLibraryCode/JuceHeader.h"


class HDF5RecordingData;
namespace H5 {
class DataSet;
class H5File;
class DataType;
}

class HDF5FileBase
{
public:
	HDF5FileBase();
	~HDF5FileBase();

	int open();
	void close();
	virtual String getFileName() = 0;
	bool isOpen() const;
	typedef enum DataTypes { U16, U32, U64, I16, I32, I64, F32} DataTypes;

	static H5::DataType getNativeType(DataTypes type);
	static H5::DataType getH5Type(DataTypes type);

protected:

	virtual int createFileStructure() = 0;

	int setAttribute(DataTypes type, void* data, String path, String name);
	int setAttributeStr(String value, String path, String name);
	int createGroup(String path);

	HDF5RecordingData* getDataSet(String path);

	//create dataset extendable on X dimension.
	HDF5RecordingData* createDataSet(DataTypes type, int sizeX, int sizeY, int chunkX, String path);

private:
	int open(bool newfile);
	ScopedPointer<H5::H5File> file;
	bool opened;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HDF5FileBase);
};

class HDF5RecordingData 
{
public:
	HDF5RecordingData(H5::DataSet* data);
	~HDF5RecordingData();

	int writeDataBlock(int xDataSize, HDF5FileBase::DataTypes type, void* data);

	int prepareDataBlock(int xDataSize);
	int writeDataRow(int yPos, int xDataSize, HDF5FileBase::DataTypes type, void* data);

private:
	int xPos;
	int xChunkSize;
	int ySize;
	int xSize;
	int rowXPos;
	int rowDataSize;
	ScopedPointer<H5::DataSet> dSet;
};

class KWDFile : public HDF5FileBase
{
public:
	KWDFile(int processorNumber, String basename);
	~KWDFile();
	void startNewRecording(int recordingNumber, int nChannels);
	void stopRecording();
	void writeBlockData(int16* data, int nSamples);
	void writeRowData(int16* data, int nSamples);
	String getFileName();

protected:
	int createFileStructure();

private:
	int recordingNumber;
	int nChannels;
	int curChan;
	String filename;
	ScopedPointer<HDF5RecordingData> recdata;

};


#endif  // HDF5RECORDING_H_INCLUDED
