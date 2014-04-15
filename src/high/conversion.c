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

#include <stdlib.h>
#if !defined(__APPLE__)
#include <malloc.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#include "../datatypes/element.h"
#include "../parsers/wpi.h"
#include "../converters/png.h"
#include "../converters/pdf.h"
#include "../converters/svg.h"
#include "../converters/json.h"

/* nested inline function turned into global static inline function for clang
 * see also: <https://wiki.freebsd.org/PortsAndClang#Build_failures_with_fixes> */
static inline void unsupported ()
{
  printf ("Only PNG (.png), SVG (.svg), PDF (.pdf) and JSON (.json) are supported.\r\n");
}

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

  if (strlen (to) > 4)
    {
      const char* extension = to + strlen (to) - 4;
      if (!strcmp (extension, "json"))
	co_json_create_file (to, data);
      else
	{
	  char* svg_data = co_svg_create (data, to);

	  if (!strcmp (extension + 1, "png"))
	    co_png_export_to_file (to, svg_data);
	  else if (!strcmp (extension + 1, "pdf"))
	    co_pdf_export_to_file (to, svg_data);
	  else if (!strcmp (extension + 1, "svg"))
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
    }
  else
    unsupported ();
}


/*----------------------------------------------------------------------------.
 | MERGE_WPI_FILES:                                                           |
 | This function merges two WPI files.                                        |
 '----------------------------------------------------------------------------*/
void
high_merge_wpi_files (const char* first, const char* second)
{
  char* ext1 = (char*)first + strlen (first) - 3;
  char* ext2 = (char*)second + strlen (second) - 3;

  if (strcmp (ext1, "WPI") || strcmp (ext2, "WPI"))
    printf ("I can only merge files with a .WPI extension.\r\n");
  else
    {
      FILE* file1 = fopen (first, "rb");
      FILE* file2 = fopen (second, "rb");

      if (!file1 && !file2)
	printf ("Couldn't open all files.\r\n");
      else
	{
	  /* Check out the length of the files. */
	  struct stat info1, info2;
	  stat (first, &info1);
	  stat (second, &info2);

	  /* Allocate memory to store both files in one variable. */
	  size_t data_len = info1.st_size + info2.st_size - 2040;
	  unsigned char* data = malloc (data_len + 4);
	  size_t written = 0;

	  /* Put the contents of the files in the memory. */
	  written += fread (data, 1, info1.st_size, file1);
	  fclose (file1);

	  /* Put the data of the second file in a separate layer. */
	  data[written] = BLOCK_STROKE;
	  data[written + 1] = 3;
	  data[written + 2] = NEW_LAYER;
	  written += 3;

	  /* Append the data from the second file. */
	  fseek (file2, 2040, SEEK_SET);
	  written += fread (data + written, 1, info2.st_size - 2040, file2);
	  fclose (file2);

	  /* Write the combined data to the second file. */
	  FILE* output = fopen (second, "wb");
	  if (!file2)
	    printf ("Couldn't write to '%s'\r\n", second);
	  else
	    {
	      data[written] = '\0';
	      fwrite (data, data_len + 1, 1, output);
	      fclose (output);
	    }

	  free (data);
	}
    }
}
