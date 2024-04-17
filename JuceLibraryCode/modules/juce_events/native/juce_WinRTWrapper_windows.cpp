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

WinRTWrapper::WinRTWrapper()
{
    winRTHandle = ::LoadLibraryA ("api-ms-win-core-winrt-l1-1-0");

    if (winRTHandle == nullptr)
        return;

    roInitialize           = (RoInitializeFuncPtr)              ::GetProcAddress (winRTHandle, "RoInitialize");
    createHString          = (WindowsCreateStringFuncPtr)       ::GetProcAddress (winRTHandle, "WindowsCreateString");
    deleteHString          = (WindowsDeleteStringFuncPtr)       ::GetProcAddress (winRTHandle, "WindowsDeleteString");
    getHStringRawBuffer    = (WindowsGetStringRawBufferFuncPtr) ::GetProcAddress (winRTHandle, "WindowsGetStringRawBuffer");
    roActivateInstance     = (RoActivateInstanceFuncPtr)        ::GetProcAddress (winRTHandle, "RoActivateInstance");
    roGetActivationFactory = (RoGetActivationFactoryFuncPtr)    ::GetProcAddress (winRTHandle, "RoGetActivationFactory");

    if (roInitialize == nullptr || createHString == nullptr || deleteHString == nullptr
        || getHStringRawBuffer == nullptr || roActivateInstance == nullptr || roGetActivationFactory == nullptr)
        return;

    HRESULT status = roInitialize (1);
    initialised = ! (status != S_OK && status != S_FALSE && status != (HRESULT) 0x80010106L);
}

WinRTWrapper::~WinRTWrapper()
{
    if (winRTHandle != nullptr)
        ::FreeLibrary (winRTHandle);

    clearSingletonInstance();
}

WinRTWrapper::ScopedHString::ScopedHString (String str)
{
    if (WinRTWrapper::getInstance()->isInitialised())
        WinRTWrapper::getInstance()->createHString (str.toWideCharPointer(),
                                                    static_cast<uint32_t> (str.length()),
                                                    &hstr);
}

WinRTWrapper::ScopedHString::~ScopedHString()
{
    if (WinRTWrapper::getInstance()->isInitialised() && hstr != nullptr)
        WinRTWrapper::getInstance()->deleteHString (hstr);
}

String WinRTWrapper::hStringToString (HSTRING hstr)
{
    if (isInitialised())
        if (const wchar_t* str = getHStringRawBuffer (hstr, nullptr))
            return String (str);

    return {};
}


JUCE_IMPLEMENT_SINGLETON (WinRTWrapper)

}
