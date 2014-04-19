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

#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include "../datatypes/configuration.h"
#include "../datatypes/element.h"
#include "../optimizers/straight_lines.h"

extern dt_configuration settings;

/* These are correction values that are used to calculate the actual position
 * of a path on a page. Whether these are the same for each document is not
 * certain yet, but it seems to work for several of my documents. */
#define SHRINK 27.0
#define OFFSET_X 375.0
#define OFFSET_Y 37.5
#define SPIKE_THRESHOLD 20.0

#define DEFAULT_COLOR "#00007c"

static void
skip_warning ()
{
  printf ("WARNING: An element has been skipped because of malformed input.\r\n");
}


/*----------------------------------------------------------------------------.
 | CO_WRITE_JSON_FILE                                                          |
 | This function writes data points to an JSON file.                           |
 '----------------------------------------------------------------------------*/
int
co_json_create_file (const char* filename, GSList* data)
{
  char* output = co_json_create (data, filename);
  if (data != NULL)
    {
      FILE* file;
      file = fopen (filename, "w");
      if (file != NULL)
	fwrite (output, strlen (output), 1, file);
      else
	{
	  printf ("%s: Couldn't write to '%s'.\r\n", __func__, filename);
	  return 1;
	}

      free (output);
      fclose (file);
      return 0;
    }

  return 1;
}

