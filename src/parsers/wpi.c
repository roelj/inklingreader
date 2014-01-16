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

#include "wpi.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <glib.h>

#include "../datatypes/element.h"
#include "../datatypes/stroke.h"
#include "../datatypes/statistics.h"

/*----------------------------------------------------------------------------.
 | BLOCK DESCRIPTORS                                                          |
 | -------------------------------------------------------------------------- |
 |                                                                            |
 | IDENTIFIER      DESCRIPTION                                                |
 | * 241           Stroke/Layer Description                                   |
 | * 97            Pen x/y data                                               |
 | * 100           Pen pressure                                               |
 | * 101           Pen tilt                                                   |
 | * 197s17        Unknown                                                    |
 | * 194s4         Unknown                                                    |
 | * 199s28/24/20  Unknown                                                    |
 '----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------.
 | PARSE_WPI:                                                                 |
 | This function reads the data and tries to get useful data out of it.       |
 '----------------------------------------------------------------------------*/
GSList*
p_wpi_parse (const char* filename)
{
  /* Create a GSList (singly-linked list) that will be the return value of 
   * this function. */
  GSList* list = NULL;

  /* Variables for keeping track of the pressure. */
  //int max_layer_pressure = 0;
  //int max_sliding_pressure = 0;
  //int current_pen_pressure = 0;
  int max_stroke_pressure = 0;

  /* Variables for keeping track of the pen tilt. */
  //unsigned char* pen_tilt_x = NULL;
  //unsigned char* pen_tilt_y = NULL;

  /* Open the file read-only in binary mode. The binary mode is important 
   * because ftell() will only correctly return the length when in this mode */
  FILE* file = fopen (filename, "rb");

  /* Determine the size of the file.
   * FIXME: Filesize limit is now around 2GB. It would be cooler when
   * the size is virtually unlimited. In practice, we're fine with the
   * 2GB limit. */
  fseek(file, 0L, SEEK_END);
  size_t data_len = ftell(file);
  fseek(file, 0L, SEEK_SET);

  printf ("Filesize: %zu bytes.\r\n", data_len);

  /* Set up an array that can keep the entire file in memory. */
  unsigned char data[data_len];
  memset (&data, 0, data_len);

  /* Variable to keep some statistics about the file's contents. */
  dt_statistics stats;

  /* Initialize all members of 'stats' to '0'. */
  memset (&stats, 0, sizeof (dt_statistics));

  /* Read the entire file into memory. */
  fread (data, 1, data_len, file);

  /* Find out interesting places. The first 2060 bytes can be skipped (according to 
   * the PaperInkConverter program). */
  size_t count;
  for (count = 2060; count < data_len; count++)
    switch (data[count])
      {
	/*--------------------------------------------------------.
	  | PROCESS X/Y DATA FROM THE PEN.                         |
	  | 97  06  X   Y                                          |
	  | 1   1   2   2   bytes                                  |
	  '--------------------------------------------------------*/
      case BLOCK_COORDINATE:
	{
	  printf ("BLOCK_COORDINATE\r\n");
	  stats.coordinates++;

	  dt_coordinate* prev = (dt_coordinate *)list->data;
	  dt_coordinate* coordinate = malloc (sizeof (dt_coordinate));
	  if (coordinate != NULL)
	    {
	      /* 'count' should be moved up 2 bytes so the X position can be 
	       * read. */
	      count += 2;

	      /* The "<<" operator does bitshifting. A coordinate is stretched
	       * over two blocks. The first block has to be "shifted" 8 places
	       * to get the right value. */
	      coordinate->x = ((int)(char)data[count]) << 8;
	      coordinate->x = coordinate->x + (int)(data[count + 1]) + 5;

	      /* Move over to the Y-coordinate. */
	      count += 2;

	      coordinate->y = ((int)(char)data[count]) << 8;
	      coordinate->y = (((int)coordinate->y + (int)data[count + 1]) << 1)  + 5;

	      /* Move past the coordinate data so we don't read 
	       * duplicate data. (Move only 1 because the for-loop will 
	       * move the other.) */
	      count += 1;

	      /* Try to avoid duplicate data without expensive algorithms.
	       * Most duplicate coordinates are grouped already. If it isn't
	       * grouped, it should not be treated as duplicate data (two
	       * points can be the same at intersection). */
	      if (prev->x != coordinate->x || prev->y != coordinate->y)
		{
		  dt_element* element = malloc (sizeof (element));
		  if (element != NULL)
		    {
		      element->type = TYPE_COORDINATE;
		      element->data = coordinate;
		      coordinate = NULL;

		      /* Add the wrapped coordinate to the list. */
		      list = g_slist_append (list, element);
		    }
		}

	      /* Don't add a duplicate entry to the list. Instead, deallocate
	       * the allocated memory. */
	      else
		{
		  free (coordinate);
		  coordinate = NULL;
		}
	    }
	  else
	    printf ("Could not allocate enough memory.\r\n");
	}
	break;

	/*--------------------------------------------------------.
	  | PROCESS PRESSURE INFORMATION.                          |
	  | 100 06  U  U  Pressure (U=Unknown)                     |
	  | 1   1   1  1  2         bytes                          |
	  '--------------------------------------------------------*/
      case BLOCK_PRESSURE:
	{
	  printf ("BLOCK_PRESSURE\r\n");
	  stats.pressure++;

	  /* Make sure the block data has the expected size. */
	  if (data[count + 1] == 6)
	    {
	      dt_pressure* pressure = malloc (sizeof (dt_pressure));
	      if (pressure != NULL)
		{
		  count += 4;
		  pressure->pressure = (int)data[count] << 8;
		  pressure->pressure = pressure->pressure + data[count + 1];

		  /* Increase the 'max_stroke_pressure' to the value that was 
		   * obtained whenever it's bigger than the current value. */
		  if (pressure->pressure > max_stroke_pressure)
		    max_stroke_pressure = pressure->pressure;

		  dt_element* element = malloc (sizeof (dt_element));
		  if (element != NULL)
		    {
		      element->type = TYPE_PRESSURE;
		      element->data = pressure;

		      /* Add the wrapped tilt information to the list. */
		      list = g_slist_append (list, element);
		    }
		}
	      else
		printf ("Could not allocate enough memory.\r\n");
	    }
	}
	break;

	/*--------------------------------------------------------.
	  | PROCESS TILT INFORMATION.                              |
	  | 101 06  X  Y  U  U  (U=Unknown)                        |
	  | 1   1   1  1  1  1  bytes                              |
	  '--------------------------------------------------------*/
      case BLOCK_TILT:
	{
	  printf ("BLOCK_TILT\r\n");
	  stats.tilt++;

	  /* Make sure the block data has the expected size. */
	  if (data[count + 1] == 6)
	    {
	      count += 2;

	      dt_tilt* tilt = malloc (sizeof (dt_tilt));
	      if (tilt != NULL)
		{
		  tilt->x = data[count];
		  tilt->y = data[count + 1];

		  dt_element* element = malloc (sizeof (dt_element));
		  if (element != NULL)
		    {
		      element->type = TYPE_TILT;
		      element->data = tilt;
		      tilt = NULL;

		      /* Add the wrapped tilt information to the list. */
		      list = g_slist_append (list, element);
		    }
		}
	      else
		printf ("Could not allocate enough memory.\r\n");
	    }
	}
	break;
	/*--------------------------------------------------------.
	  | UNKNOWN BLOCK DESCRIPTORS.                             |
	  | DES LENGTH  U  (U=Unknown)                             |
	  | 1   1       ?  bytes                                   |
	  '--------------------------------------------------------*/
      case 194:
      case 197:
      case 199:
	stats.unknown++;
	break;

	/*--------------------------------------------------------.
	  | PROCESS STROKE INFORMATION.                            |
	  | 241 { 0|1|128 }                                        |
	  | 1   1            byte                                  |
	  '--------------------------------------------------------*/
      case BLOCK_STROKE:
	{
	  /* Move up one position. */
	  count++;

	  /* Make sure it's valid stroke information. */
	  if (data[count] == 3)
	    {
	      dt_stroke* stroke = malloc (sizeof (dt_stroke));
	      if (stroke != NULL) 
		{
		  stroke->value = data[count + 1];

		  /* Update the statistics when needed. */
		  if (stroke->value == BEGIN_STROKE) 
		    {
		      stats.strokes++;
		      printf ("BLOCK_STROKE: BEGIN STROKE\r\n");
		    }
		  else
		    printf ("BLOCK_STROKE: END STROKE\r\n");

		  /* Wrap the stroke information into a 'dt_element', so it 
		   * can be added to the list. */
		  dt_element* element = malloc (sizeof (dt_element));
		  if (element != NULL)
		    {
		      element->type = TYPE_STROKE;
		      element->data = stroke;

		      /* Add the wrapped stroke information to the list. */
		      list = g_slist_append (list, element);
		    }
		}
	    }
	}
      }

  printf ("\r\n");

  printf ("\r\nSTATISTICS\r\n%-10s %-10s %-10s %-10s %-10s %-10s\r\n"
	  "%-10d %-10d %-10d %-10d %-10d %-10d\r\n\r\n", 
	  "Objects", "Strokes", "Pen XY", "Pressure", "Tilt", "Unknown",
	  stats.objects, stats.strokes, stats.coordinates, stats.pressure,
	  stats.tilt, stats.unknown);

  printf ("Maximum stroke pressure: %d\r\n", max_stroke_pressure);

  return list;
}
