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

String String::fromCFString (CFStringRef cfString)
{
    if (cfString == nullptr)
        return {};

    CFRange range = { 0, CFStringGetLength (cfString) };
    CFIndex bytesNeeded = 0;
    CFStringGetBytes (cfString, range, kCFStringEncodingUTF8, 0, false, nullptr, 0, &bytesNeeded);

    HeapBlock<UInt8> utf8 (bytesNeeded + 1);
    CFStringGetBytes (cfString, range, kCFStringEncodingUTF8, 0, false, utf8, bytesNeeded + 1, nullptr);

    return String (CharPointer_UTF8 ((const CharPointer_UTF8::CharType*) utf8.get()),
                   CharPointer_UTF8 ((const CharPointer_UTF8::CharType*) utf8.get() + bytesNeeded));
}

CFStringRef String::toCFString() const
{
    const char* const utf8 = toRawUTF8();

    if (CFStringRef result = CFStringCreateWithBytes (kCFAllocatorDefault, (const UInt8*) utf8,
                                                      (CFIndex) strlen (utf8), kCFStringEncodingUTF8, false))
        return result;

    // If CFStringCreateWithBytes fails, it probably means there was a UTF8 format
    // error, so we'll return an empty string rather than a null pointer.
    return String().toCFString();
}

String String::convertToPrecomposedUnicode() const
{
   #if JUCE_IOS
    JUCE_AUTORELEASEPOOL
    {
        return nsStringToJuce ([juceStringToNS (*this) precomposedStringWithCanonicalMapping]);
    }
   #else
    UnicodeMapping map;

    map.unicodeEncoding = CreateTextEncoding (kTextEncodingUnicodeDefault,
                                              kUnicodeNoSubset,
                                              kTextEncodingDefaultFormat);

    map.otherEncoding = CreateTextEncoding (kTextEncodingUnicodeDefault,
                                            kUnicodeCanonicalCompVariant,
                                            kTextEncodingDefaultFormat);

    map.mappingVersion = kUnicodeUseLatestMapping;

    UnicodeToTextInfo conversionInfo = {};
    String result;

    if (CreateUnicodeToTextInfo (&map, &conversionInfo) == noErr)
    {
        const size_t bytesNeeded = CharPointer_UTF16::getBytesRequiredFor (getCharPointer());

        HeapBlock<char> tempOut;
        tempOut.calloc (bytesNeeded + 4);

        ByteCount bytesRead = 0;
        ByteCount outputBufferSize = 0;

        if (ConvertFromUnicodeToText (conversionInfo,
                                      bytesNeeded, (ConstUniCharArrayPtr) toUTF16().getAddress(),
                                      kUnicodeDefaultDirectionMask,
                                      0, {}, {}, {},
                                      bytesNeeded, &bytesRead,
                                      &outputBufferSize, tempOut) == noErr)
        {
            result = String (CharPointer_UTF16 (reinterpret_cast<CharPointer_UTF16::CharType*> (tempOut.get())));
        }

        DisposeUnicodeToTextInfo (&conversionInfo);
    }

    return result;
   #endif
}

} // namespace juce
