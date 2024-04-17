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
class DirectWriteCustomFontCollectionLoader : public ComBaseClassHelper<IDWriteFontCollectionLoader>
{
private:
    struct FontRawData
    {
        const void* data;
        size_t      numBytes;
    };

    struct FontFileStream : public ComBaseClassHelper<IDWriteFontFileStream>
    {
        FontFileStream (FontRawData const& rawData_)
            : rawData (rawData_)
        {
        }
        ~FontFileStream() override = default;

        JUCE_COMRESULT GetFileSize (UINT64* fileSize) noexcept override
        {
            *fileSize = rawData.numBytes;
            return S_OK;
        }

        JUCE_COMRESULT GetLastWriteTime (UINT64* lastWriteTime) noexcept override
        {
            *lastWriteTime = 0;
            return S_OK;
        }

        JUCE_COMRESULT ReadFileFragment (void const** fragmentStart,
                                         UINT64       fileOffset,
                                         UINT64       fragmentSize,
                                         void**       fragmentContext) noexcept override
        {
            if (fileOffset + fragmentSize > rawData.numBytes)
            {
                *fragmentStart   = nullptr;
                *fragmentContext = nullptr;
                return E_INVALIDARG;
            }

            *fragmentStart   = addBytesToPointer (rawData.data, fileOffset);
            *fragmentContext = this;
            return S_OK;
        }

        void WINAPI ReleaseFileFragment (void* /*fragmentContext*/) noexcept override {}

        FontRawData rawData;
    };

    struct FontFileLoader : public ComBaseClassHelper<IDWriteFontFileLoader>
    {
        Array<FontRawData> rawDataArray;

        FontFileLoader()           = default;
        ~FontFileLoader() override = default;

        HRESULT WINAPI CreateStreamFromKey (void const* fontFileReferenceKey,
                                            UINT32 fontFileReferenceKeySize,
                                            IDWriteFontFileStream** fontFileStream) noexcept override
        {
            jassert (sizeof (char*) == fontFileReferenceKeySize);
            if (sizeof (char*) == fontFileReferenceKeySize)
            {
                const char* referenceKey = *(const char**) fontFileReferenceKey;
                for (auto const& rawData : rawDataArray)
                {
                    if (referenceKey == rawData.data)
                    {
                        *fontFileStream = new FontFileStream { rawData };
                        return S_OK;
                    }
                }
            }

            *fontFileStream = nullptr;
            return E_INVALIDARG;
        }
    } fontFileLoader;

    struct FontFileEnumerator : public ComBaseClassHelper<IDWriteFontFileEnumerator>
    {
        FontFileEnumerator (IDWriteFactory* factory_, FontFileLoader& fontFileLoader_)
            : factory (factory_),
              fontFileLoader (fontFileLoader_)
        {
        }

        ~FontFileEnumerator() override = default;

        HRESULT WINAPI GetCurrentFontFile (IDWriteFontFile** fontFile) noexcept override
        {
            if (rawDataIndex < 0 || rawDataIndex >= fontFileLoader.rawDataArray.size())
            {
                *fontFile = nullptr;
                return E_FAIL;
            }

            auto referenceKey = fontFileLoader.rawDataArray[rawDataIndex].data;
            return factory->CreateCustomFontFileReference (&referenceKey,
                                                           sizeof (referenceKey),
                                                           &fontFileLoader,
                                                           fontFile);
        }

        HRESULT WINAPI MoveNext (BOOL* hasCurrentFile) noexcept override
        {
            ++rawDataIndex;
            *hasCurrentFile = rawDataIndex < fontFileLoader.rawDataArray.size() ? TRUE : FALSE;
            return S_OK;
        }

        IDWriteFactory* factory;
        FontFileLoader& fontFileLoader;
        int             rawDataIndex = -1;
    };

public:
    DirectWriteCustomFontCollectionLoader (const void* data, size_t dataSize)
    {
        fontFileLoader.rawDataArray.add ({ data, dataSize });
    }
    ~DirectWriteCustomFontCollectionLoader() override = default;

    HRESULT WINAPI CreateEnumeratorFromKey (IDWriteFactory* factory,
                                            void const* collectionKey,
                                            UINT32 collectionKeySize,
                                            IDWriteFontFileEnumerator** fontFileEnumerator) noexcept override
    {
        jassertquiet (collectionKeySize == sizeof (key));
        jassertquiet (0 == std::memcmp (collectionKey, &key, collectionKeySize));

        *fontFileEnumerator = new FontFileEnumerator { factory, fontFileLoader };
        return S_OK;
    }

    IDWriteFontFileLoader* getFontFileLoader() const
    {
        return (IDWriteFontFileLoader*) &fontFileLoader;
    }

    bool hasRawData (const void* data, size_t /*dataSize*/)
    {
        return fontFileLoader.rawDataArray.getFirst().data == data;
    }

    ComSmartPtr<IDWriteFontCollection> customFontCollection;
    int64 const                        key = Time::getHighResolutionTicks();
};

} // namespace juce
