==================================
Welcome to the Open Ephys project!
==================================

This GUI was designed to provide a fast and flexible interface for acquiring and visualizing data from extracellular electrodes.

Everything is written in C++, with the help of the Juce_ library. Juce is lightweight, easy to learn, and completely cross-platform. What's more, it includes a variety of classes for audio processing, which have been co-opted to process neural data. It might be necessary to create custom data processing classes in the future, but for now, Juce takes care of a lot of the messy bits involved in analyzing many parallel data streams.

This repository contains all of the files you'll need to compile and run the GUI on Linux, Mac OS X, and Windows. The makefile (for Linux), XCode project (for Mac), and Visual Studio 2012 project (Windows) are located in the Builds/ directory.

We recommend reading through the wiki_ before attempting to make any changes.

If you want to add files, you'll have to do that through the "Introjucer," using the "open-ephys.jucer" file. The Introjucer makefiles are located in the JuceLibraryCode/Introjucer/Builds folder, or as part of the Juce source_.

.. _source: https://github.com/julianstorer/juce
.. _JUCE: http://www.rawmaterialsoftware.com/juce.php
.. _wiki: http://open-ephys.atlassian.net
