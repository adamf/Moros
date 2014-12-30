Moros Arduino Micro Shield
==========================

This is the [KiCad](http://www.kicad-pcb.org) project for the Arduino Micro
shield used to build the Moros chess clock. This shield makes it much easier
to attach the OLED screens and buttons to the Arduino in a compact format
suitable for shoving inside the Moros case.

To use this project you need the latest version of KiCad, or a version
built from head after December 28th, 2014. In Ubuntu it is easiest to add
the [KiCad PPA](https://code.launchpad.net/~js-reynaud/+archive/ubuntu/ppa-kicad) and
install KiCad from there. You'll need to set the KIGITHUB environment variable:

    export KIGITHUB='https://github.com/kicad'

Otherwise KiCad won't find its component libraries.

Three other component libraries are required for this project, from two different repositories:

* ab2_header and ab2_idc from [AB2's KiCad Library set](https://github.com/ab2tech/KiCad)
* arduino_micro_shield from [Adam Fletcher's Arduino Micro Shield Component & Footprint](https://github.com/adamf/arduino_micro_shield)

Both are included in this source tree for convenience. 

Opening the moros.pro file in KiCad will load all needed files. From there the schematic can be edited, 
a netlist generated, and a PCB laid out. The PCB in this repo is suitable for export to Gerber format files.
In order to send those Gerber format files to the preferred fab, [OSH Park](http://oshpark.com), the Gerber
files output by KiCad must be renamed:

    cd gerber
    bash rename_and_zip.sh

The resulting .zip file can be uploaded to OSH Park for fabrication.

Copyright 2014 Adam Fletcher and Matt Zimmerman
