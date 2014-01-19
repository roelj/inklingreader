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
  /* This function is called in several situations. Declaring it once
   * here as inline saves some space in the compiled object. */
  inline void mem_error ()
  {
    printf ("%s: Could not allocate enough memory.\r\n", __func__);
  }

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

  if (file == NULL)
    {
      printf ("Cannot open file '%s'\r\n", filename);
      return NULL;
    }
  /* Determine the size of the file.
   * FIXME: Filesize limit is now around 2GB. It would be cooler when
   * the size is virtually unlimited. In practice, we're fine with the
   * 2GB limit. */
  fseek(file, 0L, SEEK_END);
  size_t data_len = ftell(file);
  fseek(file, 0L, SEEK_SET);

  /* Set up an array that can keep the entire file in memory. */
  unsigned char data[data_len];
  memset (&data, 0, data_len);

  /* Variable to keep some statistics about the file's contents. */
  dt_statistics stats;

  /* Initialize all members of 'stats' to '0'. */
  memset (&stats, 0, sizeof (dt_statistics));

  /* Read the entire file into memory. This should give us some speed advantage
   * while processing the data. */
  fread (data, 1, data_len, file);

  /* The first 322 bytes seem to be equal for every WPI file. I've encoded 
   * these 322 bytes using the base64 encoding algorithm. The result of this
   * encoding can be found in the variable 'header'. Comparing the first 322
   * bytes from the loaded file can filter out malformed WPI files. */
  char* header = 
    "AQbwAh4AEQk10/Kz8hcAIQ8AAmJaCQAAYQcAAAAAJiQAAAAAAAAAAAAAAcvr//+UATud///GAU"
    "Yw//+0ATEj///9IyTyLIA/7ax2tjxZCjsLFaI/R55iOkhvmbWb1oA/EwieQAAAJy0AAAAH///l"
    "+hjdFX0L6vfWCFey/odHABikBnMS616JF2Qd9ZlzAflX/92H8QOAJQ8AAgIBAAADAAACAAAA8Q"
    "MAKLQBAAAAzwEAAAAQAAADAAAAAAAAAQARAAADAAAAekLSLAAgAAADAAAAAAAAAAAhAAADAAAA"
    "rQAAAAAkAAADAAAAAgAAAAAlAAADAAAAWgAAAAAmAAADAAAAQQAAAAAnAAADAAAAZHS8ygAwAA"
    "AFAAAA1P7//wAAAAAUAAAAATAAAAUAAAAsAQAAAAAAABQAAAAAMwAAAwAAAA==";

  size_t header_len = 322;
  char* file_header = malloc (header_len + 1);

  if (file_header == NULL)
    {
      mem_error();
      fclose (file);
      return NULL;
    }

  file_header = memset (file_header, '\0', header_len + 1);
  file_header = memcpy (file_header, data, header_len);
  file_header = g_base64_encode ((unsigned char*)file_header, header_len);

  if (strcmp (file_header, header))
    {
      printf ("This file is not a (supported) WPI file.\r\n");
      free (file_header);
      fclose (file);
      return NULL;
    }

  free (file_header);
  file_header = NULL;

  /* Find out interesting places. The first 2040 bytes can be skipped (according
   * to the PaperInkConverter program). This data seems to tell something about
   * the Inkling device (this could be firmware versions, or a unique identifier
   * for each Inkling device. */
  size_t count;
  for (count = 2040; count < data_len; count++)
    switch (data[count])
      {
	/*--------------------------------------------------------.
	 | PROCESS X/Y DATA FROM THE PEN.                         |
	 | 97  06  X   Y                                          |
	 | 1   1   2   2   bytes                                  |
	 '--------------------------------------------------------*/
      case BLOCK_COORDINATE:
	{
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
	      coordinate->y = (((int)coordinate->y + (int)data[count + 1]) << 1) + 5;

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

		      /* Add the wrapped coordinate to the list. */
		      list = g_slist_append (list, element);

		      /* Figure out the boundaries on the drawing. */
		      if (coordinate->y > stats.top) stats.top = coordinate->y;
		      if (coordinate->y < stats.bottom) stats.bottom = coordinate->y;
		      if (coordinate->x < stats.left) stats.left = coordinate->x;
		      if (coordinate->x > stats.right) stats.right = coordinate->x;

		      coordinate = NULL;
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
	    mem_error();
	}
	break;

	/*--------------------------------------------------------.
	 | PROCESS PRESSURE INFORMATION.                          |
	 | 100 06  U  U  Pressure (U=Unknown)                     |
	 | 1   1   1  1  2         bytes                          |
	 '--------------------------------------------------------*/
      case BLOCK_PRESSURE:
	{
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
		mem_error();
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
		mem_error();
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
		    stats.strokes++;

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

  printf ("Outer boundary (top, right, bottom, left): (%f, %f, %f, %f)\r\n", 
	  stats.top, stats.right, stats.bottom, stats.left);

  fclose (file);
  return list;
}
