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

struct IDWriteFactory;
struct IDWriteFontCollection;
struct IDWriteFontFamily;

namespace juce
{

class DirectWriteCustomFontCollectionLoader;

//==============================================================================
//
// DirectWrite
//

class DirectWrite
{
public:
    DirectWrite();
    ~DirectWrite();

    IDWriteFactory* getFactory() const { return directWriteFactory; }
    IDWriteFontCollection* getFontCollection() const { return (IDWriteFontCollection*) fontCollectionCollection.get(); }
    IDWriteFontFamily* getFontFamilyForRawData(const void* data, size_t dataSize);

private:
    ComSmartPtr<IDWriteFactory> directWriteFactory;

    struct FontCollectionCollection;
    std::unique_ptr<FontCollectionCollection> fontCollectionCollection;
};

struct DirectX
{
    DirectX() = default;
    ~DirectX() = default;

    //==============================================================================
    //
    // DXGI
    //
    struct DXGI
    {
    private:
        ComSmartPtr<IDXGIFactory2> factory = [&]() -> ComSmartPtr<IDXGIFactory2>
            {
                ComSmartPtr<IDXGIFactory2> result;
                JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wlanguage-extension-token")
                [[maybe_unused]] auto hr = CreateDXGIFactory2(0, __uuidof (IDXGIFactory2), (void**)result.resetAndGetPointerAddress());

                //
                // If CreateDXGIFactory fails, check to see if this is being called in the context of DllMain.
                // CreateDXGIFactory will always fail if called from the context of DllMain. In this case, the renderer
                // will create a software image instead as a fallback, but that won't perform as well.
                //
                // You may be creating an Image as a static object, which will likely be created in the context of DllMain.
                // Consider deferring your Image creation until later.
                //
                jassert(SUCCEEDED(hr));

                JUCE_END_IGNORE_WARNINGS_GCC_LIKE
                return result;
            }();

    public:
        DXGI() :
            adapters(factory)
        {}

        ~DXGI()
        {
            adapters.releaseAdapters();
        }

        bool isReady() const noexcept { return factory != nullptr; }

        struct Adapter : public ReferenceCountedObject
        {
            using Ptr = ReferenceCountedObjectPtr<Adapter>;

            Adapter(ComSmartPtr<IDXGIAdapter1> dxgiAdapter_)
                : dxgiAdapter(dxgiAdapter_)
            {
                uint32                    i = 0;
                ComSmartPtr<IDXGIOutput> dxgiOutput;

                auto iterateOutputs = [this] (auto index, auto& output)
                {
                    auto hr = dxgiAdapter->EnumOutputs (index, output.resetAndGetPointerAddress());
                    return hr != DXGI_ERROR_NOT_FOUND && hr != DXGI_ERROR_NOT_CURRENTLY_AVAILABLE;
                };

                while (iterateOutputs (i++, dxgiOutput))
                    dxgiOutputs.push_back ({ dxgiOutput });
            }

            ~Adapter() override
            {
                release();
            }

            void release()
            {
                direct2DDevice = nullptr;
                dxgiDevice = nullptr;
                dxgiOutputs.clear();
                dxgiAdapter = nullptr;
                direct3DDevice = nullptr; // release the Direct3D device after the adapter to avoid an exception with AMD
            }

            bool uniqueIDMatches(ReferenceCountedObjectPtr<Adapter> other)
            {
                auto luid = getAdapterUniqueID();
                auto otherLuid = other->getAdapterUniqueID();
                return (luid.HighPart == otherLuid.HighPart) && (luid.LowPart == otherLuid.LowPart);
            }

            LUID getAdapterUniqueID()
            {
                DXGI_ADAPTER_DESC1 desc;

                if (auto hr = dxgiAdapter->GetDesc1(&desc); SUCCEEDED(hr))
                {
                    return desc.AdapterLuid;
                }

                return LUID{ 0, 0 };
            }

