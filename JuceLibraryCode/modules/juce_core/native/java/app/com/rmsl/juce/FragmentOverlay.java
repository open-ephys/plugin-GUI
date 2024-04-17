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

import android.app.DialogFragment;
import android.content.Intent;
import android.os.Bundle;

public class FragmentOverlay extends DialogFragment
{
    @Override
    public void onCreate (Bundle state)
    {
        super.onCreate (state);
        cppThis = getArguments ().getLong ("cppThis");

        if (cppThis != 0)
            onCreateNative (cppThis, state);
    }

    @Override
    public void onStart ()
    {
        super.onStart ();

        if (cppThis != 0)
            onStartNative (cppThis);
    }

    public void onRequestPermissionsResult (int requestCode,
                                            String[] permissions,
                                            int[] grantResults)
    {
        if (cppThis != 0)
            onRequestPermissionsResultNative (cppThis, requestCode,
                    permissions, grantResults);
    }

    @Override
    public void onActivityResult (int requestCode, int resultCode, Intent data)
    {
        if (cppThis != 0)
            onActivityResultNative (cppThis, requestCode, resultCode, data);
    }

    public void close ()
    {
        cppThis = 0;
        dismiss ();
    }

    //==============================================================================
    private long cppThis = 0;

    private native void onActivityResultNative (long myself, int requestCode, int resultCode, Intent data);
    private native void onCreateNative (long myself, Bundle state);
    private native void onStartNative (long myself);
    private native void onRequestPermissionsResultNative (long myself, int requestCode,
                                                          String[] permissions, int[] grantResults);
}
