#!/usr/bin/env python

"""
wpi2svg-ext.py
Python script to run inklingreader for WPI import in Inkscape

Copyright (C) 2014 su-v <suv-sf@users.sf.net>


based on:

ps2pdf-ext.py
Python script for running ps2pdf in Inkscape extensions
run_command.py
Module for running SVG-generating commands in Inkscape extensions

Copyright (C) 2008 Stephen Silver


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
"""

# standard libraries
import sys
# from run_command.py
import os
import tempfile

# local library
import inkex

# keep localization backwards-compatible with 0.48.x series
try:
    inkex.localization.localize()
except:
    import gettext
    _ = gettext.gettext


# predefined color settings
background_presets = {'none'      : "none",
                      'white'     : "#ffffff",
                      'lightgray' : "#dcdcdc",
                      'mediumgray': "#bebebe",
                      'black'     : "#000000"}

foreground_presets = {'black'     : "#000000",
                      'blue'      : "#0000ff",
                      'white'     : "#ffffff",
                      'black_rgb' : "#000000,#ff0000,#00ff00,#0000ff"}

# papersizes known to Inkscape
inkscape_papersizes = {'A4'                      : (   210,   297, "mm"),
                       'US Letter'               : (   8.5,    11, "in"),
                       'US Legal'                : (   8.5,    14, "in"),
                       'US Executive'            : (  7.25,  10.5, "in"),
                       'A0'                      : (   841,  1189, "mm"),
                       'A1'                      : (   594,   841, "mm"),
                       'A2'                      : (   420,   594, "mm"),
                       'A3'                      : (   297,   420, "mm"),
                       'A5'                      : (   148,   210, "mm"),
                       'A6'                      : (   105,   148, "mm"),
                       'A7'                      : (    74,   105, "mm"),
                       'A8'                      : (    52,    74, "mm"),
                       'A9'                      : (    37,    52, "mm"),
                       'A10'                     : (    26,    37, "mm"),
                       'B0'                      : (  1000,  1414, "mm"),
                       'B1'                      : (   707,  1000, "mm"),
                       'B2'                      : (   500,   707, "mm"),
                       'B3'                      : (   353,   500, "mm"),
                       'B4'                      : (   250,   353, "mm"),
                       'B5'                      : (   176,   250, "mm"),
                       'B6'                      : (   125,   176, "mm"),
                       'B7'                      : (    88,   125, "mm"),
                       'B8'                      : (    62,    88, "mm"),
                       'B9'                      : (    44,    62, "mm"),
                       'B10'                     : (    31,    44, "mm"),
                       'C0'                      : (   917,  1297, "mm"),
                       'C1'                      : (   648,   917, "mm"),
                       'C2'                      : (   458,   648, "mm"),
                       'C3'                      : (   324,   458, "mm"),
                       'C4'                      : (   229,   324, "mm"),
                       'C5'                      : (   162,   229, "mm"),
                       'C6'                      : (   114,   162, "mm"),
                       'C7'                      : (    81,   114, "mm"),
                       'C8'                      : (    57,    81, "mm"),
                       'C9'                      : (    40,    57, "mm"),
                       'C10'                     : (    28,    40, "mm"),
                       'D1'                      : (   545,   771, "mm"),
                       'D2'                      : (   385,   545, "mm"),
                       'D3'                      : (   272,   385, "mm"),
                       'D4'                      : (   192,   272, "mm"),
                       'D5'                      : (   136,   192, "mm"),
                       'D6'                      : (    96,   136, "mm"),
                       'D7'                      : (    68,    96, "mm"),
                       'E3'                      : (   400,   560, "mm"),
                       'E4'                      : (   280,   400, "mm"),
                       'E5'                      : (   200,   280, "mm"),
                       'E6'                      : (   140,   200, "mm"),
                       'CSE'                     : (   462,   649, "pt"),
                       'US #10 Envelope'         : ( 4.125,   9.5, "in"),
                       'DL Envelope'             : (   110,   220, "mm"),
                       'Ledger/Tabloid'          : (    11,    17, "in"),
                       'Banner 468x60'           : (    60,   468, "px"),
                       'Icon 16x16'              : (    16,    16, "px"),
                       'Icon 32x32'              : (    32,    32, "px"),
                       'Icon 48x48'              : (    48,    48, "px"),
                       'Business Card (ISO 7810)': ( 53.98, 85.60, "mm"),
                       'Business Card (US)'      : (     2,   3.5, "in"),
                       'Business Card (Europe)'  : (    55,    85, "mm"),
                       'Business Card (Aus/NZ)'  : (    55,    90, "mm"),
                       'Arch A'                  : (     9,    12, "in"),
                       'Arch B'                  : (    12,    18, "in"),
                       'Arch C'                  : (    18,    24, "in"),
                       'Arch D'                  : (    24,    36, "in"),
                       'Arch E'                  : (    36,    48, "in"),
                       'Arch E1'                 : (    30,    42, "in")}


def run_direct(command, prog_name, verbose):
    """
    Run a command that directly outputs SVG from an input file.
    On success, exits with a return code of 0.
    On failure, outputs an error message to stderr, and exits with a return
    code of 1.
    """
    msg = None
    try:
        try:
            from subprocess import Popen, PIPE
            p = Popen(command, shell=True, stderr=PIPE)
            rc = p.wait()
            err = p.stderr.read()
            p.stderr.close()
        except ImportError:
            # shouldn't happen...
            msg = "Subprocess.Popen is not available"
        if rc and msg is None:
            msg = "%s failed:\n%s\n" % (prog_name, err)
    except Exception as inst:
        msg = "Error attempting to run %s: %s" % (prog_name, str(inst))

    # Ouput error message (if any) and exit.
    if msg is not None:
        sys.stderr.write(msg + "\n")
        sys.exit(1)
    else:
        sys.exit(0)


