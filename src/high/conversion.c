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

#include "conversion.h"

#include <malloc.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "../parsers/wpi.h"
#include "../converters/png.h"
#include "../converters/pdf.h"
#include "../converters/svg.h"


/*----------------------------------------------------------------------------.
 | CONVERT_DIRECTORY                                                          |
 | This function is a helper to convert all WPI files in a directory to SVG.  |
 '----------------------------------------------------------------------------*/
void
high_convert_directory (const char* path, GSList* coordinates)
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
high_export_to_file (GSList* data, const char* to)
{
  /* Even though this function is now deprecated, it is a fix for the GLib
   * version that is shipped with Ubuntu 12.04. */
  #ifndef GLIB_VERSION_2_36
  g_type_init ();
  #endif

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
