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

#include <string.h>
#include <malloc.h>

/*----------------------------------------------------------------------------.
 | HIGH_PARSE_COLORS                                                          |
 | This function puts the provided colors in an array.                        |
 '----------------------------------------------------------------------------*/
char**
high_parse_colors (const char* data, int* num_colors)
{
  char** colors = NULL;
  int count = 1;
  size_t data_len = strlen (data);

  int i = 0;
  for (; i != data_len; i++)
    if (data[i] == ',') count++;

  size_t colors_len = count * sizeof (char*);
  colors = malloc (colors_len);
  colors = memset (colors, '\0', colors_len);

  char* all = malloc (data_len + 1);
  all = strncpy (all, data, data_len);
  all[data_len] = '\0';

  colors[0] = all;
  char* ptr = all;
  int index = 1;
  while ((ptr = strchr (ptr, ',')) != NULL)
    {
      *ptr = '\0';
      ptr++;

      colors[index] = ptr;
      index++;
    }

  *num_colors = count;
  return colors;
}
