//----------------------------------------------------------------------------------
// rhd2000datablock.cpp
//
// Intan Technoloies RHD2000 Rhythm Interface API
// Rhd2000DataBlock Class
// Version 1.4 (26 February 2014)
//
// Copyright (c) 2013-2014 Intan Technologies LLC
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

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

#include "rhd2000datablock.h"

using namespace std;

// This class creates a data structure storing SAMPLES_PER_DATA_BLOCK data frames
// from a Rhythm FPGA interface controlling up to eight RHD2000 chips.

// Constructor.  Allocates memory for data block.
Rhd2000DataBlock::Rhd2000DataBlock(int numDataStreams, bool usb3) : samplesPerBlock(SAMPLES_PER_DATA_BLOCK(usb3)), usb3(usb3)
{
    allocateUIntArray1D(timeStamp, samplesPerBlock);
	allocateIntArray3D(amplifierData, numDataStreams, 32, samplesPerBlock);
	allocateIntArray3D(auxiliaryData, numDataStreams, 3, samplesPerBlock);
	allocateIntArray2D(boardAdcData, 8, samplesPerBlock);
	allocateIntArray1D(ttlIn, samplesPerBlock);
	allocateIntArray1D(ttlOut, samplesPerBlock);
}

// Allocates memory for a 1-D array of integers.
void Rhd2000DataBlock::allocateIntArray1D(vector<int> &array1D, int xSize)
{
    array1D.resize(xSize);
}

// Allocates memory for a 1-D array of unsigned integers.
void Rhd2000DataBlock::allocateUIntArray1D(vector<unsigned int> &array1D, int xSize)
{
    array1D.resize(xSize);
}

// Allocates memory for a 2-D array of integers.
void Rhd2000DataBlock::allocateIntArray2D(vector<vector<int> > & array2D, int xSize, int ySize)
{
    int i;

    array2D.resize(xSize);
    for (i = 0; i < xSize; ++i)
        array2D[i].resize(ySize);
}

// Allocates memory for a 3-D array of integers.
void Rhd2000DataBlock::allocateIntArray3D(vector<vector<vector<int> > > &array3D, int xSize, int ySize, int zSize)
{
    int i, j;

    array3D.resize(xSize);
    for (i = 0; i < xSize; ++i) {
        array3D[i].resize(ySize);

        for (j = 0; j < ySize; ++j) {
            array3D[i][j].resize(zSize);
        }
    }
}

// Returns the number of samples in a USB data block.
unsigned int Rhd2000DataBlock::getSamplesPerDataBlock(bool usb3)
{
	return SAMPLES_PER_DATA_BLOCK(usb3);
}

// Returns the number of 16-bit words in a USB data block with numDataStreams data streams enabled.
unsigned int Rhd2000DataBlock::calculateDataBlockSizeInWords(int numDataStreams, bool usb3, int nSamples)
{
	unsigned int samps = nSamples <= 0 ? SAMPLES_PER_DATA_BLOCK(usb3) : nSamples;
	return samps * (4 + 2 + numDataStreams * 36 + 8 + 2);
    // 4 = magic number; 2 = time stamp; 36 = (32 amp channels + 3 aux commands + 1 filler word); 8 = ADCs; 2 = TTL in/out
}

// Check first 64 bits of USB header against the fixed Rhythm "magic number" to verify data sync.
bool Rhd2000DataBlock::checkUsbHeader(unsigned char usbBuffer[], int index)
{
    unsigned long long x1, x2, x3, x4, x5, x6, x7, x8;
    unsigned long long header;

    x1 = usbBuffer[index];
    x2 = usbBuffer[index + 1];
    x3 = usbBuffer[index + 2];
    x4 = usbBuffer[index + 3];
    x5 = usbBuffer[index + 4];
    x6 = usbBuffer[index + 5];
    x7 = usbBuffer[index + 6];
    x8 = usbBuffer[index + 7];

    header = (x8 << 56) + (x7 << 48) + (x6 << 40) + (x5 << 32) + (x4 << 24) + (x3 << 16) + (x2 << 8) + (x1 << 0);

    return (header == RHD2000_HEADER_MAGIC_NUMBER);
}

