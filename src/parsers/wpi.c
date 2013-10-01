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
parse_wpi (const char* filename)
{
  /* Create a GSList (singly-linked list) that will be the return value of 
   * this function. */
  GSList* list = NULL;

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

  printf("Filesize: %zu bytes.\r\n", data_len);

  /* Set up an array that can keep the entire file in memory. */
  unsigned char data[data_len];
  memset (&data, 0, data_len);

  /* Variable to keep some statistics about the file's contents. */
  dt_statistics stats;

  /* Initialize all members of 'stats' to '0'. */
  memset (&stats, 0, sizeof (dt_statistics));

  /* Read the entire file into memory. */
  fread (data, 1, data_len, file);

  /* Find out interesting places. */
  size_t count;
  for (count = 2060; count < data_len; count++)
    {
      switch (data[count])
	{
	  /*--------------------------------------------------------.
	   | PROCESS X/Y DATA FROM THE PEN.                         |
	   | 97  06  X   Y                                          |
	   | 1   1   2   2   bytes                                  |
	   '--------------------------------------------------------*/
	case 97:
	  {
	    stats.coordinates++;

	    dt_coordinate* coordinate = malloc (sizeof (dt_coordinate));
	    if (coordinate != NULL)
	      {
		/* 'count' should be moved up 2 bytes so it the X 
		 * position can be read. */
		count += 2;

		/* The "<<" operator does bitshifting. A coordinate is stretched
		 * over two blocks. The first block has to be "shifted" 8 places
		 * to get the right value. */
		coordinate->x = (data[count] << 8) + data[count + 1] + 5;

		/* Move over to the Y-coordinate. */
		count += 2;

		coordinate->y = (((data[count] << 8) + data[count + 1]) << 1) + 5;

		/* Move past the coordinate data so we don't read 
		 * duplicate data. (Move only 1 because the for-loop will 
		 * move the other.) */
		count += 1;

		dt_element* element = malloc (sizeof (element));
		if (element != NULL)
		  {
		    element->type = TYPE_COORDINATE;
		    element->data = coordinate;

		    /* Add the wrapped coordinate to the list. */
		    list = g_slist_append (list, element);
		  }

	      }
	    else
	      printf ("Could not allocate enough memory.\r\n");
	  }
	  break;
	case 100:
	  {
	    stats.pressure++;
	  }
	  break;
	case 101:
	  {
	    stats.tilt++;
	  }
	  break;
	case 128:
	  {
	    stats.objects++;
	  }
	  break;
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
	case 241:
	  {
	    stats.strokes++;
	    /* Move up one position. */
	    count++;

	    /* Make sure it's valid stroke information. */
	    if (data[count] == 3)
	      {
		dt_stroke* stroke = malloc (sizeof (dt_stroke));
		if (stroke != NULL) 
		  {
		    stroke->value = data[count + 1];

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
    }

  printf("\r\n");

  printf("\r\nSTATISTICS\r\n%-10s %-10s %-10s %-10s %-10s %-10s\r\n"
	 "%-10d %-10d %-10d %-10d %-10d %-10d\r\n\r\n", 
	 "Objects", "Strokes", "Pen XY", "Pressure", "Tilt", "Unknown",
	 stats.objects, stats.strokes, stats.coordinates, stats.pressure,
	 stats.tilt, stats.unknown);

  return list;
}