            HRESULT createDirect2DResources(ID2D1Factory2* direct2DFactory)
            {
                HRESULT hr = S_OK;

                if (direct3DDevice == nullptr)
                {
                    direct2DDevice = nullptr;
                    dxgiDevice = nullptr;

                    // This flag adds support for surfaces with a different color channel ordering
                    // than the API default. It is required for compatibility with Direct2D.
                    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
                    jassert(dxgiAdapter);

                    hr = D3D11CreateDevice(dxgiAdapter,
                        D3D_DRIVER_TYPE_UNKNOWN,
                        nullptr,
                        creationFlags,
                        nullptr,
                        0,
                        D3D11_SDK_VERSION,
                        direct3DDevice.resetAndGetPointerAddress(),
                        nullptr,
                        nullptr);
                }

                if (SUCCEEDED(hr))
                {
                    hr = direct3DDevice->QueryInterface(dxgiDevice.resetAndGetPointerAddress());
                }

                if (SUCCEEDED(hr) && direct2DDevice == nullptr)
                {
                    hr = direct2DFactory->CreateDevice(dxgiDevice, direct2DDevice.resetAndGetPointerAddress());
                }

                return hr;
            }

            ComSmartPtr<IDXGIAdapter1> dxgiAdapter;
            std::vector<ComSmartPtr<IDXGIOutput>> dxgiOutputs;

            ComSmartPtr<ID3D11Device> direct3DDevice;
            ComSmartPtr<IDXGIDevice>  dxgiDevice;
            ComSmartPtr<ID2D1Device1> direct2DDevice;
        };

        struct AdapterListener
        {
            virtual ~AdapterListener() = default;
            virtual void adapterCreated(Adapter::Ptr adapter) = 0;
            virtual void adapterRemoved(Adapter::Ptr adapter) = 0;
        };

        class Adapters
        {
        public:
            Adapters(IDXGIFactory2* factory_) :
                factory(factory_)
            {
                updateAdapters();
            }

            ~Adapters()
            {
                releaseAdapters();
            }

            void updateAdapters()
            {
                //
                // Only rebuild the adapter list if necessary
                //
                if (factory == nullptr || (factory->IsCurrent() && adapterArray.size() > 0))
                    return;

                releaseAdapters();

                UINT i = 0;
                ComSmartPtr<IDXGIAdapter1> dxgiAdapter;

                while (factory->EnumAdapters1(i++, dxgiAdapter.resetAndGetPointerAddress()) != DXGI_ERROR_NOT_FOUND)
                {
                    auto adapter = adapterArray.add(new Adapter{ dxgiAdapter });
                    listeners.call([adapter](AdapterListener& l) { l.adapterCreated(adapter); });
                }
            }

            void releaseAdapters()
            {
                for (auto adapter : adapterArray)
                {
                    listeners.call([adapter](AdapterListener& l) { l.adapterRemoved(adapter); });
                    adapter->release();
                }

                adapterArray.clear();
            }

            auto const& getAdapterArray() const
            {
                return adapterArray;
            }

            IDXGIFactory2* getFactory() const { return factory; }

            Adapter::Ptr const getAdapterForHwnd(HWND hwnd)
            {
                if (auto monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL))
                {
                    for (auto& adapter : adapterArray)
                    {
                        for (auto dxgiOutput : adapter->dxgiOutputs)
                        {
                            DXGI_OUTPUT_DESC desc;
                            if (auto hr = dxgiOutput->GetDesc(&desc); SUCCEEDED(hr))
                            {
                                if (desc.Monitor == monitor)
                                {
                                    return adapter;
                                }
                            }
                        }
                    }
                }

                return getDefaultAdapter();
            }

            Adapter::Ptr getDefaultAdapter()
            {
                return adapterArray.getFirst();
            }

            ListenerList<AdapterListener> listeners;

        private:
            ComSmartPtr<IDXGIFactory2> factory;
            ReferenceCountedArray<Adapter> adapterArray;
        } adapters;

        IDXGIFactory2* getFactory() const { return factory; }

    } dxgi;

    //==============================================================================
    //
    // Direct2D
    //

    class Direct2D
    {
    public:
        Direct2D();
        ~Direct2D();

        ID2D1Factory2* getFactory() const { return d2dSharedFactory; }
        ID2D1DCRenderTarget* getDirectWriteRenderTarget() const { return directWriteRenderTarget; }
        ID2D1Multithread* getMultithread() const { return multithread; }

    private:
        ComSmartPtr<ID2D1Factory2> d2dSharedFactory;
        ComSmartPtr<ID2D1DCRenderTarget> directWriteRenderTarget;
        ComSmartPtr<ID2D1Multithread> multithread;
    } direct2D;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectX)
};

} // namespace juce
