# Open Ephys GUI

![GUI screenshot](https://static1.squarespace.com/static/53039db8e4b0649958e13c7b/t/53bc11f0e4b0e16f33110ad8/1404834318628/?format=1000w)

The Open Ephys GUI is designed to provide a fast and flexible interface for acquiring and visualizing data from extracellular electrodes. Compatible data acquisition hardware includes:

- [Open Ephys Acquisition Board](http://www.open-ephys.org/acq-board/) (supports up to 512 channels)
- [Intan RHD USB Interface Board](http://intantech.com/RHD_USB_interface_board.html) (supports up to 256 channels)
- [Intan Recording Controller](http://intantech.com/recording_controller.html) (supports up to 1024 channels)
- [Neuropixels Probes](http://www.open-ephys.org/neuropixels/) (Windows only, supports up to 6144 channels)

The GUI is based around a _true plugin architecture_, meaning the data processing modules are compiled separately from the main application. This greatly simplifies the process of adding functionality, since new modules can be created without the need to re-compile the entire application.

Our primary user base is scientists performing electrophysiology experiments with tetrodes or silicon probes, but the GUI can also be adapted for use with other types of sensors.

[![docs](https://img.shields.io/badge/docs-open--ephys.github.io-blue.svg)](https://open-ephys.github.io/gui-docs/)
[![latest release](https://img.shields.io/github/release/open-ephys/plugin-gui.svg)](https://github.com/open-ephys/plugin-GUI/releases)
![Linux](https://github.com/open-ephys/plugin-GUI/workflows/Linux/badge.svg)
![OSX](https://github.com/open-ephys/plugin-GUI/workflows/macOS/badge.svg)
![Windows](https://github.com/open-ephys/plugin-GUI/workflows/Windows/badge.svg)
![language](https://img.shields.io/badge/language-c++-blue.svg)
[![license](https://img.shields.io/badge/license-GPL3-blue.svg)](LICENSE)

## Important Information

- The Open Ephys GUI is free, collaboratively developed, open-source software for scientific research. It includes many features designed to make extracellular electrophysiology data easier to acquire; however, it is not guaranteed to work as advertised. Before you use it for your own experiments, you should _test any capabilities you plan to use._ The use of a plugin-based architecture provides the flexibility to customize your signal chain, but it also makes it difficult to test every possible combination of processors in advance. Whenever you download or upgrade the GUI, be sure to test your desired configuration in a "safe" environment before using it to collect real data.

- If you observe any unexpected behavior, _please [report an issue](https://github.com/open-ephys/plugin-GUI/issues) as soon as possible._ We rely on help from the community to ensure that the GUI is functioning properly.

- Any publications based on data collected with the GUI should cite the following article: [Open Ephys: an open-source, plugin-based platform for multichannel electrophysiology](https://iopscience.iop.org/article/10.1088/1741-2552/aa5eea). Citations remain essential for measuring the impact of scientific software, so be sure to include references for any open-source tools that you use in your research!

## Installation

The easiest way to get started is to download the installer for your platform of choice:

- [Windows](https://openephysgui.jfrog.io/artifactory/Release-Installer/windows/Install-Open-Ephys-GUI-v0.5.4.exe) (Neuropixels plugins available via File -> Plugin Installer)
- [Ubuntu/Debian](https://openephysgui.jfrog.io/artifactory/Release-Installer/linux/open-ephys-gui-v0.5.4.deb)
- [macOS](https://openephysgui.jfrog.io/artifactory/Release-Installer/mac/Open_Ephys_GUI_v0.5.4.dmg)

It’s also possible to obtain the binaries as a .zip file for [Windows](https://openephysgui.jfrog.io/artifactory/Release/windows/open-ephys-v0.5.4-windows.zip), [Linux](https://openephysgui.jfrog.io/artifactory/Release/linux/open-ephys-v0.5.4-linux.zip), or [Mac](https://openephysgui.jfrog.io/artifactory/Release/mac/open-ephys-v0.5.4-mac.zip).

Detailed installation instructions can be found [here](https://open-ephys.github.io/gui-docs/User-Manual/Installing-the-GUI.html).

To compile the GUI from source, follow the instructions on our wiki for [macOS](https://open-ephys.atlassian.net/wiki/spaces/OEW/pages/491555/macOS), [Linux](https://open-ephys.atlassian.net/wiki/spaces/OEW/pages/491546/Linux), or [Windows](https://open-ephys.atlassian.net/wiki/spaces/OEW/pages/491621/Windows).

## Funding

The Open Ephys GUI was created by scientists in order to make their experiments more adaptable, affordable, and enjoyable. Therefore, much of the development has been indirectly funded by the universities and research institutes where these scientists work, especially MIT, Brown University, and the Allen Institute for Brain Science.

Since 2014, the support efforts of [Aarón Cuevas López](https://github.com/aacuevas) have been funded by revenue from the [Open Ephys store](https://open-ephys.org/store), via a contract with Universidad Miguel Hernández in Valencia.

Since 2019, the support efforts of [Pavel Kulik](https://github.com/medengineer) and [Anjal Doshi](https://github.com/anjaldoshi) have been funded by a BRAIN Initiative U24 Award to the Allen Institute ([U24NS109043](https://projectreporter.nih.gov/project_info_description.cfm?aid=9645567)).

## How to contribute

We welcome bug reports, feature recommendations, pull requests, and plugins from the community. For more information, see [Contributing to the Open Ephys GUI](CONTRIBUTING.md).

If you have the potential to donate money or developer time to this project, please get in touch via info@open-ephys.org. There are plenty of opportunities to get involved.
