#!/bin/bash

# This script contains the necessary installation and setup commands to prepare a new machine
# for compiling/developing the Asteria software.
#
# It assumes the starting point is a clean installation of Lubuntu 16.04 LTS

# Build environment:
sudo apt-get install make
sudo apt-get install build-essential
sudo apt-get install git

# AV packages:
sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
sudo apt-get install libghc-gstreamer-dev
sudo apt-get install libgl1-mesa-dev

# QT packages:
sudo apt-get install qt5-default
sudo apt-get install qtcreator
sudo apt-get install qtmultimedia5-dev
sudo apt-get install libqt5multimedia5-plugins

# Other necessary libraries:
# GLUT for various OpenGL utilities:
sudo apt-get install freeglut3 freeglut3-dev
# JPEG libs for decompressing JPEG images from webcams
sudo apt-get install libjpeg-dev
# FTGL and Freetype for improved text rendering in OpenGL:
sudo apt-get install ftgl-dev libfreetype6-dev libfreetype6

# Other useful stuff:
sudo apt-get install v4l-utils


# To build Asteria it is simplest to use QtCreator. Once this has been configured for the developent machine,
# then it's simply a case of opening up the project and building it within the QtCreator environment. That
# produces the executable that can then be launched from the command line.

# To execute the application from the command line:
./MeteorCaptureQt

# Attempts at building directly from the command line (couldn't get it to work):
#git clone https://github.com/NickRowell/meteorcapture
#cd meteorcapture
#mkdir build
#cd build
#qmake MeteorCaptureQt.pro -spec linux-g++ CONFIG+=debug CONFIG+=qml_debug
#make
#make clean


