//----------------------------------------------------------------------------------
// rhd2000registers.h
//
// Intan Technoloies RHD2000 Rhythm Interface API
// Rhd2000Registers Class Header File
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

#ifndef RHD2000REGISTERS_H
#define RHD2000REGISTERS_H

using namespace std;

class Rhd2000Registers
{

public:
    Rhd2000Registers(double sampleRate);

    void defineSampleRate(double newSampleRate);

    void setFastSettle(bool enabled);

    void setDigOutLow();
    void setDigOutHigh();
    void setDigOutHiZ();

    void enableAux1(bool enabled);
    void enableAux2(bool enabled);
    void enableAux3(bool enabled);

    void enableDsp(bool enabled);
    void disableDsp();
    double setDspCutoffFreq(double newDspCutoffFreq);
    double getDspCutoffFreq() const;

    void enableZcheck(bool enabled);
    void setZcheckDacPower(bool enabled);

    enum ZcheckCs {
        ZcheckCs100fF,
        ZcheckCs1pF,
        ZcheckCs10pF
    };

    enum ZcheckPolarity {
        ZcheckPositiveInput,
        ZcheckNegativeInput
    };

    void setZcheckScale(ZcheckCs scale);
    void setZcheckPolarity(ZcheckPolarity polarity);
    int setZcheckChannel(int channel);

    void setAmpPowered(int channel, bool powered);
    void powerUpAllAmps();
    void powerDownAllAmps();

    int getRegisterValue(int reg) const;

    double setUpperBandwidth(double upperBandwidth);
    double setLowerBandwidth(double lowerBandwidth);

    int createCommandListRegisterConfig(vector<int> &commandList, bool calibrate);
    int createCommandListTempSensor(vector<int> &commandList);
    int createCommandListUpdateDigOut(vector<int> &commandList);
    int createCommandListZcheckDac(vector<int> &commandList, double frequency, double amplitude);

    enum Rhd2000CommandType {
        Rhd2000CommandConvert,
        Rhd2000CommandCalibrate,
        Rhd2000CommandCalClear,
        Rhd2000CommandRegWrite,
        Rhd2000CommandRegRead
    };

    int createRhd2000Command(Rhd2000CommandType commandType);
    int createRhd2000Command(Rhd2000CommandType commandType, int arg1);
    int createRhd2000Command(Rhd2000CommandType commandType, int arg1, int arg2);

private:
    double sampleRate;

    // RHD2000 Register 0 variables
    int adcReferenceBw;
    int ampFastSettle;
    int ampVrefEnable;
    int adcComparatorBias;
    int adcComparatorSelect;

    // RHD2000 Register 1 variables
    int vddSenseEnable;
    int adcBufferBias;

    // RHD2000 Register 2 variables
    int muxBias;

    // RHD2000 Register 3 variables
    int muxLoad;
    int tempS1;
    int tempS2;
    int tempEn;
    int digOutHiZ;
    int digOut;

    // RHD2000 Register 4 variables
    int weakMiso;
    int twosComp;
    int absMode;
    int dspEn;
    int dspCutoffFreq;

    // RHD2000 Register 5 variables
    int zcheckDacPower;
    int zcheckLoad;
    int zcheckScale;
    int zcheckConnAll;
    int zcheckSelPol;
    int zcheckEn;

    // RHD2000 Register 6 variables
    //int zcheckDac;     // handle Zcheck DAC waveform elsewhere

    // RHD2000 Register 7 variables
    int zcheckSelect;

    // RHD2000 Register 8-13 variables
    int offChipRH1;
    int offChipRH2;
    int offChipRL;
    int adcAux1En;
    int adcAux2En;
    int adcAux3En;
    int rH1Dac1;
    int rH1Dac2;
    int rH2Dac1;
    int rH2Dac2;
    int rLDac1;
    int rLDac2;
    int rLDac3;

    // RHD2000 Register 14-17 variables
    vector<int> aPwr;

    double rH1FromUpperBandwidth(double upperBandwidth) const;
    double rH2FromUpperBandwidth(double upperBandwidth) const;
    double rLFromLowerBandwidth(double lowerBandwidth) const;
    double upperBandwidthFromRH1(double rH1) const;
    double upperBandwidthFromRH2(double rH2) const;
    double lowerBandwidthFromRL(double rL) const;

    static const int MaxCommandLength = 1024; // size of on-FPGA auxiliary command RAM banks

};

#endif // RHD2000REGISTERS_H
