//----------------------------------------------------------------------------------
// rhd2000registersusb3.cpp
//
// Intan Technoloies RHD2000 USB3 Rhythm Interface API
// Rhd2000RegistersUsb3 Class
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

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <queue>

#include "rhd2000registersusb3.h"

using namespace std;

// This class creates and manages a data structure representing the internal RAM registers on
// a RHD2000 chip, and generates command lists to configure the chip and perform other functions.
// Changing the value of variables within an instance of this class does not directly affect a
// RHD2000 chip connected to the FPGA; rather, a command list must be generated from this class
// and then downloaded to the FPGA board using Rhd2000EvalBoard::uploadCommandList.

// Constructor.  Set RHD2000 register variables to default values.
Rhd2000RegistersUsb3::Rhd2000RegistersUsb3(double sampleRate)
{
    aPwr.resize(64);

    defineSampleRate(sampleRate);

    // Set default values for all register settings
    adcReferenceBw = 3;         // ADC reference generator bandwidth (0 [highest BW] - 3 [lowest BW]);
                                // always set to 3
    setFastSettle(false);       // amplifier fast settle (off = normal operation)
    ampVrefEnable = 1;          // enable amplifier voltage references (0 = power down; 1 = enable);
                                // 1 = normal operation
    adcComparatorBias = 3;      // ADC comparator preamp bias current (0 [lowest] - 3 [highest], only
                                // valid for comparator select = 2,3); always set to 3
    adcComparatorSelect = 2;    // ADC comparator select; always set to 2

    vddSenseEnable = 1;         // supply voltage sensor enable (0 = disable; 1 = enable)
    // adcBufferBias = 32;      // ADC reference buffer bias current (0 [highest current] - 63 [lowest current]);
                                // This value should be set according to ADC sampling rate; set in setSampleRate()

    // muxBias = 40;            // ADC input MUX bias current (0 [highest current] - 63 [lowest current]);
                                // This value should be set according to ADC sampling rate; set in setSampleRate()

    // muxLoad = 0;             // MUX capacitance load at ADC input (0 [min CL] - 7 [max CL]); LSB = 3 pF
                                // Set in setSampleRate()

    tempS1 = 0;                 // temperature sensor S1 (0-1); 0 = power saving mode when temperature sensor is
                                // not in use
    tempS2 = 0;                 // temperature sensor S2 (0-1); 0 = power saving mode when temperature sensor is
                                // not in use
    tempEn = 0;                 // temperature sensor enable (0 = disable; 1 = enable)
    setDigOutHiZ();             // auxiliary digital output state

    weakMiso = 1;               // weak MISO (0 = MISO line is HiZ when CS is inactive; 1 = MISO line is weakly
                                // driven when CS is inactive)
    twosComp = 0;               // two's complement ADC results (0 = unsigned offset representation; 1 = signed
                                // representation)
    absMode = 0;                // absolute value mode (0 = normal output; 1 = output passed through abs(x) function)
    enableDsp(true);            // DSP offset removal enable/disable
    setDspCutoffFreq(1.0);      // DSP offset removal HPF cutoff freqeuncy

    zcheckDacPower = 1;         // impedance testing DAC power-up (0 = power down; 1 = power up)
    zcheckLoad = 0;             // impedance testing dummy load (0 = normal operation; 1 = insert 60 pF to ground)
    setZcheckScale(ZcheckCs100fF);  // impedance testing scale factor (100 fF, 1.0 pF, or 10.0 pF)
    zcheckConnAll = 0;          // impedance testing connect all (0 = normal operation; 1 = connect all electrodes together)
    setZcheckPolarity(ZcheckPositiveInput); // impedance testing polarity select (RHD2216 only) (0 = test positive inputs;
                                // 1 = test negative inputs)
    enableZcheck(false);        // impedance testing enable/disable

    setZcheckChannel(0);        // impedance testing amplifier select (0-63)

    offChipRH1 = 0;             // bandwidth resistor RH1 on/off chip (0 = on chip; 1 = off chip)
    offChipRH2 = 0;             // bandwidth resistor RH2 on/off chip (0 = on chip; 1 = off chip)
    offChipRL = 0;              // bandwidth resistor RL on/off chip (0 = on chip; 1 = off chip)
    adcAux1En = 1;              // enable ADC aux1 input (when RH1 is on chip) (0 = disable; 1 = enable)
    adcAux2En = 1;              // enable ADC aux2 input (when RH2 is on chip) (0 = disable; 1 = enable)
    adcAux3En = 1;              // enable ADC aux3 input (when RL is on chip) (0 = disable; 1 = enable)

    setUpperBandwidth(10000.0); // set upper bandwidth of amplifiers
    setLowerBandwidth(1.0);     // set lower bandwidth of amplifiers

    powerUpAllAmps();           // turn on all amplifiers
}

