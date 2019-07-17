//----------------------------------------------------------------------------------
// rhd2000datablockusb3.h
//
// Intan Technoloies RHD2000 Rhythm USB3 Interface API
// Rhd2000DataBlockUsb3 Class Header File
// Version 2.04 (28 March 2017)
//
// Copyright (c) 2013-2017 Intan Technologies LLC
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any applications that
// use Intan Technologies integrated circuits, and to alter it and redistribute it
// freely.
//
// See http://www.intantech.com for documentation and product information.
//----------------------------------------------------------------------------------

#ifndef RHD2000DATABLOCKUSB3_H
#define RHD2000DATABLOCKUSB3_H

#define SAMPLES_PER_DATA_BLOCK 256
#define CHANNELS_PER_STREAM 32
#define RHD2000_HEADER_MAGIC_NUMBER 0xd7a22aaa38132a53

using namespace std;

class Rhd2000EvalBoardUsb3;

class Rhd2000DataBlockUsb3
{
public:
    Rhd2000DataBlockUsb3(int numDataStreams);
    ~Rhd2000DataBlockUsb3();
    Rhd2000DataBlockUsb3(const Rhd2000DataBlockUsb3 &obj); // copy constructor

    vector<unsigned int> timeStamp;
    int* amplifierDataFast;
    // vector<vector<vector<int> > > amplifierData;
    vector<vector<vector<int> > > auxiliaryData;
    vector<vector<int> > boardAdcData;
    vector<int> ttlIn;
    vector<int> ttlOut;

    static unsigned int calculateDataBlockSizeInWords(int numDataStreams, int nSamples = -1);
    static unsigned int getSamplesPerDataBlock();
    void fillFromUsbBuffer(unsigned char usbBuffer[], int blockIndex, int numDataStreams, int nSamples = -1);
    void print(int stream) const;
    void write(ofstream &saveOut, int numDataStreams) const;
    static bool checkUsbHeader(unsigned char usbBuffer[], int index);
	static unsigned int convertUsbTimeStamp(unsigned char usbBuffer[], int index);
    inline int fastIndex(int stream, int channel, int t) const
    {
    	return ((t * numDataStreamsStored * CHANNELS_PER_STREAM) + (channel * numDataStreamsStored) + stream);
    }

private:
    void allocateIntArray3D(vector<vector<vector<int> > > &array3D, int xSize, int ySize, int zSize);
    void allocateIntArray2D(vector<vector<int> > &array2D, int xSize, int ySize);
    void allocateIntArray1D(vector<int> &array1D, int xSize);
    void allocateUIntArray1D(vector<unsigned int> &array1D, int xSize);

    void writeWordLittleEndian(ofstream &outputStream, int dataWord) const;

    int numDataStreamsStored;
    int convertUsbWord(unsigned char usbBuffer[], int index);
};

#endif // RHD2000DATABLOCKUSB3_H
