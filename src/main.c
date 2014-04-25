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
 | - high_:  Provides functions that combine other functions ("high level").  |
 '----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "parsers/wpi.h"
#include "datatypes/element.h"
#include "datatypes/configuration.h"
#include "gui/mainwindow.h"
#include "high/conversion.h"
#include "high/configuration.h"

/* This struct stores various run-time configuration options to allow 
 * customization of the behavior of the program. */
dt_configuration settings;

/*----------------------------------------------------------------------------.
 | SHOW_VERSION                                                               |
 | This function displays the program's current version.                      |
 '----------------------------------------------------------------------------*/
void
show_version ()
{
  printf ("Version: 0.6\r\n");
}


/*----------------------------------------------------------------------------.
 | SHOW_HELP                                                                  |
 | This function displays the possible command-line arguments.                |
 '----------------------------------------------------------------------------*/
void 
show_help ()
{
  printf ("\r\nAvailable options:\r\n"
	  "  --background,        -b  Specify the background color for the document\r\n"
	  "  --colors,            -c  Specify a list of colors (comma separated)\r\n"
	  "  --pressure-factor,   -p  Specify a factor for handling pressure data.\r\n"
	  "  --convert-directory, -d  Convert all WPI files in a directory.\r\n"
	  "  --file,              -f  Specify the WPI file to convert.\r\n"
	  "  --to,                -t  Specify the file to write to.\r\n"
	  "  --merge,             -m  Merge two WPI files into one.\r\n"
	  "  --gui,               -g  Start the graphical user interface.\r\n"
	  "  --version,           -v  Show versioning information.\r\n"
	  "  --help,              -h  Show this message.\r\n\r\n");
}

/*----------------------------------------------------------------------------.
 | CLEANUP_CONFIGURATION                                                      |
 | This function should be run to free memory that was malloc'd in the        |
 | configuration object.                                                      |
 '----------------------------------------------------------------------------*/
void cleanup_configuration ()
{
  high_configuration_cleanup (&settings);
}


/*----------------------------------------------------------------------------.
 | MAIN                                                                       |
 | Execution of the program starts here.                                      |
 '----------------------------------------------------------------------------*/
