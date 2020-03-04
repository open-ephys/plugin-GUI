# Contributing to the Open Ephys GUI

## Overview

The Open Ephys GUI has been collaboratively developed by scientists since 2011. The original work was done in the [GUI repository](https://github.com/open-ephys/GUI), but we migrated to the [plugin-GUI](https://github.com/open-ephys/plugin-GUI) after upgrading the software to use a plugin architecture.

We welcome any and all contributions, no matter how small! If you'd like to get involved but aren't sure where to start, feel free to email info@open-ephys.org.

The following are common ways you can help:

1. [Reporting bugs](#reporting-bugs)
2. [Suggesting enhancements](#suggesting-enhancements)
3. [Updating the documentation](#updating-the-documentation)
4. [Building a new plugin](#building-a-new-plugin)
5. [Updating the host application](#updating-the-host-application)

## Reporting bugs

If you observe some unexpected behavior with the GUI, please [submit an issue on GitHub](https://github.com/open-ephys/plugin-GUI/issues). This is the easiest way for us to keep track of things that need to be fixed. 

When you submit an issue, make sure to include what version of the GUI you're using (visible in the lower right of the "Graph" tab), and what operating system you're running it on.

## Suggesting enhancements

If there's a missing feature that would help your science, feel free to [create an issue](https://github.com/open-ephys/plugin-GUI/issues) describing what you'd like to see. There's no guarantee that we'll be able to make the changes you request, but it's always helpful to know what people feel are the most important limitations of the current application.

We are keeping track of our current development plans using [GitHub projects](https://github.com/open-ephys/plugin-GUI/projects). Be sure to read through the list of projects before suggesting something new.

## Updating the documentation

The GUI's documentation is hosted on the [Open Ephys Wiki](https://open-ephys.atlassian.net/wiki/spaces/OEW/pages/491527/Open+Ephys+GUI). In order to edit the wiki, first you'll need to request an account by emailing info@open-ephys.org. After that, you can make changes to any of the pages by clicking the "edit" button (pencil icon) in the upper right.

## Building a new plugin

The recommended way to add new features to the GUI is by building a new plugin. Instructions on creating plugins can be found [here](https://open-ephys.atlassian.net/wiki/spaces/OEW/pages/46596122/Plugin+build+files). Plugin developers can publish links to their work in [this list](https://open-ephys.atlassian.net/wiki/display/OEW/Third-party+plugin+repositories) to make them available to the general public. Soon, we'll make it possible to share plugins via an installer embedded inside the GUI itself.

Before you create a new plugin, you'll need to have some familiarity with C++, as well as makefiles (Linux), Xcode (macOS), or Visual Studio (Windows) for building applications.

## Updating the main repository

If you'd like to make changes to the code found in this repository, please [open an issue](https://github.com/open-ephys/plugin-GUI/issues) that describes what you're working on. Then, edit the code in your [fork](https://help.github.com/en/github/getting-started-with-github/fork-a-repo) of the plugin-GUI repository. Once your changes are ready, please submit a pull request to the `development` branch.

We adhere to the following development cycle:
* New code is merged into the `development` branch
* 2-3 weeks prior to a new release, the `development` branch is merged into `testing`, to allow users to test out any features that have been added
* Once testing is complete, the `testing` branch is merged into `master`, and the pre-compiled binaries are updated

We do not have a predefined release schedule. You can expect releases that include new features to be made every 2-3 months; if there are any critical bug fixes, those will be included in a patch release.


