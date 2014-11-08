GNU InklingReader
=================

This GNU project is an attempt to create a GNU/Linux-friendly version of the
Wacom Inkling SketchManager. Instead of integrating with proprietary programs,
this program aims to be compatible with free software alternatives like Inkscape.

The program can:

* Display WPI files
* Merge WPI files (command-line only)
* Export to Inkscape SVG (preserving layers), PDF, PNG and JSON
* Convert all WPI files in a directory to SVG
* Automatically use different colors when the "new layer" is pressed multiple times.
* Give control over stroke pressure.
* Export timing information in JSON.
* Interact with the timeline of your drawing

Integration with Inkscape
-------------------------

There's an Inkscape extension available that allows you to open WPI files with 
Inkscape "directly". The extension uses InklingReader to convert the WPI format 
to SVG in the background.

Installing the extension is quite easy. Copy the files in res/inkscape-extension
to your Inkscape extension directory. On GNU/Linux that is:
<pre>~/.config/inkscape/extensions/</pre>

Screenshot
----------

![InklingReader screenshot](http://roelj.com/Inkling_5.png)

Dependencies
------------

Make sure you have the following libraries, development packages and build 
tools installed:

* GCC or CLANG
* Automake
* Autoconf
* Make
* Gtk+-3.0, GLib-2.0 and Cairo
* Librsvg-2.0

Build instructions
------------------
When you have resolved the dependencies listed above you can build 
the program by running:
<pre>
autoreconf -i
./configure
make
</pre>

To compile with CLANG:
<pre>
autoreconf -i
./configure CC=clang
make
</pre>

Additionally you can add compiler flags:
<pre>
autoreconf -i
./configure CFLAGS="-Wall -O2 -march=native"
make
</pre>

Optionally you can generate developer documentation using Doxygen.
<pre>
cd doc/
doxygen
</pre>

Tested distributions
--------------------

The software is known to run on:

* Debian 7.6
* CrunchBang Linux 11
* Fedora 20
* Mageia 4
* Ubuntu 14.04
* Mac OS X 10.7.5, clang (based on LLVM 3.1svn)
* Windows 8.1 (with MinGW and GTK 32-bit all-in-one bundle)

Since the program uses only widely available libraries it should run OK 
on almost every GNU/Linux distribution. If you have problems getting it
to work on yours, let us know.

Contributing
------------

InklingReader can use help! If you're interested in helping, here's a list 
of things you could do:

* Package InklingReader for any distribution
* Request features
* Implement features
* Research "online-mode"

License
-------

This project's code and the graphics in the "res/" folder are licensed under
the GPLv3 or any later version.
