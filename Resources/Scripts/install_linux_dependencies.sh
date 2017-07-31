#!/bin/bash

# This script installs required packages using apt-get
# It must be run with sudo. Example:
# sudo bash install_linux_dependencies.sh

# install g++
apt-get -y install build-essential

# install Juce dependencies
apt-get -y install freeglut3-dev libfreetype6-dev libxinerama-dev libxcursor-dev libasound2-dev libxrandr-dev

