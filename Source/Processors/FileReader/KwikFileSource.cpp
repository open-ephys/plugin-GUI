/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "KwikFileSource.h"

#include <h5cpp.h>

using namespace H5;

#define PROCESS_ERROR std::cerr << "KwikFilesource exception: " << error.getCDetailMsg() << std::endl

KWIKFileSource::KWIKFileSource() : samplePos(0)
{
}

KWIKFileSource::~KWIKFileSource()
{
}

bool KWIKFileSource::Open(File file)
{
	ScopedPointer<H5File> tmpFile;
	Attribute ver;
	uint16 vernum;
	try
	{
		tmpFile = new H5File(file.getFullPathName().toUTF8(),H5F_ACC_RDONLY);
		if (!tmpFile->attrExists("kwik_version"))
		{
			return false;
		}

		ver = tmpFile->openAttribute("kwik_version");
		ver.read(PredType::NATIVE_UINT16,&vernum);
		if ((vernum < MIN_KWIK_VERSION) || (vernum > MAX_KWIK_VERSION))
		{
			return false;
		}

		sourceFile = tmpFile;
		return true;

	}
	catch (FileIException error)
	{
		PROCESS_ERROR;
		return false;
	}
	catch (AttributeIException error)
	{
		PROCESS_ERROR;
		return false;
	}

	//Code should never reach here
	return false;
}

void KWIKFileSource::fillRecordInfo()
{
	Group recordings;

	try
	{
		recordings = sourceFile->openGroup("/recordings");
		int numObjs = recordings.getNumObjs();

		for (int i=0; i < numObjs; i++)
		{
			try {
				Group recordN;
				DataSet data;
				Attribute samp;
				DataSpace dSpace;
				float sampleRate;
				hsize_t dims[3];
				RecordInfo info;

				recordN = recordings.openGroup(String(i).toUTF8());
				data = recordN.openDataSet("data");
				samp = recordN.openAttribute("sample_rate");
				samp.read(PredType::NATIVE_FLOAT,&sampleRate);
				dSpace = data.getSpace();
				dSpace.getSimpleExtentDims(dims);
								
				info.name="Record "+String(i);
				info.numChannels = dims[1];
				info.numSamples = dims[0];
				info.sampleRate = sampleRate;
				infoArray.add(info);
				availableDataSets.add(i);
				numRecords++;
			}
			catch (GroupIException)
			{
			}
			catch(DataSetIException)
			{
			}
			catch (AttributeIException)
			{
			}
			catch (DataSpaceIException error)
			{
				PROCESS_ERROR;
			}
		}
	}
	catch(FileIException error)
	{
		PROCESS_ERROR;
	}
	catch(GroupIException error)
	{
		PROCESS_ERROR;
	}
}

void KWIKFileSource::updateActiveRecord()
{
	samplePos=0;
	try
	{
		String path = "/recordings/" + String(availableDataSets[activeRecord]) + "/data";
		dataSet = new DataSet(sourceFile->openDataSet(path.toUTF8()));
	}
	catch(FileIException error)
	{
		PROCESS_ERROR;
	}
	catch (DataSetIException error)
	{
		PROCESS_ERROR;
	}
}

void KWIKFileSource::seekTo(int64 sample)
{
	samplePos = sample % getActiveNumSamples();
}

int KWIKFileSource::readData(int16* buffer, int nSamples)
{
	DataSpace fSpace,mSpace;
	int samplesToRead;
	int nChannels = getActiveNumChannels();
	hsize_t dim[3],offset[3];

	if (samplePos + nSamples > getActiveNumSamples())
	{
		samplesToRead = getActiveNumSamples() - samplePos;
	}
	else
	{
		samplesToRead = nSamples;
	}

	try {
		fSpace = dataSet->getSpace();
		dim[0] = samplesToRead;
		dim[1] = nChannels;
		dim[2] = 1;
		offset[0] = samplePos;
		offset[1] = 0;
		offset[2] = 0;

		fSpace.selectHyperslab(H5S_SELECT_SET,dim,offset);
		mSpace = DataSpace(2,dim);

		dataSet->read(buffer,PredType::NATIVE_INT16,mSpace,fSpace);
		samplePos += samplesToRead;
		return samplesToRead;

	}
	catch (DataSetIException error)
	{
		PROCESS_ERROR;
		return 0;
	}
	catch (DataSpaceIException error)
	{
		PROCESS_ERROR;
		return 0;
	}
	return 0;
}