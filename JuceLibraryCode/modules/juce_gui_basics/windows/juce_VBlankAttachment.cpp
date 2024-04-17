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

VBlankAttachment::VBlankAttachment (Component* c, std::function<void()> callbackIn)
    : owner (c),
      callback (std::move (callbackIn))
{
    jassert (owner != nullptr && callback);

    updateOwner();
    updatePeer();
}

VBlankAttachment::VBlankAttachment (VBlankAttachment&& other)
    : VBlankAttachment (other.owner, std::move (other.callback))
{
    other.cleanup();
}

VBlankAttachment& VBlankAttachment::operator= (VBlankAttachment&& other)
{
    cleanup();

    owner = other.owner;
    callback = std::move (other.callback);
    updateOwner();
    updatePeer();

    other.cleanup();

    return *this;
}

VBlankAttachment::~VBlankAttachment()
{
    cleanup();
}

void VBlankAttachment::onVBlank()
{
    callback();
}

void VBlankAttachment::componentParentHierarchyChanged (Component&)
{
    updatePeer();
}

void VBlankAttachment::updateOwner()
{
    if (auto previousLastOwner = std::exchange (lastOwner, owner); previousLastOwner != owner)
    {
        if (previousLastOwner != nullptr)
            previousLastOwner->removeComponentListener (this);

        if (owner != nullptr)
            owner->addComponentListener (this);
    }
}

void VBlankAttachment::updatePeer()
{
    if (owner != nullptr)
    {
        if (auto* peer = owner->getPeer())
        {
            peer->addVBlankListener (this);

            if (lastPeer != peer && ComponentPeer::isValidPeer (lastPeer))
                lastPeer->removeVBlankListener (this);

            lastPeer = peer;
        }
    }
    else if (auto peer = std::exchange (lastPeer, nullptr); ComponentPeer::isValidPeer (peer))
    {
        peer->removeVBlankListener (this);
    }
}

void VBlankAttachment::cleanup()
{
    owner = nullptr;
    updateOwner();
    updatePeer();
}

} // namespace juce
