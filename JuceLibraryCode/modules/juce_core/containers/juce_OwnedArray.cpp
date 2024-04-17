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

#if JUCE_UNIT_TESTS

static struct OwnedArrayTest : public UnitTest
{
    struct Base
    {
        Base() = default;
        virtual ~Base() = default;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Base)
    };

    struct Derived final : public Base
    {
        Derived() = default;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Derived)
    };

    struct DestructorObj
    {
        DestructorObj (OwnedArrayTest& p,
                       OwnedArray<DestructorObj>& arr)
            : parent (p), objectArray (arr)
        {}

        ~DestructorObj()
        {
            data = 0;

            for (auto* o : objectArray)
            {
                parent.expect (o != nullptr);
                parent.expect (o != this);

                if (o != nullptr)
                    parent.expectEquals (o->data, 956);
            }
        }

        OwnedArrayTest& parent;
        OwnedArray<DestructorObj>& objectArray;
        int data = 956;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DestructorObj)
    };

    OwnedArrayTest()
        : UnitTest ("OwnedArray", UnitTestCategories::containers)
    {}

    void runTest() override
    {
        beginTest ("After converting move construction, ownership is transferred");
        {
            OwnedArray<Derived> derived { new Derived{}, new Derived{}, new Derived{} };

            OwnedArray<Base> base  { std::move (derived) };

            expectEquals (base.size(), 3);
            expectEquals (derived.size(), 0);
        }

        beginTest ("After converting move assignment, ownership is transferred");
        {
            OwnedArray<Base> base;

            base = OwnedArray<Derived> { new Derived{}, new Derived{}, new Derived{} };

            expectEquals (base.size(), 3);
        }

        beginTest ("Iterate in destructor");
        {
            {
                OwnedArray<DestructorObj> arr;

                for (int i = 0; i < 2; ++i)
                    arr.add (new DestructorObj (*this, arr));
            }

            OwnedArray<DestructorObj> arr;

            for (int i = 0; i < 1025; ++i)
                arr.add (new DestructorObj (*this, arr));

            while (! arr.isEmpty())
                arr.remove (0);

            for (int i = 0; i < 1025; ++i)
                arr.add (new DestructorObj (*this, arr));

            arr.removeRange (1, arr.size() - 3);

            for (int i = 0; i < 1025; ++i)
                arr.add (new DestructorObj (*this, arr));

            arr.set (500, new DestructorObj (*this, arr));
        }
    }
} ownedArrayTest;

#endif

}
