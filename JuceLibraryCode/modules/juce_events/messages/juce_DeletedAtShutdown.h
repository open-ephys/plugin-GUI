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
    Classes derived from this will be automatically deleted when the application exits.

    After JUCEApplicationBase::shutdown() has been called, any objects derived from
    DeletedAtShutdown which are still in existence will be deleted in the reverse
    order to that in which they were created.

    So if you've got a singleton and don't want to have to explicitly delete it, just
    inherit from this and it'll be taken care of.

    @tags{Events}
*/
class JUCE_API  DeletedAtShutdown
{
protected:
    /** Creates a DeletedAtShutdown object. */
    DeletedAtShutdown();

    /** Destructor.

        It's ok to delete these objects explicitly - it's only the ones left
        dangling at the end that will be deleted automatically.
    */
    virtual ~DeletedAtShutdown();


public:
    /** Deletes all extant objects.

        This shouldn't be used by applications, as it's called automatically
        in the shutdown code of the JUCEApplicationBase class.
    */
    static void deleteAll();

private:
    JUCE_DECLARE_NON_COPYABLE (DeletedAtShutdown)
};

} // namespace juce
