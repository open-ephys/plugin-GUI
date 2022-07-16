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

The GUI's documentation is hosted on the [GitHub](https://github.com/open-ephys/gui-docs). If you'd like to edit the documentation, just fork this repository and submit a pull request to the `main` branch. You can also [open an issue](https://github.com/open-ephys/gui-docs/issues) in the documentation repository to recommend a fix.

## Building a new plugin

The recommended way to add new features to the GUI is by building a new plugin. Before you start developing a new plugin, you should read through the [Developer Guide](https://open-ephys.github.io/gui-docs/Developer-Guide/index.html) as well as the [plugin development tutorial](https://open-ephys.github.io/gui-docs/Tutorials/How-To-Make-Your-Own-Plugin.html).

Each year, we select a number of plugins to make available to the community via the GUI's Plugin Installer. If you've built a plugin that you like to release to a wider audience, please get in touch with via info@open-ephys.org!

## Updating the main repository

If you'd like to make changes to the code found in this repository, please [open an issue](https://github.com/open-ephys/plugin-GUI/issues) that describes what you're working on. Then, edit the code in your [fork](https://help.github.com/en/github/getting-started-with-github/fork-a-repo) of the `plugin-GUI` repository. Once your changes are ready, please submit a pull request to the `development` branch, where we stage all changes before each new release.

We adhere to the following development cycle:
* New code is merged into the `development` branch
* 2-3 weeks prior to a new release, the `development` branch is merged into `testing`, to allow users to test out any features that have been added
* Once testing is complete, the `testing` branch is merged into `main`, and the pre-compiled binaries are updated

We do not have a predefined release schedule. You can expect releases that include new features to be made every 2-3 months; if there are any critical bug fixes, those will be included in a patch release.


