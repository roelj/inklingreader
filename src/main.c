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
#include "converters/png.h"
#include "converters/pdf.h"
#include "converters/svg.h"

#include <malloc.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>


/*----------------------------------------------------------------------------.
 | SHOW_VERSION                                                               |
 | This function displays the program's current version.                      |
 '----------------------------------------------------------------------------*/
void
show_version ()
{
  printf ("Version: 0.4\r\n");
}


/*----------------------------------------------------------------------------.
 | SHOW_HELP                                                                  |
 | This function displays the possible command-line arguments.                |
 '----------------------------------------------------------------------------*/
void 
show_help ()
{
  printf ("\r\nAvailable options:\r\n"
	  "  --convert-directory  -c  Convert all WPI files in a directory.\r\n"
	  "  --file,              -f  Specify the WPI file to convert.\r\n"
	  "  --to,                -t  Specify the file to write to.\r\n"
	  "  --gui,               -g  Start the graphical user interface.\r\n"
	  "  --version,           -v  Show versioning information.\r\n"
	  "  --help,              -h  Show this message.\r\n\r\n");
}


/*----------------------------------------------------------------------------.
 | CONVERT_DIRECTORY                                                          |
 | This function is a helper to convert all WPI files in a directory to SVG.  |
 '----------------------------------------------------------------------------*/
void
convert_directory (const char* path, GSList* coordinates)
{
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


/*----------------------------------------------------------------------------.
 | EXPORT_TO_FILE                                                             |
 | This function is a helper to enable exporting to all supported filetypes.  |
 '----------------------------------------------------------------------------*/
void
export_to_file (GSList* data, const char* to)
{
  inline void unsupported ()
  {
    printf ("Only PNG (.png), SVG (.svg) and PDF (.pdf) are supported.\r\n");
  }

  if (strlen (to) > 3)
    {
      const char* extension = to + strlen (to) - 3;
      char* svg_data = co_svg_create (data, to);

      if (!strcmp (extension, "png"))
	co_png_export_to_file (to, svg_data);
      else if (!strcmp (extension, "pdf"))
	co_pdf_export_to_file (to, svg_data);
      else if (!strcmp (extension, "svg"))
	{
	  FILE* file;
	  file = fopen (to, "w");
	  if (file != NULL)
	    fwrite (svg_data, strlen (svg_data), 1, file);

	  fclose (file);
	}
      else
	unsupported ();

      free (svg_data);
    }
  else
    unsupported ();
}


/*----------------------------------------------------------------------------.
 | MAIN                                                                       |
 | Execution of the program starts here.                                      |
 '----------------------------------------------------------------------------*/
int
main (int argc, char** argv)
{
  if (argc > 1)
    {

      int arg = 0;
      int index = 0;
      GSList* coordinates = NULL;

      /*----------------------------------------------------------------------.
       | OPTIONS                                                              |
       | An array of structs that list all possible arguments that can be     |
       | provided by the user.                                                |
       '----------------------------------------------------------------------*/
      static struct option options[] =
	{
	  { "convert-directory", required_argument, 0, 'c' },
	  { "file",              required_argument, 0, 'f' },
	  { "to",                required_argument, 0, 't' },
	  { "gui",               optional_argument, 0, 'g' },
	  { "help",              no_argument,       0, 'h' },
	  { "version",           no_argument,       0, 'v' },
	  { 0,                   0,                 0, 0   }
	};

      while ( arg != -1 )
	{
	  /* Make sure to list all short options in the string below. */
	  arg = getopt_long (argc, argv, "c:f:t:g:vh", options, &index);

	  switch (arg)
	    {
	      /*--------------------------------------------------------------.
	       | OPTION: CONVERT-DIRECTORY                                    |
	       | Convert all WPI files in a given directory.                  |
	       '--------------------------------------------------------------*/
	    case 'c':
	      if (optarg)
		convert_directory (optarg, coordinates);
	      break;

	      /*--------------------------------------------------------------.
	       | OPTION: FILE                                                 |
	       | Use in combination with TO, to convert file.                 |
	       '--------------------------------------------------------------*/
	    case 'f':
	      if (optarg)
		coordinates = p_wpi_parse (optarg);
	      break;

	      /*--------------------------------------------------------------.
	       | OPTION: TO                                                   |
	       | Use with FILE to convert a file.                             |
	       '--------------------------------------------------------------*/
	    case 't':
	      if (optarg)
		export_to_file (coordinates, optarg);
	      //co_svg_create_file (optarg, coordinates);
	      break;

	      /*--------------------------------------------------------------.
	       | OPTION: GUI                                                  |
	       | Start the graphical user interface.                          |
	       '--------------------------------------------------------------*/
	    case 'g':
	      gui_init_mainwindow (argc, argv, optarg);
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
    show_help ();

  return 0;
}
