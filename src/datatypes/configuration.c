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

#include "configuration.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/*----------------------------------------------------------------------------.
 | DT_CONFIGURATION_PARSE_COLORS                                              |
 | This function puts the provided colors in an array.                        |
 '----------------------------------------------------------------------------*/
void
dt_configuration_parse_colors (const char* data, dt_configuration* config)
{
  config->num_colors = 1;
  size_t data_len = strlen (data);

  int index = 0;
  for (; index != data_len; index++)
    if (data[index] == ',') config->num_colors++;

  size_t colors_len = config->num_colors * sizeof (char*);
  config->colors = calloc (1, colors_len);
  if (config->colors == NULL) return;

  char* last;
  char* ptr = last = (char*)data;
  index = 0;
  while ((ptr = strchr (ptr, ',')) != NULL)
    {
      size_t color_len = 1 + ptr - last;
      if (color_len <= 1) break;

      config->colors[index] = calloc (1, color_len);
      if (config->colors[index] == NULL) break;

      config->colors[index] = strncpy (config->colors[index], last, color_len - 1);
      ptr++;
      index++;
      last = ptr;
    }

  /* The last color doesn't have a comma after it. */
  if (strlen (last) > 0)
    {
      size_t color_len = strlen (last);
      config->colors[index] = calloc (1, color_len + 1);
      if (config->colors[index] == NULL) return;

      config->colors[index] = strncpy (config->colors[index], last, color_len);
    }
}

/*----------------------------------------------------------------------------.
 | DT_CLEANUP_CONFIGURATION                                                   |
 | This function cleans up data that was malloc'd in a dt_configuration.      |
 '----------------------------------------------------------------------------*/
void
dt_configuration_cleanup (dt_configuration* config)
{
  if (config->num_colors > 0)
    {
      int a = 0;
      for (; a < config->num_colors; a++)
	free (config->colors[a]), config->colors[a] = NULL;

      free (config->colors), config->colors = NULL;
      config->num_colors = 0;
    }

  if (config->page.measurement)
    free (config->page.measurement), config->page.measurement = NULL;

  if (config->page.orientation)
    free (config->page.orientation), config->page.orientation = NULL;

  if (config->background)
    free (config->background), config->background = NULL;

  if (config->config_location)
    free (config->config_location), config->config_location = NULL;
}

/*----------------------------------------------------------------------------.
 | DT_CONFIGURATION_PARSE                                                     |
 | This function reads the file and tries to set configuration options.       |
 '----------------------------------------------------------------------------*/
void
dt_configuration_parse (const char* filename, dt_configuration* config)
{
  if (access (filename, F_OK) != -1)
    {
      FILE* file;
      file = fopen (filename, "r");
      char* line = NULL;
      size_t line_len = 0;
      ssize_t read = 0;

      if (file == NULL)
	perror ("fopen");
      else
	{
	  while ((read = getline (&line, &line_len, file)) != -1)
	    {
	      char* location = 0;
	      if ((location = strstr (line, "colors = ")) != NULL)
		{
		  char* newline = strchr (line, '\r');
		  if (newline == NULL) newline = strchr (line, '\n');
		  if (newline != NULL) line[newline - line] = '\0';

		  location += 9;
		  dt_configuration_parse_colors (location, config);
		}

	      else if ((location = strstr (line, "background = ")) != NULL)
		{
		  char* newline = strchr (line, '\r');
		  if (newline == NULL) newline = strchr (line, '\n');
		  if (newline != NULL) line[newline - line] = '\0';

		  location += 13;
		  config->background = malloc (strlen (location) + 1);
		  memset (config->background, '0', strlen (location) + 1);
		  if (config->background != NULL)
		    sprintf (config->background, "%s", location);
		}

	      else if ((location = strstr (line, "pressure-factor = ")) != NULL)
		{
		  char* newline = strchr (line, '\r');
		  if (newline == NULL) newline = strchr (line, '\n');
		  if (newline != NULL) line[newline - line] = '\0';

		  location += 17;
		  config->pressure_factor = atof (location);
		}
	      else if ((location = strstr (line, "dimensions = ")) != NULL)
		{
		  char* newline = strchr (line, '\r');
		  if (newline == NULL) newline = strchr (line, '\n');
		  if (newline != NULL) line[newline - line] = '\0';

		  location += 13;
		  dt_configuration_parse_dimensions (location, config);
		}

	      free (line), line = NULL;
	    }

	  free (line), line = NULL;
	  fclose (file);
	}
    }
}

/*----------------------------------------------------------------------------.
 | DT_CONFIGURATION_PARSE_DIMENSIONS                                          |
 | This function parses page dimensions and sets them in 'config'.            |
 '----------------------------------------------------------------------------*/
void
dt_configuration_parse_dimensions (const char* data, dt_configuration* config)
{
  if (data == NULL)
    {
      if (config->page.width == 0) config->page.width = 210;
      if (config->page.height == 0) config->page.height = 297;

      if (config->page.measurement == NULL)
	config->page.measurement = calloc (1, 3);

      if (config->page.measurement != NULL) 
	config->page.measurement = strncpy (config->page.measurement, "mm", 2);
    }
  else if (sscanf (data, "%lfx%lf%s", &config->page.width, &config->page.height, 
		   config->page.measurement) != 3)
    dt_configuration_parse_preset_dimensions (data, config);

}

void
dt_configuration_parse_preset_dimensions (const char* data, dt_configuration* config)
{
  static dt_preset_dimensions formats[] =
    {
      { "A4",           210,   297,   "mm" },
      { "US Letter",    216,   297.4, "mm" },
      { "US Legal",     216,   355.6, "mm" },
      { "US Executive", 184.2, 266.7, "mm" },
      { "A0",           841,   1189,  "mm" },
      { "A1",           594,   841,   "mm" },
      { "A2",           420,   594,   "mm" },
      { "A3",           297,   420,   "mm" },
      { "A5",           148,   210,   "mm" },
      { "A6",           105,   148,   "mm" },
      { "A7",           74,    105,   "mm" },
      { "A8",           52,    74,    "mm" },
      { "A9",           37,    52,    "mm" },
      { "A10",          26,    37,    "mm" },
      { "B0",           1000,  1414,  "mm" },
      { "B1",           707,   1000,  "mm" },
      { "B2",           500,   707,   "mm" },
      { "B3",           353,   500,   "mm" },
      { "B4",           250,   353,   "mm" },
      { "B5",           176,   250,   "mm" },
      { "B6",           125,   176,   "mm" },
      { "B7",           88,    125,   "mm" },
      { "B8",           62,    88,    "mm" },
      { "B9",           44,    62,    "mm" },
      { "B10",          31,    44,    "mm" },
      { "C0",           917,   1297,  "mm" },
      { "C1",           648,   917,   "mm" },
      { "C2",           458,   648,   "mm" },
      { "C3",           324,   458,   "mm" },
      { "C4",           229,   324,   "mm" },
      { "C5",           162,   229,   "mm" },
      { "C6",           114,   162,   "mm" },
      { "C7",           81,    114,   "mm" },
      { "C8",           57,    81,    "mm" },
      { "C9",           40,    57,    "mm" },
      { "C10",          28,    40,    "mm" },
      { NULL,           0,     0,     NULL }
    };

  int a = 0;
  while (formats[a].name != NULL)
    {
      if (!strcmp (data, formats[a].name))
	{
	  config->page.width = formats[a].width;
	  config->page.height = formats[a].height;
	  config->page.measurement = calloc (1, 3);
	  config->page.measurement = strncpy (config->page.measurement, 
					      formats[a].measurement, 2);
	  break;
	} 
      a++;
    }
}
