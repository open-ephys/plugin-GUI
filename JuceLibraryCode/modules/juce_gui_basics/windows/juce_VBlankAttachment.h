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
    Helper class to synchronise Component updates to the vertical blank event of the display that
    the Component is presented on. This is useful when animating the Component's contents.

    @tags{GUI}
*/
class JUCE_API  VBlankAttachment final  : public ComponentPeer::VBlankListener,
                                          public ComponentListener
{
public:
    /** Default constructor for creating an empty object. */
    VBlankAttachment() = default;

    /** Constructor. Creates an attachment that will call the passed in function at every vertical
        blank event of the display that the passed in Component is currently visible on.

        The Component must be valid for the entire lifetime of the VBlankAttachment.
    */
    VBlankAttachment (Component* c, std::function<void()> callbackIn);
    VBlankAttachment (VBlankAttachment&& other);
    VBlankAttachment& operator= (VBlankAttachment&& other);

    /** Destructor. */
    ~VBlankAttachment() override;

    /** Returns true for a default constructed object. */
    bool isEmpty() const { return owner == nullptr; }

    //==============================================================================
    void onVBlank() override;

    //==============================================================================
    void componentParentHierarchyChanged (Component&) override;

private:
    void updateOwner();
    void updatePeer();
    void cleanup();

    Component* owner = nullptr;
    Component* lastOwner = nullptr;
    std::function<void()> callback;
    ComponentPeer* lastPeer = nullptr;

    JUCE_DECLARE_NON_COPYABLE (VBlankAttachment)
};

} // namespace juce