class InputWPI(inkex.EffectExtension):
    def __init__(self):
        # Call base class construtor.
        super(InputWPI, self).__init__()
        self.verbose = False

    def add_arguments(self, pars):
        # inklingreader options
        pars.add_argument("--dimensions",
                                     action="store", type=str,
                                     dest="dimensions", default="custom",
                                     help="Whether to use custom page dimensions")
        pars.add_argument("--dimensions_orientation",
                                     action="store", type=str,
                                     dest="dimensions_orientation", default="portrait",
                                     help="Specify page orientation for the document")
        pars.add_argument("--dimensions_width",
                                     action="store", type=float,
                                     dest="dimensions_width", default="210.0",
                                     help="Specify the page width for the document")
        pars.add_argument("--dimensions_height",
                                     action="store", type=float,
                                     dest="dimensions_height", default="297.0",
                                     help="Specify the page height for the document")
        pars.add_argument("--dimensions_units",
                                     action="store", type=str,
                                     dest="dimensions_units", default="mm",
                                     help="Specify the units for the custom document dimensions")
        pars.add_argument("--background",
                                     action="store", type=str,
                                     dest="background", default="custom",
                                     help="Whether to use a background color for the document")
        pars.add_argument("--background_color",
                                     action="store", type=str,
                                     dest="background_color", default="#ffffff",
                                     help="Specify the background color for the document")
        pars.add_argument("--foreground",
                                     action="store", type=str,
                                     dest="foreground", default="custom",
                                     help="Whether to use a list of colors for the foreground")
        pars.add_argument("--foreground_colors",
                                     action="store", type=str,
                                     dest="foreground_colors",
                                     default="#000000,#ff0000,#0000ff,#00ff00",
                                     help="Specify a list of colors (comma separated)")
        pars.add_argument("--pressure_factor",
                                     action="store", type=float,
                                     dest="pressure_factor", default="1.0",
                                     help="Specify a factor for handling pressure data")
        # tabs
        pars.add_argument("--tab",
                                     action="store", type=str,
                                     dest="tab")
        pars.add_argument("--notebook_colors",
                                     action="store", type=str,
                                     dest="notebook_colors")
        pars.add_argument("--notebook_dimensions",
                                     action="store", type=str,
                                     dest="notebook_dimensions")
        # global
        pars.add_argument("--verbose",
                                     action="store", type=inkex.Boolean,
                                     dest="verbose", default=False,
                                     help="Use verbose output for debugging")

    def wpi_import_direct(self, infile, custom_opts=''):
        """
        Import via inklingreader's direct SVG output (no temp file needed)
        """
        helper_app = 'inklingreader'
        inout_opts = ' --file "%s" --direct-output'

        # run external command
        if self.verbose:
            inkex.utils.debug(((helper_app + custom_opts + inout_opts)
                         % infile, helper_app, self.verbose))
        run_direct((helper_app + custom_opts + inout_opts)
                   % infile, helper_app, self.verbose)

    def effect(self):
        pass

    def output(self):
        pass

    def input(self):
        """
        Parse input options, construct and run external command string
        """
        infile = self.options.input_file

        # defaults
        pressure_factor_cmd = ''
        background_cmd = ''
        colors_cmd = ''
        dimensions_cmd = ''

        # pressure factor
        if self.options.pressure_factor != 1.0:
            pressure_factor_cmd = (' --pressure-factor=%.2f' % self.options.pressure_factor)

        # page background and foreground colors
        if self.options.notebook_colors == '"notebook_colors_presets"':
            if self.options.background != "default":
                try:
                    bg = background_presets[self.options.background]
                    background_cmd = (' --background="%s"' % bg)
                except:
                    pass
            if self.options.foreground != "default":
                try:
                    fg = foreground_presets[self.options.foreground]
                    colors_cmd = (' --colors="%s"' % fg)
                except:
                    pass
        elif self.options.notebook_colors == '"notebook_colors_custom"':
            background_cmd = (' --background="%s"' % self.options.background_color)
            colors_cmd = (' --colors="%s"' % self.options.foreground_colors)

        # page dimensions
        if self.options.notebook_dimensions == '"notebook_dimensions_presets"':
            try:
                # test for known PAPERSIZE
                known_papersize = inkscape_papersizes[self.options.dimensions]
                if self.options.dimensions_orientation == "portrait":
                    dimensions_cmd = (' --dimensions="%sx%s%s"'
                                      % (known_papersize[0],
                                         known_papersize[1],
                                         known_papersize[2]))
                else:  # landscape
                    dimensions_cmd = (' --dimensions="%sx%s%s"'
                                      % (known_papersize[1],
                                         known_papersize[0],
                                         known_papersize[2]))
            except:
                pass
        elif self.options.notebook_dimensions == '"notebook_dimensions_custom"':
            dimensions_cmd = (' --dimensions="%sx%s%s"'
                              % (self.options.dimensions_width,
                                 self.options.dimensions_height,
                                 self.options.dimensions_units))

        custom_options = dimensions_cmd + background_cmd + colors_cmd + pressure_factor_cmd
        # import with custom settings
        self.wpi_import_direct(infile, custom_options)

    def affect(self, args=sys.argv[1:], output=False, input=True):
        """
        Affect input document with a callback effect
        """
        self.parse_arguments(args)
        self.verbose = self.options.verbose
        self.effect()
        if output: self.output()
        if input: self.input()


if __name__ == '__main__':
    e = InputWPI()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
