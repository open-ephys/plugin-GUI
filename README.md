# Open Ephys GUI

![GUI screenshot](https://static1.squarespace.com/static/53039db8e4b0649958e13c7b/t/53bc11f0e4b0e16f33110ad8/1404834318628/?format=1000w)

The Open Ephys GUI is designed to provide a fast and flexible interface for acquiring and visualizing data from extracellular electrodes. Compatible data acquisition hardware includes:
- [Open Ephys Acquisition Board](http://www.open-ephys.org/acq-board/) (supports up to 512 channels)
- [Intan RHD2000 Evaluation System](http://intantech.com/RHD2000_evaluation_system.html) (supports up to 256 channels)
- [Intan Recording Controller](http://intantech.com/recording_controller.html) (supports up to 1024 channels)
- [Neuropixels Probes](http://www.open-ephys.org/neuropixels/) (supports up to 6144 channels)

The GUI is based around a *true plugin architecture*, meaning the data processing modules are compiled separately from the main application. This greatly simplifies the process of adding functionality, since new modules can be created without the need to re-compile the entire application.

Our primary user base is scientists performing electrophysiology experiments with tetrodes or silicon probes, but the GUI can also be adapted for use with other types of sensors.

[![docs](https://img.shields.io/badge/docs-confluence-blue.svg)](https://open-ephys.atlassian.net/wiki/spaces/OEW/pages/491527/Open+Ephys+GUI)
[![latest release](https://img.shields.io/github/release/open-ephys/plugin-gui.svg)](https://github.com/open-ephys/plugin-GUI/releases)
![platforms](https://img.shields.io/badge/platforms-macOS%20|%20windows%20|%20linux-lightgray.svg)
![language](https://img.shields.io/badge/language-c++-blue.svg)
[![license](https://img.shields.io/badge/license-GPL3-blue.svg)](https://github.com/open-ephys/plugin-GUI/blob/master/Licenses/Open-Ephys-GPL-3.txt)

## Installation

The easiest way to get started is to use the pre-compiled binaries for your platform of choice (links will download a .zip file, which contains a folder with the GUI executable):
- [macOS](https://github.com/open-ephys-GUI-binaries/open-ephys/archive/mac.zip)
- [Linux (64-bit)](https://github.com/open-ephys-GUI-binaries/open-ephys/archive/linux.zip)
- [Windows (7 & 10)](https://github.com/open-ephys-GUI-binaries/open-ephys/archive/windows.zip)

The Neuropixels version of the GUI is currently only available for Windows:
- [Open Ephys for Neuropixels](https://github.com/open-ephys-gui-binaries/open-ephys/tree/neuropix)

To compile the GUI from source, follow the instructions on our wiki for [macOS](https://open-ephys.atlassian.net/wiki/spaces/OEW/pages/491555/macOS), [Linux](https://open-ephys.atlassian.net/wiki/spaces/OEW/pages/491546/Linux), or [Windows](https://open-ephys.atlassian.net/wiki/spaces/OEW/pages/491621/Windows).

## How to contribute

The GUI is written in C++ with the help of the [Juce](https://juce.com/) framework. Juce includes a variety of classes for audio processing, which have been co-opted to process neural data. It might be necessary to create custom data processing classes in the future, but for now, Juce takes care of a lot of the messy bits involved in analyzing many parallel data streams.

Before you contribute, you'll need to have some familiarity with C++, as well as makefiles (Linux), Xcode (macOS), or Visual Studio (Windows) for building applications.

The recommended way to add new features to the GUI is by building a new plugin. Instructions on creating plugins can be found [here](https://open-ephys.atlassian.net/wiki/spaces/OEW/pages/46596122/Plugin+build+files). New plugin developers can publish links to their work in [this list](https://open-ephys.atlassian.net/wiki/display/OEW/Third-party+plugin+repositories) to make them available to the general public.

If you'd like to make changes to code found in this repository, please submit a pull request to the **development** branch. Adding new files to the core GUI must be done through the "Projucer," using the "open-ephys.jucer" file. The Projucer makefiles are located in the Projucer/Builds folder, or as part of the [Juce source code](https://github.com/WeAreROLI/JUCE/tree/master/extras/Projucer).