// Define RHD2000 per-channel sampling rate so that certain sampling-rate-dependent registers are set correctly
// (This function does not change the sampling rate of the FPGA; for this, use Rhd2000EvalBoard::setSampleRate.)
void Rhd2000RegistersUsb3::defineSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;

    muxLoad = 0;

    if (sampleRate < 3334.0) {
        muxBias = 40;
        adcBufferBias = 32;
    } else if (sampleRate < 4001.0) {
        muxBias = 40;
        adcBufferBias = 16;
    } else if (sampleRate < 5001.0) {
        muxBias = 40;
        adcBufferBias = 8;
    } else if (sampleRate < 6251.0) {
        muxBias = 32;
        adcBufferBias = 8;
    } else if (sampleRate < 8001.0) {
        muxBias = 26;
        adcBufferBias = 8;
    } else if (sampleRate < 10001.0) {
        muxBias = 18;
        adcBufferBias = 4;
    } else if (sampleRate < 12501.0) {
        muxBias = 16;
        adcBufferBias = 3;
    } else if (sampleRate < 15001.0) {
        muxBias = 7;
        adcBufferBias = 3;
    } else {
        muxBias = 4;
        adcBufferBias = 2;
    }
}

// Enable or disable amplifier fast settle function; drive amplifiers to baseline
// if enabled.
void Rhd2000RegistersUsb3::setFastSettle(bool enabled)
{
    ampFastSettle = (enabled ? 1 : 0);
}

// Drive auxiliary digital output low
void Rhd2000RegistersUsb3::setDigOutLow()
{
    digOut = 0;
    digOutHiZ = 0;
}

// Drive auxiliary digital output high
void Rhd2000RegistersUsb3::setDigOutHigh()
{
    digOut = 1;
    digOutHiZ = 0;
}

// Set auxiliary digital output to high-impedance (HiZ) state
void Rhd2000RegistersUsb3::setDigOutHiZ()
{
    digOut = 0;
    digOutHiZ = 1;
}

// Enable or disable ADC auxiliary input 1
void Rhd2000RegistersUsb3::enableAux1(bool enabled)
{
    adcAux1En = (enabled ? 1 : 0);
}

// Enable or disable ADC auxiliary input 2
void Rhd2000RegistersUsb3::enableAux2(bool enabled)
{
    adcAux2En = (enabled ? 1 : 0);
}

// Enable or disable ADC auxiliary input 3
void Rhd2000RegistersUsb3::enableAux3(bool enabled)
{
    adcAux3En = (enabled ? 1 : 0);
}

// Enable or disable DSP offset removal filter
void Rhd2000RegistersUsb3::enableDsp(bool enabled)
{
    dspEn = (enabled ? 1 : 0);
}

// Set the DSP offset removal filter cutoff frequency as closely to the requested
// newDspCutoffFreq (in Hz) as possible; returns the actual cutoff frequency (in Hz).
double Rhd2000RegistersUsb3::setDspCutoffFreq(double newDspCutoffFreq)
{
    int n;
    double x, fCutoff[16], logNewDspCutoffFreq, logFCutoff[16], minLogDiff;
    const double Pi = 2*acos(0.0);

    fCutoff[0] = 0.0;   // We will not be using fCutoff[0], but we initialize it to be safe

    logNewDspCutoffFreq = log10(newDspCutoffFreq);

    // Generate table of all possible DSP cutoff frequencies
    for (n = 1; n < 16; ++n) {
        x = pow(2.0, (double) n);
        fCutoff[n] = sampleRate * log(x / (x - 1.0)) / (2*Pi);
        logFCutoff[n] = log10(fCutoff[n]);
        // cout << "  fCutoff[" << n << "] = " << fCutoff[n] << " Hz" << endl;
    }

    // Now find the closest value to the requested cutoff frequency (on a logarithmic scale)
    if (newDspCutoffFreq > fCutoff[1]) {
        dspCutoffFreq = 1;
    } else if (newDspCutoffFreq < fCutoff[15]) {
        dspCutoffFreq = 15;
    } else {
        minLogDiff = 10000000.0;
        for (n = 1; n < 16; ++n) {
            if (fabs(logNewDspCutoffFreq - logFCutoff[n]) < minLogDiff) {
                minLogDiff = fabs(logNewDspCutoffFreq - logFCutoff[n]);
                dspCutoffFreq = n;
            }
        }
    }

    return fCutoff[dspCutoffFreq];
}

