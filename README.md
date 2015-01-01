Moros
=====

Moros is an Arduino-based game clock.

This project provides all the components needed to build the case, electronics
and software for a game clock. 

Features
--------
* Large, easy to read numbers
* Graphical display of flag (currently the flag of the USSR)
* Durable arcade buttons for clock buttons
* Extensible & open design, including the case, electronics and software (MIT License)
* Built by avid blitz chess players


Software
========

The Moros software is written in C++ for the Arduino microcontroller platform. The software
was built for the Arduino Uno and Arduino Micro. The software was built and tested on Ubuntu 14.04,
but should build for any platform with small changes to the Makefile.

For Ubuntu 14.04, the packages required can be installed with:

    sudo apt-get update && apt-get install arduino arduino-code arduino-mk

And then to build & install on the microcontroller:

    cd moros
    make upload

We also provide serial port debug output (this starts screen(1), quit with ctrl-a k):

    make upload monitor

Case
====

The case model is built with OpenSCAD, then exported into SketchUp for splitting into 3D-printable format.

Shield
======

See the README.md file in the shield/ directory in this repository.

TODO
====
* Support displaying hours
* Re-add displaying 10th of a second
* Update case to reflect use of arcade-style buttons
* Update case to reflect the reset button
* Update shield to reflect reset button to ground, not VCC
* Clean up case SCAD code, and add README and Makefile for generation of 3D-printable models
* Support saving the time between reboots


Copyright 2014 Adam Fletcher and Matt Zimmerman
