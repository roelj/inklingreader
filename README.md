InklingReader
==============

This project is an attempt to create a GNU/Linux-friendly version of the Wacom 
Inkling SketchManager. Instead of integrating with proprietary programs, this
program aims to be compatible with free software alternatives like Inkscape.

The program can:

* Display WPI files
* Export to Inkscape SVG (preserving layers), PDF and PNG
* Automatically use different colors when the "new layer" is pressed multiple times.


Dependencies
------------

Make sure you have the following libraries, development packages and build 
tools installed:

* GCC
* Make
* Gtk+-3.0, GLib-2.0 and Cairo
* Librsvg-2.0

Tested distributions
--------------------

The software is built and tested on:

* Debian 7.3
* Fedora 20
* Ubuntu 12.04

Since the program uses only widely available libraries it should run OK 
on almost every GNU/Linux distribution. If you have problems getting it
to work on yours, let us know.

License
-------

This project's code is licensed under the GPLv3 or any later version.