// Read 32-bit time stamp from USB data frame.
unsigned int Rhd2000DataBlock::convertUsbTimeStamp(unsigned char usbBuffer[], int index)
{
    unsigned int x1, x2, x3, x4;
    x1 = usbBuffer[index];
    x2 = usbBuffer[index + 1];
    x3 = usbBuffer[index + 2];
    x4 = usbBuffer[index + 3];

    return (x4 << 24) + (x3 << 16) + (x2 << 8) + (x1 << 0);
}

// Convert two USB bytes into 16-bit word.
int Rhd2000DataBlock::convertUsbWord(unsigned char usbBuffer[], int index)
{
    unsigned int x1, x2, result;

    x1 = (unsigned int) usbBuffer[index];
    x2 = (unsigned int) usbBuffer[index + 1];

    result = (x2 << 8) | (x1 << 0);

    return (int) result;
}

// Fill data block with raw data from USB input buffer.
void Rhd2000DataBlock::fillFromUsbBuffer(unsigned char usbBuffer[], int blockIndex, int numDataStreams, int nSamples)
{
    int index, t, channel, stream, i;
	int samplesToRead = nSamples <= 0 ? samplesPerBlock : nSamples;
	int num = 0;

    index = blockIndex * 2 * calculateDataBlockSizeInWords(numDataStreams, usb3);
	for (t = 0; t < samplesToRead; ++t) {
		if (!checkUsbHeader(usbBuffer, index)) {
			cerr << "Error in Rhd2000EvalBoard::readDataBlock: Incorrect header." << endl;
			break;
		}
		else
			num++;
		//else cerr << "Block ok" << endl;
        index += 8;
        timeStamp[t] = convertUsbTimeStamp(usbBuffer, index);
        index += 4;

        // Read auxiliary results
        for (channel = 0; channel < 3; ++channel) {
            for (stream = 0; stream < numDataStreams; ++stream) {
                auxiliaryData[stream][channel][t] = convertUsbWord(usbBuffer, index);
                index += 2;
            }
        }

        // Read amplifier channels
        for (channel = 0; channel < 32; ++channel) {
            for (stream = 0; stream < numDataStreams; ++stream) {
                amplifierData[stream][channel][t] = convertUsbWord(usbBuffer, index);
                index += 2;
            }
        }

        // skip 36th filler word in each data stream
        index += 2 * numDataStreams;

        // Read from AD5662 ADCs
        for (i = 0; i < 8; ++i) {
            boardAdcData[i][t] = convertUsbWord(usbBuffer, index);
            index += 2;
        }

        // Read TTL input and output values
        ttlIn[t] = convertUsbWord(usbBuffer, index);
        index += 2;

        ttlOut[t] = convertUsbWord(usbBuffer, index);
        index += 2;
    }
	//cout << "Read " << num << " valid samples with " << numDataStreams << " streams. Usb mode status: " << usb3 << endl;
}

