/*
 ------------------------------------------------------------------
 
 This file is part of the Open Ephys GUI
 Copyright (C) 2013 Florian Franzen
 
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

#include <H5Cpp.h>
#include "HDF5FileFormat.h"

#ifndef BLOCK_XSIZE
#define BLOCK_XSIZE 256
#endif

#ifndef CHUNK_XSIZE
#define CHUNK_XSIZE 256
#endif

#define PROCESS_ERROR std::cerr << error.getCDetailMsg() << std::endl; return -1
#define CHECK_ERROR(x) if (x) std::cerr << "Error at HDFRecording " << __LINE__ << std::endl;

using namespace H5;

//HDF5FileBase

HDF5FileBase::HDF5FileBase() : opened(false), readyToOpen(false) 
{
	Exception::dontPrint();
};

HDF5FileBase::~HDF5FileBase() 
{
	close();
}

bool HDF5FileBase::isOpen() const
{
	return opened;
}

int HDF5FileBase::open()
{
	if (!readyToOpen) return -1;
	if (File(getFileName()).existsAsFile())
		return open(false);
	else
		return open(true);

}

int HDF5FileBase::open(bool newfile)
{
	int accFlags,ret=0;

	if (opened) return -1;

	try {

		if (newfile) accFlags = H5F_ACC_TRUNC;
		else accFlags = H5F_ACC_RDWR;
		file = new H5File(getFileName().toUTF8(),accFlags);
		opened = true;
		if (newfile)
		{
			ret = createFileStructure();
		}
		
		if (ret)
		{
			file = nullptr;
			opened = false;
			std::cerr << "Error creating file structure" << std::endl;
		}
			

		return ret;
	}
	catch(FileIException error)
	{
		PROCESS_ERROR;
	}
}

void HDF5FileBase::close()
{
	file = nullptr;
	opened = false;
}

int HDF5FileBase::setAttribute(DataTypes type, void* data, String path, String name)
{
	Group loc;
	Attribute attr;
	DataType H5type;
	DataType origType;

	if (!opened) return -1;
	try {
		loc = file->openGroup(path.toUTF8());

		H5type = getH5Type(type);
		origType = getNativeType(type);

		if (loc.attrExists(name.toUTF8()))
		{
			attr = loc.openAttribute(name.toUTF8());
		}
		else
		{
			DataSpace attr_dataspace(H5S_SCALAR);
			attr = loc.createAttribute(name.toUTF8(),H5type,attr_dataspace);
		}

		attr.write(origType,data);

	} catch (GroupIException error) {
		PROCESS_ERROR;
	}
	catch (AttributeIException error)
	{
		PROCESS_ERROR;
	}

	return 0;
}

int HDF5FileBase::setAttributeStr(String value, String path, String name)
{
	Group loc;
	Attribute attr;

	if (!opened) return -1;
	
	StrType type(PredType::C_S1, value.length());
	try {
		loc = file->openGroup(path.toUTF8());

		if (loc.attrExists(name.toUTF8()))
		{
			//attr = loc.openAttribute(name.toUTF8());
			return -1; //string attributes cannot change size easily, better not allow overwritting.
		}
		else
		{
			DataSpace attr_dataspace(H5S_SCALAR);
			attr = loc.createAttribute(name.toUTF8(), type, attr_dataspace);
		}
		attr.write(type,value.toUTF8());

	} catch (GroupIException error) {
		PROCESS_ERROR;
	}
	catch (AttributeIException error)
	{
		PROCESS_ERROR;
	}
	catch (FileIException error)
	{
		PROCESS_ERROR;
	}

	return 0;
}

int HDF5FileBase::createGroup(String path)
{
	if (!opened) return -1;
	try {
		file->createGroup(path.toUTF8());
	}
	catch (FileIException error)
	{
		PROCESS_ERROR;
	}
	catch (GroupIException error)
	{
		PROCESS_ERROR;
	}
	return 0;
}

HDF5RecordingData* HDF5FileBase::getDataSet(String path)
{
	ScopedPointer<DataSet> data;

	if (!opened) return nullptr;

	try{
		data = new DataSet(file->openDataSet(path.toUTF8()));
		return new HDF5RecordingData(data.release());
	}
	catch(DataSetIException error)
	{
		error.printError();
		return nullptr;
	}
	catch(FileIException error)
	{
		error.printError();
		return nullptr;
	}
	catch(DataSpaceIException error)
	{
		error.printError();
		return nullptr;
	}
}

HDF5RecordingData* HDF5FileBase::createDataSet(DataTypes type, int sizeX, int sizeY, int chunkX, String path)
{
	ScopedPointer<DataSet> data;
	DSetCreatPropList prop;
	if (!opened) return nullptr;
	DataType H5type = getH5Type(type); 

	hsize_t dims[2], chunk_dims[2], max_dims[2];
	dims[0] = sizeX;
	dims[1] = sizeY;
	chunk_dims[0] = chunkX;
	chunk_dims[1] = sizeY;
	max_dims[0] = H5S_UNLIMITED;
	max_dims[1] = sizeY;

	try {
		DataSpace dSpace(2,dims,max_dims);
		prop.setChunk(2,chunk_dims);
	 
		data = new DataSet(file->createDataSet(path.toUTF8(),H5type,dSpace,prop));
		return new HDF5RecordingData(data.release());
	}
	catch(DataSetIException error)
	{
		error.printError();
		return nullptr;
	}
	catch(FileIException error)
	{
		error.printError();
		return nullptr;
	}
	catch(DataSpaceIException error)
	{
		error.printError();
		return nullptr;
	}


}

H5::DataType HDF5FileBase::getNativeType(DataTypes type)
{
	switch (type)
	{
	case I16:
		return PredType::NATIVE_INT16;
		break;
	case I32:
		return PredType::NATIVE_INT32;
		break;
	case I64:
		return PredType::NATIVE_INT64;
		break;
	case U16:
		return PredType::NATIVE_UINT16;
		break;
	case U32:
		return PredType::NATIVE_UINT32;
		break;
	case U64:
		return PredType::NATIVE_UINT64;
		break;
	case F32:
		return PredType::NATIVE_FLOAT;
		break;
	}
	return PredType::NATIVE_INT32;
}

H5::DataType HDF5FileBase::getH5Type(DataTypes type)
{
	switch (type)
	{
	case I16:
		return PredType::STD_I16BE;
		break;
	case I32:
		return PredType::STD_I32BE;
		break;
	case I64:
		return PredType::STD_I64BE;
		break;
	case U16:
		return PredType::STD_U16BE;
		break;
	case U32:
		return PredType::STD_U32BE;
		break;
	case U64:
		return PredType::STD_U64BE;
		break;
	case F32:
		return PredType::IEEE_F32BE;
		break;
	}
	return PredType::STD_I32BE;
}

HDF5RecordingData::HDF5RecordingData(DataSet *data)
{
	DataSpace dSpace;
	DSetCreatPropList prop;
	ScopedPointer<DataSet> dSet = data;
	hsize_t dims[2], chunk[2];

	dSpace = dSet->getSpace();
	prop = dSet->getCreatePlist();

	dSpace.getSimpleExtentDims(dims);
	prop.getChunk(2,chunk);

	this->xSize = dims[0];
	this->ySize = dims[1];
	this->xChunkSize = chunk[0];
	this->xPos = 0;
	this->dSet = dSet;	
	this->rowXPos = 0;
	this->rowDataSize = 0;
}

HDF5RecordingData::~HDF5RecordingData()
{
}

int HDF5RecordingData::writeDataBlock(int xDataSize, HDF5FileBase::DataTypes type, void* data)
{
	hsize_t dim[2],offset[2];
	DataSpace fSpace;
	DataType nativeType;

	dim[1] = ySize;
	dim[0] = xPos + xDataSize;
	try {
		//First be sure that we have enough space
		dSet->extend(dim);

		fSpace = dSet->getSpace();
		fSpace.getSimpleExtentDims(dim);
		xSize=dim[0];

		//Create memory space
		dim[0]=xDataSize;
		dim[1]=ySize;
		DataSpace mSpace(2,dim);

		//select where to write
		offset[0]=xPos;
		offset[1]=0;
		fSpace.selectHyperslab(H5S_SELECT_SET, dim, offset);

		nativeType = HDF5FileBase::getNativeType(type);

		dSet->write(data,nativeType,mSpace,fSpace);
		xPos += xDataSize;
	}
	catch (DataSetIException error)
	{
		PROCESS_ERROR;
	}
	catch (DataSpaceIException error)
	{
		PROCESS_ERROR;
	}
	return 0;
}

int HDF5RecordingData::prepareDataBlock(int xDataSize)
{
	hsize_t dim[2];
	DataSpace fSpace;

	dim[1] = ySize;
	dim[0] = xPos + xDataSize;
	try{
		dSet->extend(dim);

		fSpace = dSet->getSpace();
		fSpace.getSimpleExtentDims(dim);
		xSize=dim[0];
	}
	catch(DataSetIException error)
	{
		PROCESS_ERROR;
	}
	rowXPos = xPos;
	rowDataSize = xDataSize;
	xPos += xDataSize;
	return 0;
}

int HDF5RecordingData::writeDataRow(int yPos, int xDataSize, HDF5FileBase::DataTypes type, void* data)
{
	hsize_t dim[2],offset[2];
	DataSpace fSpace;
	DataType nativeType;
	if (xDataSize != rowDataSize) return -2;
	if ((yPos < 0 ) || (yPos >= ySize)) return -3;

	try {
		dim[0] = xDataSize;
		dim[1] = 1;
		DataSpace mSpace(2,dim);

		fSpace = dSet->getSpace();
		offset[0] = rowXPos;
		offset[1] = yPos;
		fSpace.selectHyperslab(H5S_SELECT_SET, dim, offset);

		nativeType = HDF5FileBase::getNativeType(type);


		dSet->write(data,nativeType,mSpace,fSpace);
	}
	catch(DataSetIException error)
	{
		PROCESS_ERROR;
	}
	catch(DataSpaceIException error)
	{
		PROCESS_ERROR;
	}
	catch (FileIException error)
	{
		PROCESS_ERROR;
	}
	return 0;
}

KWDFile::KWDFile(int processorNumber, String basename) : HDF5FileBase()
{
	initFile(processorNumber, basename);
}

KWDFile::KWDFile() : HDF5FileBase()
{
}

KWDFile::~KWDFile() {}

String KWDFile::getFileName()
{
	return filename;
}

void KWDFile::initFile(int processorNumber, String basename)
{
	if (isOpen()) return;
	filename = basename + String(processorNumber) + "_raw.kwd";
	readyToOpen=true;
}

void KWDFile::startNewRecording(int recordingNumber, int nChannels, HDF5RecordingInfo* info)
{
	this->recordingNumber = recordingNumber;
	this->nChannels = nChannels;
	
	String recordPath = String("/recordings/")+String(recordingNumber);
	CHECK_ERROR(createGroup(recordPath));
	CHECK_ERROR(setAttributeStr(info->name,recordPath,String("name")));
	CHECK_ERROR(setAttribute(I64,&(info->start_time),recordPath,String("start_time")));
	CHECK_ERROR(setAttribute(U32,&(info->start_sample),recordPath,String("start_sample")));
	CHECK_ERROR(setAttribute(F32,&(info->sample_rate),recordPath,String("sample_rate")));
	CHECK_ERROR(setAttribute(F32,&(info->bit_depth),recordPath,String("bit_depth")));
	recdata = createDataSet(I16,BLOCK_XSIZE,nChannels,CHUNK_XSIZE,recordPath+"/data");
	if (!recdata.get())
		std::cerr << "Error creating data set" << std::endl;
	curChan = nChannels;
}

void KWDFile::stopRecording() 
{
	//ScopedPointer does the deletion and destructors the closings
	recdata = nullptr;
}

int KWDFile::createFileStructure()
{
	const uint16 ver = 2;
	if(createGroup("/recordings")) return -1;
	if(setAttribute(U16,(void*)&ver,"/","kwik_version")) return -1;
	return 0;
}

void KWDFile::writeBlockData(int16* data, int nSamples)
{
	CHECK_ERROR(recdata->writeDataBlock(nSamples,I16,data));
}

void KWDFile::writeRowData(int16* data, int nSamples)
{
	if (curChan >= nChannels)
	{
		CHECK_ERROR(recdata->prepareDataBlock(nSamples));
		curChan=0;
	}
	CHECK_ERROR(recdata->writeDataRow(curChan,nSamples,I16,data));
	curChan++;
}