// Returns the current value of the DSP offset removal cutoff frequency (in Hz).
double Rhd2000RegistersUsb3::getDspCutoffFreq() const
{
    double x;
    const double Pi = 2*acos(0.0);

    x = pow(2.0, (double) dspCutoffFreq);

    return sampleRate * log(x / (x - 1.0)) / (2*Pi);
}

// Enable or disable impedance checking mode
void Rhd2000RegistersUsb3::enableZcheck(bool enabled)
{
    zcheckEn = (enabled ? 1: 0);
}

// Power up or down impedance checking DAC
void Rhd2000RegistersUsb3::setZcheckDacPower(bool enabled)
{
    zcheckDacPower = (enabled ? 1 : 0);
}

// Select the series capacitor used to convert the voltage waveform generated by the on-chip
// DAC into an AC current waveform that stimulates a selected electrode for impedance testing
// (ZcheckCs100fF, ZcheckCs1pF, or Zcheck10pF).
void Rhd2000RegistersUsb3::setZcheckScale(ZcheckCs scale)
{
    switch (scale) {
        case ZcheckCs100fF:
            zcheckScale = 0x00;     // Cs = 0.1 pF
            break;
        case ZcheckCs1pF:
            zcheckScale = 0x01;     // Cs = 1.0 pF
            break;
        case ZcheckCs10pF:
            zcheckScale = 0x03;     // Cs = 10.0 pF
            break;
    }
}

// Select impedance testing of positive or negative amplifier inputs (RHD2216 only), based
// on the variable polarity (ZcheckPositiveInput or ZcheckNegativeInput)
void Rhd2000RegistersUsb3::setZcheckPolarity(ZcheckPolarity polarity)
{
    switch (polarity) {
        case ZcheckPositiveInput:
            zcheckSelPol = 0;
            break;
    case ZcheckNegativeInput:
            zcheckSelPol = 1;
            break;
    }
}

// Select the amplifier channel (0-63) for impedance testing.
int Rhd2000RegistersUsb3::setZcheckChannel(int channel)
{
    if (channel < 0 || channel > 63) {
        return -1;
    } else {
        zcheckSelect = channel;
        return zcheckSelect;
    }
}

// Power up or down selected amplifier on chip
void Rhd2000RegistersUsb3::setAmpPowered(int channel, bool powered)
{
    if (channel >= 0 && channel <= 63) {
        aPwr[channel] = (powered ? 1 : 0);
    }
}

// Power up all amplifiers on chip
void Rhd2000RegistersUsb3::powerUpAllAmps()
{
    for (int channel = 0; channel < 64; ++channel) {
        aPwr[channel] = 1;
    }
}

// Power down all amplifiers on chip
void Rhd2000RegistersUsb3::powerDownAllAmps()
{
    for (int channel = 0; channel < 64; ++channel) {
        aPwr[channel] = 0;
    }
}

