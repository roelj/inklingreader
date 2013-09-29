#include "wpi.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
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
void
parse_wpi (const char* filename)
{
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

  printf("Analyzing ");

  /* Find out interesting places. */
  size_t count;
  for (count = 2060; count < data_len; count++)
    {
      /*
      if (count % (data_len / 10) == 0)
	{
	  printf(".");
	  fflush(stdout);
	}
      */
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
		//coordinate = { 0, 0 };

		/* 'count' should be moved up 2 bytes so it the X 
		 * position can be read. */
		count += 2;

		/* The bitshifting and stuff is taken from "PaperInkConverter". */
		coordinate->x = (data[count] << 8) + data[count + 1] + 5;

		/* Move over to the Y-coordinate. */
		count += 2;

		/* The bitshifting and stuff is taken from "PaperInkConverter". */
		coordinate->y = (((data[count] << 8) + data[count + 1]) << 1) + 5;

		printf("Coordinate (x = %d, y = %d)\r\n", 
		       coordinate->x, coordinate->y);

		/* Move past the coordinate data so we don't read 
		 * duplicate data. (Move only 1 because the for-loop will 
		 * move the other. */
		count += 1;

		/* Clean up the allocated memory because we're not doing 
		 * anything with it from now on. */
		free (coordinate);
		coordinate = NULL;
	      }
	  }
	  break;
	case 100:
	  {
	    stats.pressure++;
	    //printf("\r\nPRESSURE\r\n");
	  }
	  break;
	case 101:
	  {
	    stats.tilt++;
	    //printf("\r\nTILT\r\n");
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
	case 241:
	  {
	    stats.strokes++;
	    //printf ("Stroke on %zu\r\n", count);
	  
	    /* Move up one position. */
	    count++;

	    size_t block_size = data[count];
	    //printf ("Block size: %zu\r\n", block_size);

	    unsigned char block_data[block_size];
	    size_t a;
	    //printf ("Block data:\r\n");
	    for (a = 0; a < block_size; a++)
	      {
		block_data[a] = data[count + 1 + a];

		switch (block_data[a])
		  {
		  case 0:
		    //printf("\r\nEND STROKE\r\n");
		    break;
		  case 1:
		    //printf("\r\nBEGIN STROKE\r\n");
		    break;
		  case 128:
		    //printf("\r\nNEW LAYER\r\n");
		    break;
		  default:
		    //printf("%u ", block_data[a]);
		    break;
		  }
	      }
 	  }
	default:
	  //printf("%u ", data[count]);
	  break;
	}
    }

  printf("\r\n");

  printf("\r\nSTATISTICS\r\n%-10s %-10s %-10s %-10s %-10s %-10s\r\n"
	 "%-10d %-10d %-10d %-10d %-10d %-10d\r\n\r\n", 
	 "Objects", "Strokes", "Pen XY", "Pressure", "Tilt", "Unknown",
	 stats.objects, stats.strokes, stats.coordinates, stats.pressure,
	 stats.tilt, stats.unknown);
}
