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
//
// DirectWrite
//
// FontCollectionCollection is a collection of IDWriteFontCollection objects
//
// There's quite a bit of code that relies on the DirectWrite system font collection;
// rather than rewrite all that code, FontCollectionCollection aggregates the system fonts
// along with any custom fonts to present a unified single font collection to the user.
//
struct DirectWrite::FontCollectionCollection : public ComBaseClassHelper<IDWriteFontCollection>
{
    FontCollectionCollection(IDWriteFactory* factory)
    {
        factory->GetSystemFontCollection(systemFonts.resetAndGetPointerAddress());
        if (systemFonts)
        {
            collections.emplace_back(systemFonts);
        }
    }

    ~FontCollectionCollection() override = default;

    STDMETHOD_(UINT32, GetFontFamilyCount()) override
    {
        UINT32 total = 0;
        for (auto collection : collections)
        {
            if (collection)
                total += collection->GetFontFamilyCount();
        }

        return total;
    }

    STDMETHOD(GetFontFamily)(UINT32 index, IDWriteFontFamily** fontFamily) override
    {
        for (auto collection : collections)
        {
            if (index < collection->GetFontFamilyCount())
                return collection->GetFontFamily(index, fontFamily);

            index -= collection->GetFontFamilyCount();
        }

        *fontFamily = nullptr;
        return E_FAIL;
    }

    STDMETHOD(FindFamilyName)(const WCHAR* familyName, UINT32* index, BOOL* exists) override
    {
        UINT32 baseIndex = 0;
        for (auto collection : collections)
        {
            if (collection)
            {
                if (auto hr = collection->FindFamilyName(familyName, index, exists); *exists || FAILED(hr))
                {
                    *index += baseIndex;
                    return hr;
                }

                baseIndex += collection->GetFontFamilyCount();
            }
        }

        *index = UINT_MAX;
        *exists = FALSE;
        return S_OK;
    }

    STDMETHOD(GetFontFromFontFace)(IDWriteFontFace* fontFace, IDWriteFont** font) noexcept override
    {
        for (auto collection : collections)
        {
            if (auto hr = collection->GetFontFromFontFace(fontFace, font); SUCCEEDED(hr))
                return hr;
        }

        return DWRITE_E_NOFONT;
    }

    ComSmartPtr<IDWriteFontCollection> systemFonts;
    OwnedArray<DirectWriteCustomFontCollectionLoader> customFontCollectionLoaders;
    std::vector<ComSmartPtr<IDWriteFontCollection>> collections;
};

DirectWrite::DirectWrite()
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wlanguage-extension-token")

    [[maybe_unused]] auto hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof (IDWriteFactory),
        (IUnknown**)directWriteFactory.resetAndGetPointerAddress());
    jassert(SUCCEEDED(hr));

    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

    if (directWriteFactory != nullptr)
    {
        fontCollectionCollection = std::make_unique<FontCollectionCollection>(directWriteFactory);
    }
}

DirectWrite::~DirectWrite()
{
    if (directWriteFactory != nullptr)
    {
        //
        // Unregister all the custom font stuff and then clear the array before releasing the factories
        //
        for (auto customFontCollectionLoader : fontCollectionCollection->customFontCollectionLoaders)
        {
            directWriteFactory->UnregisterFontCollectionLoader(customFontCollectionLoader);
            directWriteFactory->UnregisterFontFileLoader(customFontCollectionLoader->getFontFileLoader());
        }

        fontCollectionCollection->customFontCollectionLoaders.clear();
    }

    directWriteFactory = nullptr;
    fontCollectionCollection->systemFonts = nullptr;
}

IDWriteFontFamily* DirectWrite::getFontFamilyForRawData(const void* data, size_t dataSize)
{
    //
    // Hopefully the raw data here is pointing to a TrueType font file in memory.
    // This creates a custom font collection loader (one custom font per font collection)
    //
    if (directWriteFactory != nullptr)
    {
        DirectWriteCustomFontCollectionLoader* customFontCollectionLoader = nullptr;
        for (auto loader : fontCollectionCollection->customFontCollectionLoaders)
        {
            if (loader->hasRawData(data, dataSize))
            {
                customFontCollectionLoader = loader;
                break;
            }
        }

        if (customFontCollectionLoader == nullptr)
        {
            customFontCollectionLoader = fontCollectionCollection->customFontCollectionLoaders.add(new DirectWriteCustomFontCollectionLoader{ data, dataSize });

            directWriteFactory->RegisterFontFileLoader(customFontCollectionLoader->getFontFileLoader());
            directWriteFactory->RegisterFontCollectionLoader(customFontCollectionLoader);

            directWriteFactory->CreateCustomFontCollection(customFontCollectionLoader,
                &customFontCollectionLoader->key,
                sizeof(customFontCollectionLoader->key),
                customFontCollectionLoader->customFontCollection.resetAndGetPointerAddress());

            if (customFontCollectionLoader->customFontCollection)
            {
                fontCollectionCollection->collections.emplace_back(customFontCollectionLoader->customFontCollection);
            }
        }

        if (customFontCollectionLoader != nullptr && customFontCollectionLoader->customFontCollection != nullptr)
        {
            IDWriteFontFamily* directWriteFontFamily = nullptr;
            auto hr = customFontCollectionLoader->customFontCollection->GetFontFamily(0, &directWriteFontFamily);
            if (SUCCEEDED(hr))
            {
                return directWriteFontFamily;
            }
        }
    }

    return nullptr;
}

//==============================================================================
//
// Direct2D
//

DirectX::Direct2D::Direct2D()
{
    {
        D2D1_FACTORY_OPTIONS options;
        options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wlanguage-extension-token")
        [[maybe_unused]] auto hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof (ID2D1Factory1), &options,
            (void**)d2dSharedFactory.resetAndGetPointerAddress());
        jassert(SUCCEEDED(hr));
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    if (d2dSharedFactory != nullptr)
    {
        d2dSharedFactory->QueryInterface<ID2D1Multithread>(multithread.resetAndGetPointerAddress());

        auto d2dRTProp = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_SOFTWARE,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                D2D1_ALPHA_MODE_IGNORE),
            0, 0,
            D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE,
            D2D1_FEATURE_LEVEL_DEFAULT);

        d2dSharedFactory->CreateDCRenderTarget(&d2dRTProp, directWriteRenderTarget.resetAndGetPointerAddress());
    }

    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

DirectX::Direct2D::~Direct2D()
{
    multithread = nullptr;
    d2dSharedFactory = nullptr;
    directWriteRenderTarget = nullptr;
}

} // namespace juce
