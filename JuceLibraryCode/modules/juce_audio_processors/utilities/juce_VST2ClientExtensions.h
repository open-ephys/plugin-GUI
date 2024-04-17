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

/**
    An interface to allow an AudioProcessor to implement extended VST2-specific functionality.

    To use this class, create an object that inherits from it, implement the methods, then return
    a pointer to the object in your AudioProcessor::getVST2ClientExtensions() method.

    @see AudioProcessor, AAXClientExtensions, VST3ClientExtensions

    @tags{Audio}
*/
struct VST2ClientExtensions
{
    virtual ~VST2ClientExtensions() = default;

    /** This is called by the VST plug-in wrapper when it receives unhandled
        plug-in "can do" calls from the host.
    */
    virtual pointer_sized_int handleVstPluginCanDo (int32 index,
                                                    pointer_sized_int value,
                                                    void* ptr,
                                                    float opt);

    /** This is called by the VST plug-in wrapper when it receives unhandled
        vendor specific calls from the host.
    */
    virtual pointer_sized_int handleVstManufacturerSpecific (int32 index,
                                                             pointer_sized_int value,
                                                             void* ptr,
                                                             float opt) = 0;

    /** The host callback function type. */
    using VstHostCallbackType = pointer_sized_int (int32 opcode,
                                                   int32 index,
                                                   pointer_sized_int value,
                                                   void* ptr,
                                                   float opt);

    /** This is called once by the VST plug-in wrapper after its constructor.
        You can use the supplied function to query the VST host.
    */
    virtual void handleVstHostCallbackAvailable (std::function<VstHostCallbackType>&& callback);
};

using VSTCallbackHandler [[deprecated ("replace with VST2ClientExtensions")]] = VST2ClientExtensions;

} // namespace juce