// Returns the value of a selected RAM register (0-21) on the RHD2000 chip, based
// on the current register variables in the class instance.
int Rhd2000RegistersUsb3::getRegisterValue(int reg) const
{
    int regout;
    const int zcheckDac = 128;  // midrange

    switch (reg) {
    case 0:
        regout = (adcReferenceBw << 6) + (ampFastSettle << 5) + (ampVrefEnable << 4) +
                (adcComparatorBias << 2) + adcComparatorSelect;
        break;
    case 1:
        regout = (vddSenseEnable << 6) + adcBufferBias;
        break;
    case 2:
        regout = muxBias;
        break;
    case 3:
        regout = (muxLoad << 5) + (tempS2 << 4) + (tempS1 << 3) + (tempEn << 2) +
                (digOutHiZ << 1) + digOut;
        break;
    case 4:
        regout = (weakMiso << 7) + (twosComp << 6) + (absMode << 5) + (dspEn << 4) +
                dspCutoffFreq;
        break;
    case 5:
        regout = (zcheckDacPower << 6) + (zcheckLoad << 5) + (zcheckScale << 3) +
                (zcheckConnAll << 2) + (zcheckSelPol << 1) + zcheckEn;
        break;
    case 6:
        regout = zcheckDac;
        break;
    case 7:
        regout = zcheckSelect;
        break;
    case 8:
        regout = (offChipRH1 << 7) + rH1Dac1;
        break;
    case 9:
        regout = (adcAux1En << 7) + rH1Dac2;
        break;
    case 10:
        regout = (offChipRH2 << 7) + rH2Dac1;
        break;
    case 11:
        regout = (adcAux2En << 7) + rH2Dac2;
        break;
    case 12:
        regout = (offChipRL << 7) + rLDac1;
        break;
    case 13:
        regout = (adcAux3En << 7) + (rLDac3 << 6) + rLDac2;
        break;
    case 14:
        regout = (aPwr[7] << 7) + (aPwr[6] << 6) + (aPwr[5] << 5) + (aPwr[4] << 4) +
                (aPwr[3] << 3) + (aPwr[2] << 2) + (aPwr[1] << 1) + aPwr[0];
        break;
    case 15:
        regout = (aPwr[15] << 7) + (aPwr[14] << 6) + (aPwr[13] << 5) + (aPwr[12] << 4) +
                (aPwr[11] << 3) + (aPwr[10] << 2) + (aPwr[9] << 1) + aPwr[0];
        break;
    case 16:
        regout = (aPwr[23] << 7) + (aPwr[22] << 6) + (aPwr[21] << 5) + (aPwr[20] << 4) +
                (aPwr[19] << 3) + (aPwr[18] << 2) + (aPwr[17] << 1) + aPwr[16];
        break;
    case 17:
        regout = (aPwr[31] << 7) + (aPwr[30] << 6) + (aPwr[29] << 5) + (aPwr[28] << 4) +
                (aPwr[27] << 3) + (aPwr[26] << 2) + (aPwr[25] << 1) + aPwr[24];
        break;
    case 18:
        regout = (aPwr[39] << 7) + (aPwr[38] << 6) + (aPwr[37] << 5) + (aPwr[36] << 4) +
                (aPwr[35] << 3) + (aPwr[34] << 2) + (aPwr[33] << 1) + aPwr[32];
        break;
    case 19:
        regout = (aPwr[47] << 7) + (aPwr[46] << 6) + (aPwr[45] << 5) + (aPwr[44] << 4) +
                (aPwr[43] << 3) + (aPwr[42] << 2) + (aPwr[41] << 1) + aPwr[40];
        break;
    case 20:
        regout = (aPwr[55] << 7) + (aPwr[54] << 6) + (aPwr[53] << 5) + (aPwr[52] << 4) +
                (aPwr[51] << 3) + (aPwr[50] << 2) + (aPwr[49] << 1) + aPwr[48];
        break;
    case 21:
        regout = (aPwr[63] << 7) + (aPwr[62] << 6) + (aPwr[61] << 5) + (aPwr[60] << 4) +
                (aPwr[59] << 3) + (aPwr[58] << 2) + (aPwr[57] << 1) + aPwr[56];
        break;
    default:
        regout = -1;
    }
    return regout;
}

// Returns the value of the RH1 resistor (in ohms) corresponding to a particular upper
// bandwidth value (in Hz).
double Rhd2000RegistersUsb3::rH1FromUpperBandwidth(double upperBandwidth) const
{
    double log10f = log10(upperBandwidth);

    return 0.9730 * pow(10.0, (8.0968 - 1.1892 * log10f + 0.04767 * log10f * log10f));
}

// Returns the value of the RH2 resistor (in ohms) corresponding to a particular upper
// bandwidth value (in Hz).
double Rhd2000RegistersUsb3::rH2FromUpperBandwidth(double upperBandwidth) const
{
    double log10f = log10(upperBandwidth);

    return 1.0191 * pow(10.0, (8.1009 - 1.0821 * log10f + 0.03383 * log10f * log10f));
}

// Returns the value of the RL resistor (in ohms) corresponding to a particular lower
// bandwidth value (in Hz).
double Rhd2000RegistersUsb3::rLFromLowerBandwidth(double lowerBandwidth) const
{
    double log10f = log10(lowerBandwidth);

    if (lowerBandwidth < 4.0) {
        return 1.0061 * pow(10.0, (4.9391 - 1.2088 * log10f + 0.5698 * log10f * log10f +
                                   0.1442 * log10f * log10f * log10f));
    } else {
        return 1.0061 * pow(10.0, (4.7351 - 0.5916 * log10f + 0.08482 * log10f * log10f));
    }
}

// Returns the amplifier upper bandwidth (in Hz) corresponding to a particular value
// of the resistor RH1 (in ohms).
double Rhd2000RegistersUsb3::upperBandwidthFromRH1(double rH1) const
{
    double a, b, c;

    a = 0.04767;
    b = -1.1892;
    c = 8.0968 - log10(rH1/0.9730);

    return pow(10.0, ((-b - sqrt(b * b - 4 * a * c))/(2 * a)));
}

// Returns the amplifier upper bandwidth (in Hz) corresponding to a particular value
// of the resistor RH2 (in ohms).
double Rhd2000RegistersUsb3::upperBandwidthFromRH2(double rH2) const
{
    double a, b, c;

    a = 0.03383;
    b = -1.0821;
    c = 8.1009 - log10(rH2/1.0191);

    return pow(10.0, ((-b - sqrt(b * b - 4 * a * c))/(2 * a)));
}

