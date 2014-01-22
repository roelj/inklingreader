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

#include <malloc.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>

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
    { "convert-directory", required_argument, 0, 'c' },
    { "file",              required_argument, 0, 'f' },
    { "to",                required_argument, 0, 't' },
    { "gui",               no_argument,       0, 'g' },
    { "help",              no_argument,       0, 'h' },
    { "version",           no_argument,       0, 'v' },
    { 0,                   0,                 0, 0   }
  };

  while ( arg != -1 )
  {
    /* Make sure to list all short options in the string below. */
    arg = getopt_long (argc, argv, "c:f:t:gvh", options, &index);

    switch (arg)
    {
    case 'c':
      {
	if (optarg)
	  {
	    char* path = optarg;

	    DIR* directory;
	    struct dirent* entry;

	    directory = opendir (path);
	    while ((entry = readdir (directory)) != NULL)
	      {
		/* Don't show files starting with a dot, '.' and '..' and only show 
		 * files with the WPI extension (others are not relevant). */
		char* extension = entry->d_name + strlen (entry->d_name) - 3;
		if (entry->d_name[0] != '.' && !strcmp (extension, "WPI"))
		  {
		    size_t name_len = strlen (path) + strlen (entry->d_name) + 2;
		    char* name = malloc (name_len);
		    char* new_name = malloc (name_len);
		    if (name == NULL || new_name == NULL) break;

		    /* Construct a string that holds "path/name". */
		    snprintf (name, name_len, "%s/%s", path, entry->d_name);

		    /* Parse a file.*/
		    coordinates = p_wpi_parse (name);

		    /* Construct a string for the new filename. */
		    snprintf (new_name, name_len - 3, "%s/%s", path, entry->d_name);
		    strcat (new_name, "svg");

		    /* Convert a file. */
		    co_svg_create_file (new_name, coordinates);

		    /* Clean up. 
		     * TODO: coordinates is leaking a lot of of memory! */
		    coordinates = NULL;
		    free (name);
		    free (new_name);
		  }
	      }
	    closedir (directory);
	  }
      }
      break;
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
	co_svg_create_file (optarg, coordinates);
      break;

    case 'g':
      gui_init_mainwindow (argc, argv);
      break;

    case 'h':
      printf ("\r\nAvailable options:\r\n"
	      "  --convert-directory  -c  Convert all WPI files in a directory.\r\n"
	      "  --file,              -f  Specify the WPI file to convert.\r\n"
	      "  --to,                -t  Specify the file to write to.\r\n"
	      "  --gui,               -g  Start the graphical user interface.\r\n"
	      "  --version,           -v  Show versioning information.\r\n"
	      "  --help,              -h  Show this message.\r\n\r\n");
      break;
    case 'v':
      break;

    };
  }

  return 0;
}