// Print the contents of RHD2000 registers from a selected USB data stream (0-7)
// to the console.
void Rhd2000DataBlock::print(int stream) const
{
    const int RamOffset = 37;

    cout << endl;
    cout << "RHD 2000 Data Block contents:" << endl;
    cout << "  ROM contents:" << endl;
    cout << "    Chip Name: " <<
           (char) auxiliaryData[stream][2][24] <<
           (char) auxiliaryData[stream][2][25] <<
           (char) auxiliaryData[stream][2][26] <<
           (char) auxiliaryData[stream][2][27] <<
           (char) auxiliaryData[stream][2][28] <<
           (char) auxiliaryData[stream][2][29] <<
           (char) auxiliaryData[stream][2][30] <<
           (char) auxiliaryData[stream][2][31] << endl;
    cout << "    Company Name:" <<
           (char) auxiliaryData[stream][2][32] <<
           (char) auxiliaryData[stream][2][33] <<
           (char) auxiliaryData[stream][2][34] <<
           (char) auxiliaryData[stream][2][35] <<
           (char) auxiliaryData[stream][2][36] << endl;
    cout << "    Intan Chip ID: " << auxiliaryData[stream][2][19] << endl;
    cout << "    Number of Amps: " << auxiliaryData[stream][2][20] << endl;
    cout << "    Unipolar/Bipolar Amps: ";
    switch (auxiliaryData[stream][2][21]) {
        case 0:
            cout << "bipolar";
            break;
        case 1:
            cout << "unipolar";
            break;
        default:
            cout << "UNKNOWN";
    }
    cout << endl;
    cout << "    Die Revision: " << auxiliaryData[stream][2][22] << endl;
    cout << "    Future Expansion Register: " << auxiliaryData[stream][2][23] << endl;

    cout << "  RAM contents:" << endl;
    cout << "    ADC reference BW:      " << ((auxiliaryData[stream][2][RamOffset + 0] & 0xc0) >> 6) << endl;
    cout << "    amp fast settle:       " << ((auxiliaryData[stream][2][RamOffset + 0] & 0x20) >> 5) << endl;
    cout << "    amp Vref enable:       " << ((auxiliaryData[stream][2][RamOffset + 0] & 0x10) >> 4) << endl;
    cout << "    ADC comparator bias:   " << ((auxiliaryData[stream][2][RamOffset + 0] & 0x0c) >> 2) << endl;
    cout << "    ADC comparator select: " << ((auxiliaryData[stream][2][RamOffset + 0] & 0x03) >> 0) << endl;
    cout << "    VDD sense enable:      " << ((auxiliaryData[stream][2][RamOffset + 1] & 0x40) >> 6) << endl;
    cout << "    ADC buffer bias:       " << ((auxiliaryData[stream][2][RamOffset + 1] & 0x3f) >> 0) << endl;
    cout << "    MUX bias:              " << ((auxiliaryData[stream][2][RamOffset + 2] & 0x3f) >> 0) << endl;
    cout << "    MUX load:              " << ((auxiliaryData[stream][2][RamOffset + 3] & 0xe0) >> 5) << endl;
    cout << "    tempS2, tempS1:        " << ((auxiliaryData[stream][2][RamOffset + 3] & 0x10) >> 4) << "," <<
           ((auxiliaryData[stream][2][RamOffset + 3] & 0x08) >> 3) << endl;
    cout << "    tempen:                " << ((auxiliaryData[stream][2][RamOffset + 3] & 0x04) >> 2) << endl;
    cout << "    digout HiZ:            " << ((auxiliaryData[stream][2][RamOffset + 3] & 0x02) >> 1) << endl;
    cout << "    digout:                " << ((auxiliaryData[stream][2][RamOffset + 3] & 0x01) >> 0) << endl;
    cout << "    weak MISO:             " << ((auxiliaryData[stream][2][RamOffset + 4] & 0x80) >> 7) << endl;
    cout << "    twoscomp:              " << ((auxiliaryData[stream][2][RamOffset + 4] & 0x40) >> 6) << endl;
    cout << "    absmode:               " << ((auxiliaryData[stream][2][RamOffset + 4] & 0x20) >> 5) << endl;
    cout << "    DSPen:                 " << ((auxiliaryData[stream][2][RamOffset + 4] & 0x10) >> 4) << endl;
    cout << "    DSP cutoff freq:       " << ((auxiliaryData[stream][2][RamOffset + 4] & 0x0f) >> 0) << endl;
    cout << "    Zcheck DAC power:      " << ((auxiliaryData[stream][2][RamOffset + 5] & 0x40) >> 6) << endl;
    cout << "    Zcheck load:           " << ((auxiliaryData[stream][2][RamOffset + 5] & 0x20) >> 5) << endl;
    cout << "    Zcheck scale:          " << ((auxiliaryData[stream][2][RamOffset + 5] & 0x18) >> 3) << endl;
    cout << "    Zcheck conn all:       " << ((auxiliaryData[stream][2][RamOffset + 5] & 0x04) >> 2) << endl;
    cout << "    Zcheck sel pol:        " << ((auxiliaryData[stream][2][RamOffset + 5] & 0x02) >> 1) << endl;
    cout << "    Zcheck en:             " << ((auxiliaryData[stream][2][RamOffset + 5] & 0x01) >> 0) << endl;
    cout << "    Zcheck DAC:            " << ((auxiliaryData[stream][2][RamOffset + 6] & 0xff) >> 0) << endl;
    cout << "    Zcheck select:         " << ((auxiliaryData[stream][2][RamOffset + 7] & 0x3f) >> 0) << endl;
    cout << "    ADC aux1 en:           " << ((auxiliaryData[stream][2][RamOffset + 9] & 0x80) >> 7) << endl;
    cout << "    ADC aux2 en:           " << ((auxiliaryData[stream][2][RamOffset + 11] & 0x80) >> 7) << endl;
    cout << "    ADC aux3 en:           " << ((auxiliaryData[stream][2][RamOffset + 13] & 0x80) >> 7) << endl;
    cout << "    offchip RH1:           " << ((auxiliaryData[stream][2][RamOffset + 8] & 0x80) >> 7) << endl;
    cout << "    offchip RH2:           " << ((auxiliaryData[stream][2][RamOffset + 10] & 0x80) >> 7) << endl;
    cout << "    offchip RL:            " << ((auxiliaryData[stream][2][RamOffset + 12] & 0x80) >> 7) << endl;

    int rH1Dac1 = auxiliaryData[stream][2][RamOffset + 8] & 0x3f;
    int rH1Dac2 = auxiliaryData[stream][2][RamOffset + 9] & 0x1f;
    int rH2Dac1 = auxiliaryData[stream][2][RamOffset + 10] & 0x3f;
    int rH2Dac2 = auxiliaryData[stream][2][RamOffset + 11] & 0x1f;
    int rLDac1 = auxiliaryData[stream][2][RamOffset + 12] & 0x7f;
    int rLDac2 = auxiliaryData[stream][2][RamOffset + 13] & 0x3f;
    int rLDac3 = auxiliaryData[stream][2][RamOffset + 13] & 0x40 >> 6;

    double rH1 = 2630.0 + rH1Dac2 * 30800.0 + rH1Dac1 * 590.0;
    double rH2 = 8200.0 + rH2Dac2 * 38400.0 + rH2Dac1 * 730.0;
    double rL = 3300.0 + rLDac3 * 3000000.0 + rLDac2 * 15400.0 + rLDac1 * 190.0;

    cout << fixed << setprecision(2);

    cout << "    RH1 DAC1, DAC2:        " << rH1Dac1 << " " << rH1Dac2 << " = " << (rH1 / 1000) <<
            " kOhm" << endl;
    cout << "    RH2 DAC1, DAC2:        " << rH2Dac1 << " " << rH2Dac2 << " = " << (rH2 / 1000) <<
            " kOhm" << endl;
    cout << "    RL DAC1, DAC2, DAC3:   " << rLDac1 << " " << rLDac2 << " " << rLDac3 << " = " <<
            (rL / 1000) << " kOhm" << endl;

    cout << "    amp power[31:0]:       " <<
           ((auxiliaryData[stream][2][RamOffset + 17] & 0x80) >> 7) <<
           ((auxiliaryData[stream][2][RamOffset + 17] & 0x40) >> 6) <<
           ((auxiliaryData[stream][2][RamOffset + 17] & 0x20) >> 5) <<
           ((auxiliaryData[stream][2][RamOffset + 17] & 0x10) >> 4) <<
           ((auxiliaryData[stream][2][RamOffset + 17] & 0x08) >> 3) <<
           ((auxiliaryData[stream][2][RamOffset + 17] & 0x04) >> 2) <<
           ((auxiliaryData[stream][2][RamOffset + 17] & 0x02) >> 1) <<
           ((auxiliaryData[stream][2][RamOffset + 17] & 0x01) >> 0) << " " <<
           ((auxiliaryData[stream][2][RamOffset + 16] & 0x80) >> 7) <<
           ((auxiliaryData[stream][2][RamOffset + 16] & 0x40) >> 6) <<
           ((auxiliaryData[stream][2][RamOffset + 16] & 0x20) >> 5) <<
           ((auxiliaryData[stream][2][RamOffset + 16] & 0x10) >> 4) <<
           ((auxiliaryData[stream][2][RamOffset + 16] & 0x08) >> 3) <<
           ((auxiliaryData[stream][2][RamOffset + 16] & 0x04) >> 2) <<
           ((auxiliaryData[stream][2][RamOffset + 16] & 0x02) >> 1) <<
           ((auxiliaryData[stream][2][RamOffset + 16] & 0x01) >> 0) << " " <<
           ((auxiliaryData[stream][2][RamOffset + 15] & 0x80) >> 7) <<
           ((auxiliaryData[stream][2][RamOffset + 15] & 0x40) >> 6) <<
           ((auxiliaryData[stream][2][RamOffset + 15] & 0x20) >> 5) <<
           ((auxiliaryData[stream][2][RamOffset + 15] & 0x10) >> 4) <<
           ((auxiliaryData[stream][2][RamOffset + 15] & 0x08) >> 3) <<
           ((auxiliaryData[stream][2][RamOffset + 15] & 0x04) >> 2) <<
           ((auxiliaryData[stream][2][RamOffset + 15] & 0x02) >> 1) <<
           ((auxiliaryData[stream][2][RamOffset + 15] & 0x01) >> 0) << " " <<
           ((auxiliaryData[stream][2][RamOffset + 14] & 0x80) >> 7) <<
           ((auxiliaryData[stream][2][RamOffset + 14] & 0x40) >> 6) <<
           ((auxiliaryData[stream][2][RamOffset + 14] & 0x20) >> 5) <<
           ((auxiliaryData[stream][2][RamOffset + 14] & 0x10) >> 4) <<
           ((auxiliaryData[stream][2][RamOffset + 14] & 0x08) >> 3) <<
           ((auxiliaryData[stream][2][RamOffset + 14] & 0x04) >> 2) <<
           ((auxiliaryData[stream][2][RamOffset + 14] & 0x02) >> 1) <<
           ((auxiliaryData[stream][2][RamOffset + 14] & 0x01) >> 0) << endl;

    cout << endl;

    int tempA = auxiliaryData[stream][1][12];
    int tempB = auxiliaryData[stream][1][20];
    int vddSample = auxiliaryData[stream][1][28];

    double tempUnitsC = ((double)(tempB - tempA)) / 98.9 - 273.15;
    double tempUnitsF = (9.0/5.0) * tempUnitsC + 32.0;

    double vddSense = 0.0000748 * ((double) vddSample);

    cout << setprecision(1);
    cout << "  Temperature sensor (only one reading): " << tempUnitsC << " C (" <<
            tempUnitsF << " F)" << endl;

    cout << setprecision(2);
    cout << "  Supply voltage sensor                : " << vddSense << " V" << endl;

    cout << setprecision(6);
    cout.unsetf(ios::floatfield);
    cout << endl;
}