// Returns the amplifier lower bandwidth (in Hz) corresponding to a particular value
// of the resistor RL (in ohms).
double Rhd2000RegistersUsb3::lowerBandwidthFromRL(double rL) const
{
    double a, b, c;

    // Quadratic fit below is invalid for values of RL less than 5.1 kOhm
    if (rL < 5100.0) {
        rL = 5100.0;
    }

    if (rL < 30000.0) {
        a = 0.08482;
        b = -0.5916;
        c = 4.7351 - log10(rL/1.0061);
    } else {
        a = 0.3303;
        b = -1.2100;
        c = 4.9873 - log10(rL/1.0061);
    }

    return pow(10.0, ((-b - sqrt(b * b - 4 * a * c))/(2 * a)));
}

// Sets the on-chip RH1 and RH2 DAC values appropriately to set a particular amplifier
// upper bandwidth (in Hz).  Returns an estimate of the actual upper bandwidth achieved.
double Rhd2000RegistersUsb3::setUpperBandwidth(double upperBandwidth)
{
    const double RH1Base = 2200.0;
    const double RH1Dac1Unit = 600.0;
    const double RH1Dac2Unit = 29400.0;
    const int RH1Dac1Steps = 63;
    const int RH1Dac2Steps = 31;

    const double RH2Base = 8700.0;
    const double RH2Dac1Unit = 763.0;
    const double RH2Dac2Unit = 38400.0;
    const int RH2Dac1Steps = 63;
    const int RH2Dac2Steps = 31;

    double actualUpperBandwidth;
    double rH1Target, rH2Target;
    double rH1Actual, rH2Actual;
    int i;

    // Upper bandwidths higher than 30 kHz don't work well with the RHD2000 amplifiers
    if (upperBandwidth > 30000.0) {
        upperBandwidth = 30000.0;
    }

    rH1Target = rH1FromUpperBandwidth(upperBandwidth);

    rH1Dac1 = 0;
    rH1Dac2 = 0;
    rH1Actual = RH1Base;

    for (i = 0; i < RH1Dac2Steps; ++i) {
        if (rH1Actual < rH1Target - (RH1Dac2Unit - RH1Dac1Unit / 2)) {
            rH1Actual += RH1Dac2Unit;
            ++rH1Dac2;
        }
    }

    for (i = 0; i < RH1Dac1Steps; ++i) {
        if (rH1Actual < rH1Target - (RH1Dac1Unit / 2)) {
            rH1Actual += RH1Dac1Unit;
            ++rH1Dac1;
        }
    }

    rH2Target = rH2FromUpperBandwidth(upperBandwidth);

    rH2Dac1 = 0;
    rH2Dac2 = 0;
    rH2Actual = RH2Base;

    for (i = 0; i < RH2Dac2Steps; ++i) {
        if (rH2Actual < rH2Target - (RH2Dac2Unit - RH2Dac1Unit / 2)) {
            rH2Actual += RH2Dac2Unit;
            ++rH2Dac2;
        }
    }

    for (i = 0; i < RH2Dac1Steps; ++i) {
        if (rH2Actual < rH2Target - (RH2Dac1Unit / 2)) {
            rH2Actual += RH2Dac1Unit;
            ++rH2Dac1;
        }
    }

    double actualUpperBandwidth1, actualUpperBandwidth2;

    actualUpperBandwidth1 = upperBandwidthFromRH1(rH1Actual);
    actualUpperBandwidth2 = upperBandwidthFromRH2(rH2Actual);

    // Upper bandwidth estimates calculated from actual RH1 value and acutal RH2 value
    // should be very close; we will take their geometric mean to get a single
    // number.
    actualUpperBandwidth = sqrt(actualUpperBandwidth1 * actualUpperBandwidth2);

    /*
    cout << endl;
    cout << "Rhd2000RegistersUsb3::setUpperBandwidth" << endl;
    cout << fixed << setprecision(1);

    cout << "RH1 DAC2 = " << rH1Dac2 << ", DAC1 = " << rH1Dac1 << endl;
    cout << "RH1 target: " << rH1Target << " Ohms" << endl;
    cout << "RH1 actual: " << rH1Actual << " Ohms" << endl;

    cout << "RH2 DAC2 = " << rH2Dac2 << ", DAC1 = " << rH2Dac1 << endl;
    cout << "RH2 target: " << rH2Target << " Ohms" << endl;
    cout << "RH2 actual: " << rH2Actual << " Ohms" << endl;

    cout << "Upper bandwidth target: " << upperBandwidth << " Hz" << endl;
    cout << "Upper bandwidth actual: " << actualUpperBandwidth << " Hz" << endl;

    cout << endl;
    cout << setprecision(6);
    cout.unsetf(ios::floatfield);
    */

    return actualUpperBandwidth;
}

