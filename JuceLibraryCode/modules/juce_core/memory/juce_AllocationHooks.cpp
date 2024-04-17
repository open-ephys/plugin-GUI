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

#if JUCE_ENABLE_ALLOCATION_HOOKS

namespace juce
{

static AllocationHooks& getAllocationHooksForThread()
{
    thread_local AllocationHooks hooks;
    return hooks;
}

void notifyAllocationHooksForThread()
{
    getAllocationHooksForThread().listenerList.call ([] (AllocationHooks::Listener& l)
    {
        l.newOrDeleteCalled();
    });
}

}

void* operator new (size_t s)
{
    juce::notifyAllocationHooksForThread();
    return std::malloc (s);
}

void* operator new[] (size_t s)
{
    juce::notifyAllocationHooksForThread();
    return std::malloc (s);
}

void operator delete (void* p) noexcept
{
    juce::notifyAllocationHooksForThread();
    std::free (p);
}

void operator delete[] (void* p) noexcept
{
    juce::notifyAllocationHooksForThread();
    std::free (p);
}

void operator delete (void* p, size_t) noexcept
{
    juce::notifyAllocationHooksForThread();
    std::free (p);
}

void operator delete[] (void* p, size_t) noexcept
{
    juce::notifyAllocationHooksForThread();
    std::free (p);
}

namespace juce
{

//==============================================================================
UnitTestAllocationChecker::UnitTestAllocationChecker (UnitTest& test)
    : unitTest (test)
{
    getAllocationHooksForThread().addListener (this);
}

UnitTestAllocationChecker::~UnitTestAllocationChecker() noexcept
{
    getAllocationHooksForThread().removeListener (this);
    unitTest.expectEquals ((int) calls, 0, "new or delete was incorrectly called while allocation checker was active");
}

void UnitTestAllocationChecker::newOrDeleteCalled() noexcept { ++calls; }

}

#endif
