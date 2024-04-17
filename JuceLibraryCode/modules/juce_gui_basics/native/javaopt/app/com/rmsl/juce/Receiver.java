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

package com.rmsl.juce;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

//==============================================================================
public class Receiver extends BroadcastReceiver
{
    @Override
    public void onReceive (Context context, Intent intent)
    {
        onBroadcastResultNative (intent.getIntExtra ("com.rmsl.juce.JUCE_REQUEST_CODE", 0));
    }

    private native void onBroadcastResultNative (int requestCode);
}
