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
  fseek (file, 0L, SEEK_END);
  size_t data_len = ftell (file);
  fseek (file, 0L, SEEK_SET);

  /* Set up an array that can keep the entire file in memory. */
  unsigned char data[data_len];
  memset (&data, 0, data_len);

  /* Read the entire file into memory. This should give us some speed advantage
   * while processing the data. */
  if (fread (data, 1, data_len, file) != data_len)
    {
      printf ("An error occurred when reading the file.\r\n");
      return NULL;
    }

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
  char* file_header = malloc (header_len);

  if (file_header == NULL)
    {
      mem_error();
      fclose (file);
      return NULL;
    }

  file_header = memcpy (file_header, data, header_len);
  gchar* base64_header = g_base64_encode ((unsigned char*)file_header, header_len);

  if (strcmp (base64_header, header))
    {
      printf ("This file is not a (supported) WPI file.\r\n");
      free (file_header);
      fclose (file);
      return NULL;
    }

  g_free (base64_header);
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
	  dt_coordinate* coordinate = malloc (sizeof (dt_coordinate));
	  if (coordinate != NULL)
	    {
	      coordinate->type = TYPE_COORDINATE;
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

	      list = g_slist_prepend (list, coordinate);
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
	  /* Make sure the block data has the expected size. */
	  if (data[count + 1] == 6)
	    {
	      dt_pressure* pressure = malloc (sizeof (dt_pressure));
	      if (pressure != NULL)
		{
		  pressure->type = TYPE_PRESSURE;
		  count += 4;
		  pressure->pressure = (int)data[count] << 8;
		  pressure->pressure = pressure->pressure + data[count + 1];

		  list = g_slist_prepend (list, pressure);
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
	  /* Make sure the block data has the expected size. */
	  if (data[count + 1] == 6)
	    {
	      count += 2;

	      dt_tilt* tilt = malloc (sizeof (dt_tilt));
	      if (tilt != NULL)
		{
		  tilt->type = TYPE_TILT;
		  tilt->x = data[count];
		  tilt->y = data[count + 1];

		  list = g_slist_prepend (list, tilt);
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
	//case 194:
	//case 197:
	//case 199:
	//break;

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
		  stroke->type = TYPE_STROKE;
		  stroke->value = data[count + 1];

		  list = g_slist_prepend (list, stroke);
		}
	    }
	}
      }

  list = g_slist_reverse (list);
  fclose (file);
  return list;
}
