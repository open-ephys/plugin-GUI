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
#include <H5Cpp.h>
#include "KwikFileSource.h"
#include <CoreServicesHeader.h>


using namespace H5;

#define PROCESS_ERROR std::cerr << "KwikFilesource exception: " << error.getCDetailMsg() << std::endl

KWIKFileSource::KWIKFileSource() : samplePos(0), skipRecordEngineCheck(false)
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
        int numObjs = (int) recordings.getNumObjs();

        for (int i=0; i < numObjs; i++)
        {
            try
            {
                Group recordN;
                DataSet data;
                Attribute attr;
                DataSpace dSpace;
                float sampleRate;
                float bitVolts;
                hsize_t dims[3];
                RecordInfo info;

                recordN = recordings.openGroup(String(i).toUTF8());
                data = recordN.openDataSet("data");
                attr = recordN.openAttribute("sample_rate");
                attr.read(PredType::NATIVE_FLOAT,&sampleRate);
                attr = recordN.openAttribute("bit_depth");
                attr.read(PredType::NATIVE_FLOAT,&bitVolts);
                dSpace = data.getSpace();
                dSpace.getSimpleExtentDims(dims);

                info.name="Record "+String(i);
                info.numSamples = dims[0];
                info.sampleRate = sampleRate;

                bool foundBitVoltArray = false;
                HeapBlock<float> bitVoltArray(dims[1]);

                try
                {
                    recordN = recordings.openGroup((String(i) + "/application_data").toUTF8());
                    attr=recordN.openAttribute("channel_bit_volts");
                    attr.read(ArrayType(PredType::NATIVE_FLOAT,1,&dims[1]),bitVoltArray);
                    foundBitVoltArray = true;
                } catch (GroupIException)
                {
                } catch (AttributeIException)
                {
                }

                for (int j = 0; j < dims[1]; j++)
                {
                    RecordedChannelInfo c;
                    c.name = "CH" + String(j);
                    
                    if (foundBitVoltArray)
                        c.bitVolts = bitVoltArray[j];
                    else
                        c.bitVolts = bitVolts;

                    info.channels.add(c);
                }
                infoArray.add(info);
                availableDataSets.add(i);
                numRecords++;
            }
            catch (GroupIException)
            {
            }
            catch (DataSetIException)
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
    catch (FileIException error)
    {
        PROCESS_ERROR;
    }
    catch (GroupIException error)
    {
        PROCESS_ERROR;
    }
}

void KWIKFileSource::updateActiveRecord()
{
    samplePos=0;
    try
    {
        String path = "/recordings/" + String(availableDataSets[activeRecord.get()]) + "/data";
        dataSet = new DataSet(sourceFile->openDataSet(path.toUTF8()));
    }
    catch (FileIException error)
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
        samplesToRead = (int) getActiveNumSamples() - (int) samplePos;
    }
    else
    {
        samplesToRead = nSamples;
    }

    try
    {
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

void KWIKFileSource::processChannelData(int16* inBuffer, float* outBuffer, int channel, int64 numSamples)
{
    int n = getActiveNumChannels();
    float bitVolts = getChannelInfo(channel).bitVolts;

    for (int i=0; i < numSamples; i++)
    {
        *(outBuffer+i) = *(inBuffer+(n*i)+channel) * bitVolts;
    }

}

bool KWIKFileSource::isReady()
{
	//HDF5 is by default not thread-safe, so we must warn the user.
	if ((!skipRecordEngineCheck) && (CoreServices::getSelectedRecordEngineId() == "KWIK"))
	{
		int res = AlertWindow::showYesNoCancelBox(AlertWindow::WarningIcon, "Record format conflict",
			"Both the selected input file for the File Reader and the output file format for recording use the HDF5 library.\n"
			"This library is, by default, not thread safe, so running both at the same time might cause unexpected crashes (chances increase with signal complexity and number of recorded channels).\n\n"
			"If you have a custom-built hdf5 library with the thread safe features turned on, you can safely continue, but performance will be reduced.\n"
			"More information on:\n"
			"https://www.hdfgroup.org/HDF5/doc/TechNotes/ThreadSafeLibrary.html\n"
			"https://www.hdfgroup.org/hdf5-quest.html\n\n"
			"Do you want to continue acquisition?", "Yes", "Yes and don't ask again", "No");
		switch (res)
		{
		case 2:
			skipRecordEngineCheck = true;
		case 1:
			return true;
			break;
		default:
			return false;
		}
	}
	else
		return true;
}