// Sets the on-chip RL DAC values appropriately to set a particular amplifier
// lower bandwidth (in Hz).  Returns an estimate of the actual lower bandwidth achieved.
double Rhd2000RegistersUsb3::setLowerBandwidth(double lowerBandwidth)
{
    const double RLBase = 3500.0;
    const double RLDac1Unit = 175.0;
    const double RLDac2Unit = 12700.0;
    const double RLDac3Unit = 3000000.0;
    const int RLDac1Steps = 127;
    const int RLDac2Steps = 63;

    double actualLowerBandwidth;
    double rLTarget;
    double rLActual;
    int i;

    // Lower bandwidths higher than 1.5 kHz don't work well with the RHD2000 amplifiers
    if (lowerBandwidth > 1500.0) {
        lowerBandwidth = 1500.0;
    }

    rLTarget = rLFromLowerBandwidth(lowerBandwidth);

    rLDac1 = 0;
    rLDac2 = 0;
    rLDac3 = 0;
    rLActual = RLBase;

    if (lowerBandwidth < 0.15) {
        rLActual += RLDac3Unit;
        ++rLDac3;
    }

    for (i = 0; i < RLDac2Steps; ++i) {
        if (rLActual < rLTarget - (RLDac2Unit - RLDac1Unit / 2)) {
            rLActual += RLDac2Unit;
            ++rLDac2;
        }
    }

    for (i = 0; i < RLDac1Steps; ++i) {
        if (rLActual < rLTarget - (RLDac1Unit / 2)) {
            rLActual += RLDac1Unit;
            ++rLDac1;
        }
    }

    actualLowerBandwidth = lowerBandwidthFromRL(rLActual);

    /*
    cout << endl;
    cout << fixed << setprecision(1);
    cout << "Rhd2000RegistersUsb3::setLowerBandwidth" << endl;

    cout << "RL DAC3 = " << rLDac3 << ", DAC2 = " << rLDac2 << ", DAC1 = " << rLDac1 << endl;
    cout << "RL target: " << rLTarget << " Ohms" << endl;
    cout << "RL actual: " << rLActual << " Ohms" << endl;

    cout << setprecision(3);

    cout << "Lower bandwidth target: " << lowerBandwidth << " Hz" << endl;
    cout << "Lower bandwidth actual: " << actualLowerBandwidth << " Hz" << endl;

    cout << endl;
    cout << setprecision(6);
    cout.unsetf(ios::floatfield);
    */

    return actualLowerBandwidth;
}

// Return a 16-bit MOSI command (CALIBRATE or CLEAR)
int Rhd2000RegistersUsb3::createRhd2000Command(Rhd2000CommandType commandType)
{
    switch (commandType) {
        case Rhd2000CommandCalibrate:
            return 0x5500;   // 0101010100000000
            break;
        case Rhd2000CommandCalClear:
            return 0x6a00;   // 0110101000000000
            break;
        default:
        cerr << "Error in Rhd2000RegistersUsb3::createRhd2000Command: " <<
                "Only 'Calibrate' or 'Clear Calibration' commands take zero arguments." << endl;
            return -1;
    }
}

// Return a 16-bit MOSI command (CONVERT or READ)
int Rhd2000RegistersUsb3::createRhd2000Command(Rhd2000CommandType commandType, int arg1)
{
    switch (commandType) {
        case Rhd2000CommandConvert:
            if (arg1 < 0 || arg1 > 63) {
                cerr << "Error in Rhd2000RegistersUsb3::createRhd2000Command: " <<
                        "Channel number out of range." << endl;
                return -1;
            }
            return 0x0000 + (arg1 << 8);  // 00cccccc0000000h; if the command is 'Convert',
                                          // arg1 is the channel number
        case Rhd2000CommandRegRead:
            if (arg1 < 0 || arg1 > 63) {
                cerr << "Error in Rhd2000RegistersUsb3::createRhd2000Command: " <<
                        "Register address out of range." << endl;
                return -1;
            }
            return 0xc000 + (arg1 << 8);  // 11rrrrrr00000000; if the command is 'Register Read',
                                          // arg1 is the register address
            break;
        default:
            cerr << "Error in Rhd2000RegistersUsb3::createRhd2000Command: " <<
                    "Only 'Convert' and 'Register Read' commands take one argument." << endl;
            return -1;
    }
}