// Write a 16-bit dataWord to an outputStream in "little endian" format (i.e., least significant
// byte first).  We must do this explicitly for cross-platform consistency.  For example, Windows
// is a little-endian OS, while Mac OS X and Linux can be little-endian or big-endian depending on
// the processor running the operating system.
//
// (See "Endianness" article in Wikipedia for more information.)
void Rhd2000DataBlock::writeWordLittleEndian(ofstream &outputStream, int dataWord) const
{
    unsigned short msb, lsb;

    lsb = ((unsigned short) dataWord) & 0x00ff;
    msb = (((unsigned short) dataWord) & 0xff00) >> 8;

    outputStream << (unsigned char) lsb;
    outputStream << (unsigned char) msb;
}

// Write contents of data block to a binary output stream (saveOut) in little endian format.
void Rhd2000DataBlock::write(ofstream &saveOut, int numDataStreams) const
{
    int t, channel, stream, i;

	for (t = 0; t < samplesPerBlock; ++t) {
        writeWordLittleEndian(saveOut, timeStamp[t]);
        for (channel = 0; channel < 32; ++channel) {
            for (stream = 0; stream < numDataStreams; ++stream) {
                writeWordLittleEndian(saveOut, amplifierData[stream][channel][t]);
            }
        }
        for (channel = 0; channel < 3; ++channel) {
            for (stream = 0; stream < numDataStreams; ++stream) {
                writeWordLittleEndian(saveOut, auxiliaryData[stream][channel][t]);
            }
        }
        for (i = 0; i < 8; ++i) {
            writeWordLittleEndian(saveOut, boardAdcData[i][t]);
        }
        writeWordLittleEndian(saveOut, ttlIn[t]);
        writeWordLittleEndian(saveOut, ttlOut[t]);
    }
}
