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
    An AudioSource which takes some float audio data as an input.

    @tags{Audio}
*/
class JUCE_API MemoryAudioSource   : public PositionableAudioSource
{
public:
    //==============================================================================
    /**  Creates a MemoryAudioSource by providing an audio buffer.

         If copyMemory is true then the buffer will be copied into an internal
         buffer which will be owned by the MemoryAudioSource. If copyMemory is
         false, then you must ensure that the lifetime of the audio buffer is
         at least as long as the MemoryAudioSource.
    */
    MemoryAudioSource (AudioBuffer<float>& audioBuffer, bool copyMemory, bool shouldLoop = false);

    //==============================================================================
    /** Implementation of the AudioSource method. */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    /** Implementation of the AudioSource method. */
    void releaseResources() override;

    /** Implementation of the AudioSource method. */
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;

    //==============================================================================
    /** Implementation of the PositionableAudioSource method. */
    void setNextReadPosition (int64 newPosition) override;

    /** Implementation of the PositionableAudioSource method. */
    int64 getNextReadPosition() const override;

    /** Implementation of the PositionableAudioSource method. */
    int64 getTotalLength() const override;

    //==============================================================================
    /** Implementation of the PositionableAudioSource method. */
    bool isLooping() const override;

    /** Implementation of the PositionableAudioSource method. */
    void setLooping (bool shouldLoop) override;

private:
    //==============================================================================
    AudioBuffer<float> buffer;
    int position = 0;
    bool isCurrentlyLooping;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryAudioSource)
};

} // namespace juce
