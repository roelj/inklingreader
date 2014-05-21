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

#include "svg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include "../datatypes/configuration.h"
#include "../datatypes/element.h"

/* These are correction values that are used to calculate the actual position
 * of a path on a page. Whether these are the same for each document is not
 * certain yet, but it seems to work for several of my documents. */
#define SHRINK 27.0
#define OFFSET_X 375.0
#define OFFSET_Y 37.5
#define PRESSURE_FACTOR 2000.0
#define SPIKE_THRESHOLD 25.0
#define MM_TO_PT 3.5433

#define DEFAULT_COLOR "#00007c"

/*----------------------------------------------------------------------------.
 | CO_WRITE_SVG_FILE                                                          |
 | This function writes data points to an SVG file.                           |
 '----------------------------------------------------------------------------*/
int
co_svg_create_file (const char* filename, GSList* data, dt_configuration* settings)
{
  int return_val = 0;
  char* output = co_svg_create (data, filename, settings);
  if (data == NULL) return 1;

  FILE* file;
  file = fopen (filename, "w");
  if (file != NULL)
    fwrite (output, strlen (output), 1, file);
  else
    return_val = 1;

  free (output);
  fclose (file);

  return return_val;
}

char* 
co_svg_create (GSList* data, const char* title, dt_configuration* settings)
{
  /* Floating-point numbers should be written with a dot instead of a comma.
   * To ensure that this happens, (temporarily) set the locale to the "C"
   * locale for this program. */
  setlocale (LC_NUMERIC, "C");

  if (g_slist_length (data) == 0)
    {
      puts ("co_svg_create: No useful data was found in the file.\r\n");
      return NULL;
    }

  int written = 0;

  /* On average, 10 bytes are written per list item. The header and background 
   * layer take 571 bytes, so these are added to the amount to allocate. With 
   * small files the averages may not be work out well, so over-allocate a bit.
   * There's no mechanism in place to allocate more. So this is something 
   * to look into. */
  size_t output_len = 20 * g_slist_length (data) + 1000;
  char* output = calloc (1, output_len);
  if (output == NULL)
    {
      puts ("co_svg_create: Couldn't allocate enough memory.\r\n");
      return NULL;
    }

  /*--------------------------------------------------------------------------.
   | WRITE SVG HEADER                                                         |
   '--------------------------------------------------------------------------*/

  /* Make sure we have valid dimensions. */
  if (settings->page.width == 0) settings->page.width = 210;
  if (settings->page.height == 0) settings->page.height = 297;
  if (settings->page.measurement == NULL) 
    {
      settings->page.measurement = calloc (1, 3);
      settings->page.measurement = strncpy (settings->page.measurement, "mm", 2);
    }

  written += sprintf (output + written,
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n" 
    "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"
    "  \"http://www.w3.org/Graphics/SVG/2.2/DTD/svg11.dtd\">\n"
    "<svg width=\"%f%s\" height=\"%f%s\" version=\"1.1\" viewBox=\"0 0 %f %f\""
    "  xmlns=\"http://www.w3.org/2000/svg\"\n"
    "  xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\">\n",
    settings->page.width, settings->page.measurement, settings->page.height, 
    settings->page.measurement, settings->page.width * MM_TO_PT, 
    settings->page.height * MM_TO_PT);

  if (title != NULL)
    written += sprintf (output + written, "<title>%s</title>\n", title);

  /* If no background color was set, use white. */
  if (settings->background == NULL) 
    {
      settings->background = calloc (1, 8);
      settings->background = strncpy (settings->background, "#ffffff", 7);
    }

  /* Create a background layer for Inkscape when the background color hasn't 
   * been set to "none". */
  if (strcmp (settings->background, "none"))
    written += sprintf (output + written,
			"<g inkscape:label=\"Background\" inkscape:groupmode=\"layer\" id=\"layer0\">"
			"<rect style=\"fill:%s;stroke:none\" id=\"background\" "
			"width=\"%f%s\" height=\"%f%s\" x=\"0\" y=\"0\" /></g>\n"
			"<g inkscape:label=\"Layer 1\" inkscape:groupmode=\"layer\" "
			"id=\"layer1\">\n",
			settings->background, settings->page.width, settings->page.measurement, 
			settings->page.height, settings->page.measurement);
  else
    written += sprintf (output + written,
			"<g inkscape:label=\"Layer 1\" inkscape:groupmode=\"layer\" "
			"id=\"layer1\">\n");


  /*--------------------------------------------------------------------------.
   | COUNTING VARIABLES                                                       |
   '--------------------------------------------------------------------------*/
  unsigned int group = 0;
  unsigned int layer = 2;
  unsigned int layer_color = 1;
  unsigned char has_stroke_data = 0;
  unsigned char has_been_positioned = 0;
  unsigned char is_in_stroke = 0;
  float previous_x = 0;
  float previous_y = 0;

  GSList* stroke_data = NULL; 

  /*--------------------------------------------------------------------------.
   | WRITE DATA POINTS                                                        |
   '--------------------------------------------------------------------------*/
  GSList* data_head = data;
  while (data != NULL)
    {
      dt_element* e = (dt_element *)data->data;
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
		{
		  char* color = DEFAULT_COLOR;
		  if (layer_color <= settings->num_colors)
		    color = settings->colors[layer_color - 1];
		  else if (settings->num_colors > 0)
		    color = settings->colors[0];
                  if (settings->pressure_factor != 0)
                    written += sprintf (output + written, "  <g id=\"group%d\">\n    <path "
                                        "style=\"fill:%s; stroke:none\" d=\"", group, color);
                  else
                    written += sprintf (output + written, "  <g id=\"group%d\">\n    <path "
                                        "style=\"fill:none; stroke:%s\" d=\"", group, color);

		  has_been_positioned = 0;
		  is_in_stroke = 1;
		  has_stroke_data = 1;
		  group++;
		}
	      break;
	      case END_STROKE:
		{
		  /* Skip when we're not in a stroke. */
		  if (is_in_stroke != 1) break;

		  if (settings->pressure_factor == 0)
		    {
		      /* end SVG path. */
		      written += sprintf (output + written, "\" />\n  </g>\n");
		      is_in_stroke = 0;
		      break;
		    }

		  /* Go through all the points in reversed order and add the other edge of the stroke. */
		  GSList* stroke_data_head = stroke_data;
		  while (stroke_data != NULL)
		    {
		      dt_coordinate* c = (dt_coordinate*)stroke_data->data;
		      float x = c->x / SHRINK + OFFSET_X;
		      float y = c->y / SHRINK + OFFSET_Y;

		      /* When points are too far away, skip them.
		       * When points are exactly the same, skip them.
		       * This prevents weird stripes and clutter from 
		       * disturbing the document. */
		      float distance = sqrt ((x - previous_x) * (x - previous_x) +
					     (y - previous_y) * (y - previous_y));
		      if ( distance <= SPIKE_THRESHOLD &&
			   x != previous_x && y != previous_y)
			{
			  /* Avoid division by zero. If distance is zero, delta_x and
			   * delta_y are also zero. */
			  if (distance == 0) distance = 1;

			  written += sprintf (output + written, " L %f,%f",
					      x + (previous_y - y) / distance * c->pressure * settings->pressure_factor,
					      y + (x - previous_x) / distance * c->pressure * settings->pressure_factor);
			  previous_x = x;
			  previous_y = y;
			}

		      stroke_data = stroke_data->next;
		    }

		  g_slist_free (stroke_data_head), stroke_data = NULL;

		  /* 'Z' means 'closepath' */
		  written += sprintf (output + written, " z\" />\n  </g>\n");
		  is_in_stroke = 0;
		}
		break;
	      case NEW_LAYER:
		{
		  if (has_stroke_data == 0)
		    layer_color++;
		  else
		    {
		      has_stroke_data = 0;
		      layer_color = 1;
		      written += sprintf (output + written, 
		        "\n  </g>\n<g inkscape:label=\"Layer %d\" inkscape:"
			"groupmode=\"layer\" id=\"layer%d\">\n", 
			layer, layer);
		      layer++;
		    }
		}
		break;
	      }
	  }
	  break;
	  /*------------------------------------------------------------------.
	   | COORDINATE DESCRIPTOR                                            |
	   '------------------------------------------------------------------*/
	case TYPE_COORDINATE:
	  {
	    if (is_in_stroke != 1) break;

	    dt_coordinate* c = (dt_coordinate *)e;

	    /* There's no point in going on when the coordinate is empty. */
	    if (c == NULL) break;

	    if (data->next != NULL)
	      {
		dt_element* elem = (dt_element*)data->next->data;
		if (elem != NULL && elem->type == TYPE_PRESSURE)
		  {
		    dt_pressure* p = (dt_pressure*)elem;
		    c->pressure = p->pressure;
		    c->pressure = c->pressure / PRESSURE_FACTOR;
		  }
	      }

	    float x = c->x / SHRINK + OFFSET_X;
	    float y = c->y / SHRINK + OFFSET_Y;
		    
	    char* type = "M";

	    if (has_been_positioned > 0)
	      {
		// When points are exactly the same, skip them.
		if (x == previous_x && y == previous_y)
		  break;
		type = " L";
	      }
	    else
	      {  //begin a new stroke
		previous_x = x;//
		previous_y = y;//
		has_been_positioned = 1;
	      }


	    float distance = sqrt ((x - previous_x) * (x - previous_x) +
				   (y - previous_y) * (y - previous_y));
	    // Avoid division by zero. If distance is zero, delta_x and
	    // delta_y are also zero.
	    if (distance == 0)
	      distance = 1;
	    if (distance > SPIKE_THRESHOLD)
	      break;

	    written += sprintf (output + written, "%s %f,%f", 
				type,
				x + (previous_y - y) / distance * c->pressure * settings->pressure_factor,
				y + (x - previous_x) / distance * c->pressure * settings->pressure_factor);
	    previous_x = x;
	    previous_y = y;

	    stroke_data = g_slist_prepend (stroke_data, c);
	  }
	  break;
	  /*
	case TYPE_TILT:
	  {
	    //dt_tilt* t = (dt_tilt *)e->data;
	  }
	  break;
	  */
	}

      data = data->next;
    }

  data = data_head;

  written += sprintf (output + written, "</g>\n</svg>");

  output_len = written + 1;
  output = realloc (output, output_len);
  output[written] = '\0';

  /* Reset to default locale settings. */
  setlocale (LC_NUMERIC, "");

  return output;
}