int
main (int argc, char** argv)
{
  /* A variable that controls whether the graphical user interface should be 
   * opened or not. 0 means "don't open" and 1 means "open". */
  unsigned char launch_gui = 0;
  unsigned char config_set = 0;
  settings.pressure_factor = 1.0;

  if (argc > 1)
    {
      int arg = 0;
      int index = 0;
      GSList* coordinates = NULL;
      char* merge_val = NULL;

      /*----------------------------------------------------------------------.
       | OPTIONS                                                              |
       | An array of structs that list all possible arguments that can be     |
       | provided by the user.                                                |
       '----------------------------------------------------------------------*/
      static struct option options[] =
	{
	  { "background",        required_argument, 0, 'b' },
	  { "colors",            required_argument, 0, 'c' },
	  { "convert-directory", required_argument, 0, 'd' },
	  { "file",              required_argument, 0, 'f' },
	  { "config",            required_argument, 0, 'e' },
	  { "merge",             required_argument, 0, 'm' },
	  { "pressure-factor",   required_argument, 0, 'p' },
	  { "to",                required_argument, 0, 't' },
	  { "gui",               optional_argument, 0, 'g' },
	  { "help",              no_argument,       0, 'h' },
	  { "version",           no_argument,       0, 'v' },
	  { 0,                   0,                 0, 0   }
	};

      while ( arg != -1 )
	{
	  /* Make sure to list all short options in the string below. */
	  arg = getopt_long (argc, argv, "b:c:d:s:f:m:p:t:g:vh", options, &index);

	  switch (arg)
	    {
	      /*--------------------------------------------------------------.
	       | OPTION: BACKGROUND                                           |
	       | Let's the user specify the background color.                 |
	       '--------------------------------------------------------------*/
	    case 'b':
	      if (optarg)
		{
		  settings.background = optarg;
		  launch_gui = 1;
		}
	      break;


	      /*--------------------------------------------------------------.
	       | OPTION: COLORS                                               |
	       | Let's the user specify a range of colors.                    |
	       '--------------------------------------------------------------*/
	    case 'c':
	      if (optarg)
		{
		  settings.colors = high_parse_colors (optarg, &settings.num_colors);
		  atexit (cleanup_configuration);
		  launch_gui = 1;
		}
	      break;

	      /*--------------------------------------------------------------.
	       | OPTION: CONVERT-DIRECTORY                                    |
	       | Convert all WPI files in a given directory.                  |
	       '--------------------------------------------------------------*/
	    case 'd':
	      {
		if (optarg)
		  high_convert_directory (optarg, coordinates);
		launch_gui = 0;
	      }
	      break;

	      /*--------------------------------------------------------------.
	       | OPTION: CONFIG                                               |
	       | Read a configuration file.                                   |
	       '--------------------------------------------------------------*/
	    case 'e':
	      {
		if (optarg)
		  high_parse_configuration (optarg, &settings);
		config_set = 1;
	      }
	      break;

	      /*--------------------------------------------------------------.
	       | OPTION: FILE                                                 |
	       | Use in combination with TO, to convert file.                 |
	       '--------------------------------------------------------------*/
	    case 'f':
	      {
		if (optarg)
		  coordinates = p_wpi_parse (optarg);
		launch_gui = 0;
	      }
	      break;

	      /*--------------------------------------------------------------.
	       | OPTION: MERGE                                                |
	       | Use with TO to merge two files.                              |
	       '--------------------------------------------------------------*/
	    case 'm':
	      {
		if (optarg)
		  merge_val = optarg;
		launch_gui = 0;
	      }
	      break;

	      /*--------------------------------------------------------------.
	       | OPTION: PRESSURE-FACTOR                                      |
	       | Sets the factor for handling pressure data.                  |
	       '--------------------------------------------------------------*/
	    case 'p':
	      {
		if (optarg)
		  settings.pressure_factor = atof (optarg);
		launch_gui = 1;
	      }
	      break;

	      /*--------------------------------------------------------------.
	       | OPTION: TO                                                   |
	       | Use with FILE to convert a file.                             |
	       '--------------------------------------------------------------*/
	    case 't':
	      {
		if (optarg)
		  {
		    if (merge_val)
		      high_merge_wpi_files (merge_val, optarg);
		    else
		      high_export_to_file (coordinates, optarg);
		  }
		launch_gui = 0;
	      }
	      break;

	      /*--------------------------------------------------------------.
	       | OPTION: GUI                                                  |
	       | Start the graphical user interface.                          |
	       '--------------------------------------------------------------*/
	    case 'g':
	      {
		gui_init_mainwindow (argc, argv, optarg);
		launch_gui = 0;
	      }
	      break;

	      /*--------------------------------------------------------------.
	       | OPTION: HELP                                                 |
	       | Show a help message.                                         |
	       '--------------------------------------------------------------*/
	    case 'h':
	      show_help ();
	      break;

	      /*--------------------------------------------------------------.
	       | OPTION: VERSION                                              |
	       | Show version information.                                    |
	       '--------------------------------------------------------------*/
	    case 'v':
	      show_version ();
	      break;
	    };
	}
    }
  else
    launch_gui = 1;


  /* Read the default configuration file. */
  struct passwd *pw = getpwuid(getuid());
  const char *homedir = pw->pw_dir;
  char* conf_loc = malloc (strlen (pw->pw_dir) + 17);
  if (conf_loc != NULL)
    {
      sprintf (conf_loc, "%s/%s", pw->pw_dir, ".inklingreaderrc");
      if (access (conf_loc, F_OK) != -1)
	high_parse_configuration (conf_loc, &settings);
    }

  if (launch_gui == 1)
    {
      /* Set a default color before launching the GUI. */
      if (settings.num_colors == 0)
	{
	  settings.colors = malloc (sizeof (char*));
	  settings.colors[0] = malloc (8);
	  snprintf (settings.colors[0], 8, "#00007c");
	  settings.num_colors = 1;
	}

      gui_init_mainwindow (argc, argv, NULL);
    }
  return 0;
}
