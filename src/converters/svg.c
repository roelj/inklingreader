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
#include "../datatypes/element.h"
#include "../optimizers/straight_lines.h"

/* These are correction values that are used to calculate the actual position
 * of a path on a page. Whether these are the same for each document is not
 * certain yet, but it seems to work for several of my documents. */
#define SHRINK 27.0
#define OFFSET_X 375.0
#define OFFSET_Y 37.5

/*----------------------------------------------------------------------------.
 | CO_WRITE_SVG_FILE                                                          |
 | This function writes data points to an SVG file.                           |
 '----------------------------------------------------------------------------*/
int
co_svg_create_file (const char* filename, GSList* data)
{
  char* output = co_svg_create (data, filename);
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
co_svg_create (GSList* data, const char* title)
{
  if (g_slist_length (data) == 0)
    {
      printf ("%s: No useful data was found in the file.\r\n", __func__);
      return NULL;
    }

  int written = 0;

  /* On average, 10 bytes are written per list item. The header and background 
   * layer take 571 bytes, so these are added to the amount to allocate. 
   * There's no mechanism in place to allocate more. So this is something 
   * to look into. */
  size_t output_len = 10 * g_slist_length (data) + 571;
  char* output = malloc (output_len);
  if (output == NULL)
    {
      printf ("%s: Couldn't allocate enough memory.\r\n", __func__);
      return NULL;
    }

  //else
  //  printf ("Allocated      %lu bytes.\r\n", output_len);

  /*--------------------------------------------------------------------------.
   | WRITE SVG HEADER                                                         |
   '--------------------------------------------------------------------------*/
  const char* header =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n" 
    "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"
    "  \"http://www.w3.org/Graphics/SVG/2.2/DTD/svg11.dtd\">\n"
    "<svg width=\"210mm\" height=\"297mm\" version=\"1.1\""
    "  xmlns=\"http://www.w3.org/2000/svg\"\n"
    "  xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\">\n";

  written += sprintf (output + written, header);
  //fwrite (header, strlen (header), 1, file);

  /* Write a document title and a create a layer for Inkscape. */
  written += sprintf (output + written, "<title>%s</title>\n"
		      "<g inkscape:label=\"Background\" inkscape:groupmode=\"layer\" id=\"layer0\">"
		      "<rect style=\"fill:#ffffff;stroke:none\" id=\"background\" "
		      "width=\"210mm\" height=\"297mm\" x=\"0\" y=\"0\" /></g>\n"
		      "<g inkscape:label=\"Layer 1\" inkscape:groupmode=\"layer\" "
		      "id=\"layer1\">\n", title);

  /*--------------------------------------------------------------------------.
   | COUNTING VARIABLES                                                       |
   '--------------------------------------------------------------------------*/
  unsigned int group = 0;
  unsigned int layer = 2;
  unsigned char has_been_positioned = 0;
  unsigned char is_in_stroke = 0;

  /*--------------------------------------------------------------------------.
   | WRITE DATA POINTS                                                        |
   '--------------------------------------------------------------------------*/
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
	    dt_stroke* s = (dt_stroke *)e->data;
	    switch (s->value)
	      {
	      case BEGIN_STROKE:
		{
		  written += sprintf (output + written, "  <g id=\"group%d\" style=\"fill: none; "
			   "stroke: #000000; stroke-width: 1\">\n    <path "
			   "fill=\"none\" stroke=\"#000000\" d=\"", group);

		  has_been_positioned = 0;
		  is_in_stroke = 1;
		  group++;
		}
	      break;
	      case END_STROKE:
		{
		  if (is_in_stroke == 1)
		    {
		      written += sprintf (output + written, "\" />\n  </g>\n");
		      is_in_stroke = 0;
		    }
		}
		break;
	      case NEW_LAYER:
		{
		  written += sprintf (output + written, 
			   "\n  </g>\n<g inkscape:label=\"Layer %d\" inkscape:"
			   "groupmode=\"layer\" id=\"layer%d\">\n", 
			   layer, layer);
		  layer++;
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
	    if (is_in_stroke == 1)
	      {
		dt_coordinate* c = (dt_coordinate *)e->data;
		//c = opt_straight_lines_filter (data);
		if (c != NULL)
		  {
		    char* type = "M";

		    if (has_been_positioned > 0)
		      type = " L";
		    else
		      has_been_positioned = 1;

		    float x = c->x / SHRINK + OFFSET_X;
		    float y = c->y / SHRINK + OFFSET_Y;

		    if (x > 0 && y > 0)
		      written += sprintf (output + written, "%s %f,%f", type, x, y);
		  }
	      }
	  }
	  break;
	case TYPE_PRESSURE:
	  {
	    //dt_pressure* p = (dt_pressure *)e->data;
	    //written += sprintf (output + written, "    <!-- Pressure: %d -->\n", p->pressure);
	  }
	  break;
	case TYPE_TILT:
	  {
	    //dt_tilt* t = (dt_tilt *)e->data;
	    //written += sprintf (output + written, " L%u,%u", t->x, t->y);
	  }
	  break;
	}

      free (e->data);
      free (e);
      e = NULL;
      data = data->next;
    }

  written += sprintf (output + written, "</g>\n</svg>");

  output_len = written + 1;
  output = realloc (output, output_len);
  output[written] = '\0';

  return output;
}
