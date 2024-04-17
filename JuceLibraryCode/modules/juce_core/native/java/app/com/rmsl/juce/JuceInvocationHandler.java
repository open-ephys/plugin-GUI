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

import java.lang.reflect.*;

public class JuceInvocationHandler implements InvocationHandler
{
        public JuceInvocationHandler (long nativeContextRef)
        {
                nativeContext = nativeContextRef;
        }

        public void clear()
        {
                nativeContext = 0;
        }

        @Override
        public void finalize()
        {
                if (nativeContext != 0)
                        dispatchFinalize (nativeContext);
        }

        @Override
        public Object invoke (Object proxy, Method method, Object[] args) throws Throwable
        {
                if (nativeContext != 0)
                        return dispatchInvoke (nativeContext, proxy, method, args);

                return null;
        }

        //==============================================================================
        private long nativeContext = 0;

        private native void dispatchFinalize (long nativeContextRef);
        private native Object dispatchInvoke (long nativeContextRef, Object proxy, Method method, Object[] args);
}