// Return a 16-bit MOSI command (WRITE)
int Rhd2000RegistersUsb3::createRhd2000Command(Rhd2000CommandType commandType, int arg1, int arg2)
{
    switch (commandType) {
        case Rhd2000CommandRegWrite:
            if (arg1 < 0 || arg1 > 63) {
                cerr << "Error in Rhd2000RegistersUsb3::createRhd2000Command: " <<
                        "Register address out of range." << endl;
                return -1;
            }
            if (arg2 < 0 || arg2 > 255) {
                cerr << "Error in Rhd2000RegistersUsb3::createRhd2000Command: " <<
                        "Register data out of range." << endl;
                return -1;
            }
            return 0x8000 + (arg1 << 8) + arg2; // 10rrrrrrdddddddd; if the command is 'Register Write',
                                                // arg1 is the register address and arg1 is the data
            break;
        default:
            cerr << "Error in Rhd2000RegistersUsb3::createRhd2000Command: " <<
                    "Only 'Register Write' commands take two arguments." << endl;
            return -1;
    }
}


// Create a list of 128 commands to program most RAM registers on a RHD2000 chip, read those values
// back to confirm programming, read ROM registers, and (if calibrate == true) run ADC calibration.
// Returns the length of the command list.
int Rhd2000RegistersUsb3::createCommandListRegisterConfig(vector<int> &commandList, bool calibrate)
{
    int i;

    commandList.clear();    // if command list already exists, erase it and start a new one

    // Start with a few dummy commands in case chip is still powering up
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 63));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 63));

    // Program RAM registers
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite,  0, getRegisterValue( 0)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite,  1, getRegisterValue( 1)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite,  2, getRegisterValue( 2)));
    // Don't program Register 3 (MUX Load, Temperature Sensor, and Auxiliary Digital Output);
    // control temperature sensor in another command stream
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite,  4, getRegisterValue( 4)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite,  5, getRegisterValue( 5)));
    // Don't program Register 6 (Impedance Check DAC) here; create DAC waveform in another command stream
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite,  7, getRegisterValue( 7)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite,  8, getRegisterValue( 8)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite,  9, getRegisterValue( 9)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 10, getRegisterValue(10)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 11, getRegisterValue(11)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 12, getRegisterValue(12)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 13, getRegisterValue(13)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 14, getRegisterValue(14)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 15, getRegisterValue(15)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 16, getRegisterValue(16)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 17, getRegisterValue(17)));

    // Read ROM registers
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 63));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 62));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 61));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 60));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 59));

    // Read chip name from ROM
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 48));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 49));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 50));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 51));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 52));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 53));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 54));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 55));

    // Read Intan name from ROM
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 40));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 41));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 42));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 43));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 44));

    // Read back RAM registers to confirm programming
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead,  0));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead,  1));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead,  2));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead,  3));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead,  4));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead,  5));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead,  6));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead,  7));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead,  8));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead,  9));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 10));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 11));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 12));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 13));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 14));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 15));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 16));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 17));

    // Optionally, run ADC calibration (should only be run once after board is plugged in)
    if (calibrate) {
        commandList.push_back(createRhd2000Command(Rhd2000CommandCalibrate));
    } else {
        commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 63));
    }

    // Added in Version 1.2:
    // Program amplifier 31-63 power up/down registers in case a RHD2164 is connected
    // Note: We don't read these registers back, since they are only 'visible' on MISO B.
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 18, getRegisterValue(18)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 19, getRegisterValue(19)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 20, getRegisterValue(20)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 21, getRegisterValue(21)));

    // End with a dummy command
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 63));

    for (i = 0; i < (128-60); ++i) {
        commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 63));
    }

    return static_cast<int>(commandList.size());
}

