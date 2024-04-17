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

//==============================================================================
/**
    An AudioSource that performs an IIR filter on another source.

    @tags{Audio}
*/
class JUCE_API  IIRFilterAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a IIRFilterAudioSource for a given input source.

        @param inputSource              the input source to read from - this must not be null
        @param deleteInputWhenDeleted   if true, the input source will be deleted when
                                        this object is deleted
    */
    IIRFilterAudioSource (AudioSource* inputSource,
                          bool deleteInputWhenDeleted);

    /** Destructor. */
    ~IIRFilterAudioSource() override;

    //==============================================================================
    /** Changes the filter to use the same parameters as the one being passed in. */
    void setCoefficients (const IIRCoefficients& newCoefficients);

    /** Calls IIRFilter::makeInactive() on all the filters being used internally. */
    void makeInactive();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const AudioSourceChannelInfo&) override;

private:
    //==============================================================================
    OptionalScopedPointer<AudioSource> input;
    OwnedArray<IIRFilter> iirFilters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IIRFilterAudioSource)
};

} // namespace juce
