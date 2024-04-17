/*
  ==============================================================================

   This file is part of the JUCE 8 technical preview.
   Copyright (c) Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class iOSAudioIODeviceType;

class iOSAudioIODevice : public AudioIODevice
{
public:
    //==============================================================================
    String open (const BigInteger&, const BigInteger&, double, int) override;
    void close() override;

    void start (AudioIODeviceCallback*) override;
    void stop() override;

    Array<double> getAvailableSampleRates() override;
    Array<int> getAvailableBufferSizes() override;

    bool setAudioPreprocessingEnabled (bool) override;

    //==============================================================================
    bool isPlaying() override;
    bool isOpen() override;
    String getLastError() override;

    //==============================================================================
    StringArray getOutputChannelNames() override;
    StringArray getInputChannelNames() override;

    int getDefaultBufferSize() override;
    int getCurrentBufferSizeSamples() override;

    double getCurrentSampleRate() override;

    int getCurrentBitDepth() override;

    BigInteger getActiveOutputChannels() const override;
    BigInteger getActiveInputChannels() const override;

    int getOutputLatencyInSamples() override;
    int getInputLatencyInSamples() override;

    int getXRunCount() const noexcept override;

    AudioWorkgroup getWorkgroup() const override;

    //==============================================================================
    void setMidiMessageCollector (MidiMessageCollector*);
    AudioPlayHead* getAudioPlayHead() const;

    //==============================================================================
    bool isInterAppAudioConnected() const;
   #if JUCE_MODULE_AVAILABLE_juce_graphics
    Image getIcon (int size);
   #endif
    void switchApplication();

private:
    //==============================================================================
    iOSAudioIODevice (iOSAudioIODeviceType*, const String&, const String&);

    //==============================================================================
    friend class iOSAudioIODeviceType;
    friend struct AudioSessionHolder;

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE (iOSAudioIODevice)
};

} // namespace juce