// Create a list of 128 commands to sample auxiliary ADC inputs, temperature sensor, and supply
// voltage sensor.  One temperature reading (one sample of ResultA and one sample of ResultB)
// is taken during this 128-command sequence.  One supply voltage sample is taken.  Auxiliary
// ADC inputs are continuously sampled at 1/4 the amplifier sampling rate.
//
// Since this command list consists of writing to Register 3, it also sets the state of the
// auxiliary digital output.  If the digital output value needs to be changed dynamically,
// then variations of this command list need to be generated for each state and programmed into
// different RAM banks, and the appropriate command list selected at the right time.
//
// Returns the length of the command list.
int Rhd2000RegistersUsb3::createCommandListTempSensor(vector<int> &commandList)
{
    int i;

    commandList.clear();    // if command list already exists, erase it and start a new one

    tempEn = 1;

    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 32));     // sample AuxIn1
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 33));     // sample AuxIn2
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 34));     // sample AuxIn3
    tempS1 = tempEn;
    tempS2 = 0;
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));

    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 32));     // sample AuxIn1
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 33));     // sample AuxIn2
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 34));     // sample AuxIn3
    tempS1 = tempEn;
    tempS2 = tempEn;
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));

    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 32));     // sample AuxIn1
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 33));     // sample AuxIn2
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 34));     // sample AuxIn3
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 49));     // sample Temperature Sensor

    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 32));     // sample AuxIn1
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 33));     // sample AuxIn2
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 34));     // sample AuxIn3
    tempS1 = 0;
    tempS2 = tempEn;
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));

    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 32));     // sample AuxIn1
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 33));     // sample AuxIn2
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 34));     // sample AuxIn3
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 49));     // sample Temperature Sensor

    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 32));     // sample AuxIn1
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 33));     // sample AuxIn2
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 34));     // sample AuxIn3
    tempS1 = 0;
    tempS2 = 0;
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));

    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 32));     // sample AuxIn1
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 33));     // sample AuxIn2
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 34));     // sample AuxIn3
    commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 48));     // sample Supply Voltage Sensor

    for (i = 0; i < 25; ++i) {
        commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 32));     // sample AuxIn1
        commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 33));     // sample AuxIn2
        commandList.push_back(createRhd2000Command(Rhd2000CommandConvert, 34));     // sample AuxIn3
        commandList.push_back(createRhd2000Command(Rhd2000CommandRegRead, 63));      // dummy command
    }

    return static_cast<int>(commandList.size());
}

// Create a list of 128 commands to update Register 3 (controlling the auxiliary digital ouput
// pin) every sampling period.
//
// Since this command list consists of writing to Register 3, it also sets the state of the
// on-chip temperature sensor.  The temperature sensor settings are therefore changed throughout
// this command list to coordinate with the 128-command list generated by createCommandListTempSensor().
//
// Returns the length of the command list.
int Rhd2000RegistersUsb3::createCommandListUpdateDigOut(vector<int> &commandList)
{
    int i;

    commandList.clear();    // if command list already exists, erase it and start a new one

    tempEn = 1;

    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    tempS1 = tempEn;
    tempS2 = 0;
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));

    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    tempS1 = tempEn;
    tempS2 = tempEn;
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));

    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));

    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    tempS1 = 0;
    tempS2 = tempEn;
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));

    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));

    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    tempS1 = 0;
    tempS2 = 0;
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));

    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));

    for (i = 0; i < 25; ++i) {
        commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
        commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
        commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
        commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 3, getRegisterValue(3)));
    }

    return static_cast<int>(commandList.size());
}

// Create a list of up to 1024 commands to generate a sine wave of particular frequency (in Hz) and
// amplitude (in DAC steps, 0-128) using the on-chip impedance testing voltage DAC.  If frequency is set to zero,
// a DC baseline waveform is created.
// Returns the length of the command list.
int Rhd2000RegistersUsb3::createCommandListZcheckDac(vector<int> &commandList, double frequency, double amplitude)
{
    int i, period, value;
    double t;
    const double Pi = 2*acos(0.0);

    commandList.clear();    // if command list already exists, erase it and start a new one

    if (amplitude < 0.0 || amplitude > 128.0) {
        cerr << "Error in Rhd2000RegistersUsb3::createCommandListZcheckDac: Amplitude out of range." << endl;
        return -1;
    }
    if (frequency < 0.0) {
        cerr << "Error in Rhd2000RegistersUsb3::createCommandListZcheckDac: Negative frequency not allowed." << endl;
        return -1;
    } else if (frequency > sampleRate / 4.0) {
        cerr << "Error in Rhd2000RegistersUsb3::createCommandListZcheckDac: " <<
                "Frequency too high relative to sampling rate." << endl;
        return -1;
    }
    if (frequency == 0.0) {
        for (i = 0; i < MaxCommandLength; ++i) {
            commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 6, 128));
        }
    } else {
        period = (int) floor(sampleRate / frequency + 0.5);
        if (period > MaxCommandLength) {
            cerr << "Error in Rhd2000RegistersUsb3::createCommandListZcheckDac: Frequency too low." << endl;
            return -1;
        } else {
            t = 0.0;
            for (i = 0; i < period; ++i) {
                value = (int) floor(amplitude * sin(2 * Pi * frequency * t) + 128.0 + 0.5);
                if (value < 0) {
                    value = 0;
                } else if (value > 255) {
                    value = 255;
                }
                commandList.push_back(createRhd2000Command(Rhd2000CommandRegWrite, 6, value));
                t += 1.0 / sampleRate;
            }
        }
    }

    return static_cast<int>(commandList.size());
}