char*
co_json_create (GSList* data, const char* title)
{
  /* Floating-point numbers should be written with a dot instead of a comma.
   * To ensure that this happens, (temporarily) set the locale to the "C"
   * locale for this program. */
  setlocale (LC_NUMERIC, "C");

  if (g_slist_length (data) == 0)
    {
      printf ("%s: No useful data was found in the file.\r\n", __func__);
      return NULL;
    }

  int written = 0;

  /* On average, 100 bytes are written per list item. The header and background 
   * layer take 571 bytes, so these are added to the amount to allocate. 
   * There's no mechanism in place to allocate more. So this is something 
   * to look into. */
  size_t output_len = 100 * g_slist_length (data) + 50;
  char* output = malloc (output_len);
  if (output == NULL)
    {
      printf ("%s: Couldn't allocate enough memory.\r\n", __func__);
      return NULL;
    }

  /*--------------------------------------------------------------------------.
   | WRITE JSON HEADER                                                         |
   '--------------------------------------------------------------------------*/
  written += sprintf (output + written, "{\r\n  \"0\" : {\r\n");

  /*--------------------------------------------------------------------------.
   | COUNTING VARIABLES                                                       |
   '--------------------------------------------------------------------------*/
  unsigned int point = 0;
  unsigned int group = 0;
  unsigned int layer = 1;
  unsigned int layer_color = 1;
  unsigned char has_stroke_data = 0;
  unsigned char is_in_stroke = 0;
  dt_coordinate prev = { TYPE_COORDINATE, 0, 0 };

  GSList* stroke_data = NULL; 

  /*--------------------------------------------------------------------------.
   | WRITE DATA POINTS                                                        |
   '--------------------------------------------------------------------------*/
  while (data != NULL)
    {
      dt_element* e = (dt_element *)data->data;
      dt_element* next = NULL;

      /* Provide an easy way to access the next element. */
      if (data->next != NULL) next = (dt_element *)data->next->data;

      switch (e->type)
	{
	  /*------------------------------------------------------------------.
	   | STROKE DESCRIPTOR                                                |
	   '------------------------------------------------------------------*/
	case TYPE_STROKE:
	  {
	    dt_stroke* s = (dt_stroke *)e;
	    switch (s->value)
	      {
	      case BEGIN_STROKE:
		written += sprintf (output + written, "    \"%d\" : {\r\n", group),
		  is_in_stroke = 1, has_stroke_data = 1, group++;
	      break;
	      case END_STROKE:
		{
		  if (is_in_stroke != 1) { skip_warning (); break; }

		  while (stroke_data != NULL)
		    {
		      dt_coordinate* c = (dt_coordinate*)stroke_data->data;
		      float x = c->x / SHRINK + OFFSET_X;
		      float y = c->y / SHRINK + OFFSET_Y;

		      /* When points are too far away, skip them.
		       * When points are exactly the same, skip them.
		       * When the data is within the borders of an A4 page, add
		       * it. This prevents weird stripes and clutter from 
		       * disturbing the document. */
		      float distance = sqrt ((x - prev.x) * (x - prev.x) +
					     (y - prev.y) * (y - prev.y));

		      if (distance <= SPIKE_THRESHOLD)
			{
			  /* Avoid division by zero. If distance is zero, delta_x and
			   * delta_y are also zero. */
			  if (distance == 0) distance = 1;

			  written += sprintf (output + written, 
					      "      \"%d\" : {\r\n"
					      "        \"x\" : %f,\r\n"
					      "        \"y\" : %f,\r\n"
					      "        \"pressure\" : %f",
					      point,
					      x + (prev.y - y) / distance * c->pressure,
					      y + (x - prev.x) / distance * c->pressure,
					      c->pressure);

			  if (stroke_data->next == NULL)
			    written += sprintf (output + written, "\r\n       }\r\n");
			  else
			    written += sprintf (output + written, "\r\n       },\r\n");

			  prev.x = x, prev.y = y, point++;
			}

		      free (c);
		      stroke_data = stroke_data->next;
		    }

		  if (data->next == NULL)
		    written += sprintf (output + written, "\r\n    }\r\n");
		  else
		    written += sprintf (output + written, "\r\n    },\r\n");

		  is_in_stroke = 0;
		}
		break;
	      case NEW_LAYER:
		{
		  if (has_stroke_data == 0) layer_color++;
		  else
		    has_stroke_data = 0, layer_color = 1, layer++,
		      written += sprintf (output + written, "  }\r\n  \"%d\" : {\r\n", layer);
		}
		break;
	      }

	    free (s), s = NULL;
	  }
	  break;
	  /*------------------------------------------------------------------.
	   | COORDINATE DESCRIPTOR                                            |
	   '------------------------------------------------------------------*/
	case TYPE_COORDINATE:
	  {
	    if (is_in_stroke != 1) { skip_warning (); break; }

	    dt_coordinate* c = (dt_coordinate *)e;
	    if (next->type == TYPE_PRESSURE)
	      {
		if (settings.pressure_factor != 0)
		  c->pressure = ((dt_pressure*)next)->pressure / settings.pressure_factor;

		if (data->next != NULL)
		  next = (dt_element *)data->next->next->data;
	      }

	    float x = c->x / SHRINK + OFFSET_X;
	    float y = c->y / SHRINK + OFFSET_Y;

	    /* When points are exactly the same, skip them. Otherwise begin a new stroke. */
	    if (x == prev.x && y == prev.y) break;
	    else
	      prev.x = x, prev.y = y;

	    // When the data is within the borders of an A4 page, add
	    // it. This prevents weird stripes and clutter from 
	    // disturbing the document.
	    if (x < 1 || y < 101 || y > 1049) { skip_warning (); break; }

	    if (settings.pressure_factor != 0)
	      stroke_data = g_slist_prepend (stroke_data, c);

	    float distance = sqrt ((x - prev.x) * (x - prev.x) +
				   (y - prev.y) * (y - prev.y));
	    // Avoid division by zero. If distance is zero, delta_x and
	    // delta_y are also zero.
	    if (distance == 0) distance = 1;
	    else if (distance > SPIKE_THRESHOLD) break;

	    written += sprintf (output + written, 
				"       \"%d\" : {\r\n"
				"         \"x\" : %f,\r\n"
				"         \"y\" : %f,\r\n"
				"         \"pressure\" : %f",
				point,
				x + (prev.y - y) / distance * c->pressure,
				y + (x - prev.x) / distance * c->pressure,
				c->pressure);

	    if (next->type == TYPE_TILT)
	      {
		dt_tilt* tilt = (dt_tilt*) next;
		written += sprintf (output + written, 
				    ",\r\n         \"tilt\" : [ %d, %d ]", 
				    tilt->x, tilt->y);
	      }

	    if (data->next == NULL)
	      written += sprintf (output + written, "\r\n       }\r\n");
	    else
	      written += sprintf (output + written, "\r\n       },\r\n");

	    point++, prev.x = x, prev.y = y;
	  }
	  break;
	}

      data = data->next;
    }

  written += sprintf (output + written, "  }\r\n}\r\n");

  output_len = written + 1;
  output = realloc (output, output_len);
  output[written] = '\0';

  g_slist_free (data);
  data = NULL;

  /* Reset to default locale settings. */
  setlocale (LC_NUMERIC, "");

  return output;
}
