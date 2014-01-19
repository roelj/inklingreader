/*
 * Copyright (C) 2013  Roel Janssen <roel@moefel.org>
 *
 * This file is part of InklingReader
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*----------------------------------------------------------------------------.
 | STRUCTURE OF THE CODE                                                      |
 |                                                                            |
 | I tried to make the code compilable on all major GNU distributions. I      |
 | based this project on code provided by Herbert Ellebruch's                 |
 | PaperInkRecognizer. Unfortunately his code is for Windows only.            |
 |                                                                            |
 | Namespaces used throughout the program:                                    |
 | - p_:     Parsers                                                          |
 | - gui_:   Graphical User Interface                                         |
 | - co_:    Converters                                                       |
 '----------------------------------------------------------------------------*/

#include <stdio.h>
#include <getopt.h>
#include "parsers/wpi.h"
#include "datatypes/element.h"
#include "gui/mainwindow.h"
#include "converters/svg.h"

int
main (int argc, char** argv)
{
  int arg = 0;
  int index = 0;
  GSList* coordinates = NULL;

  /*--------------------------------------------------------------------------.
   | OPTIONS                                                                  |
   | An array of structs that list all possible arguments that can be         |
   | provided by the user.                                                    |
   '--------------------------------------------------------------------------*/
  static struct option options[] =
  {
    { "file",            required_argument, 0, 'f' },
    { "to",              required_argument, 0, 't' },
    { "gui",             no_argument,       0, 'g' },
    { "help",            no_argument,       0, 'h' },
    { "version",         no_argument,       0, 'v' },
    { 0,                 0,                 0, 0   }
  };

  while ( arg != -1 )
  {
    /* Make sure to list all short options in the string below. */
    arg = getopt_long (argc, argv, "f:t:gvh", options, &index);

    switch (arg)
    {
    /*--------------------------------------------------------------------.
     | OPTION: FILE                                                       |
     | The user can provide a file to parse and convert to something      |
     | useful.                                                            |
     '--------------------------------------------------------------------*/
    case 'f':
      if (optarg)
	coordinates = p_wpi_parse (optarg);
      break;

    case 't':
      if (optarg)
	co_write_svg_file (optarg, coordinates);
      break;

    case 'g':
      gui_init_mainwindow (argc, argv);
      break;

    case 'h':
      printf ("\r\nAvailable options:\r\n"
	      "  --file,    -f     Specify the WPI file to convert.\r\n"
	      "  --to,      -t     Specify the file to write to.\r\n"
	      "  --gui,     -g     Start the graphical user interface.\r\n"
	      "  --version, -v     Show versioning information.\r\n"
	      "  --help,    -h     Show this message.\r\n\r\n");
      break;
    case 'v':
      break;

    };
  }

  return 0;
}
