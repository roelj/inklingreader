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

/*----------------------------------------------------------------------------.
 | CO_WRITE_SVG_FILE                                                          |
 | This function writes data points to an SVG file.                           |
 '----------------------------------------------------------------------------*/
int
co_write_svg_file (const char* filename, GSList* data)
{
  FILE* file;
  file = fopen (filename, "w");

  /*--------------------------------------------------------------------------.
   | WRITE SVG HEADER                                                         |
   '--------------------------------------------------------------------------*/
  char* header =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n" 
    "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"
    "  \"http://www.w3.org/Graphics/SVG/2.2/DTD/svg11.dtd\">\n"
    "<svg width=\"210mm\" height=\"297mm\" version=\"1.1\""
    "  xmlns=\"http://www.w3.org/2000/svg\"\n"
    "  xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\">\n";

  fwrite (header, strlen(header), 1, file);

  /* Write a document title and a create a layer for Inkscape. */
  fprintf (file, "<title>%s</title>\n"
	   "<g inkscape:label=\"Layer 0\" inkscape:groupmode=\"layer\" "
	   "id=\"layer0\">\n", filename);

  /*--------------------------------------------------------------------------.
   | COUNTING VARIABLES                                                       |
   '--------------------------------------------------------------------------*/
  unsigned int group = 0;
  unsigned int layer = 1;
  unsigned char has_been_positioned = 0;

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
		  fprintf (file, "  <g id=\"group%d\" fill=\"none\" stroke=\"#000000\" "
			   "stroke-width=\"2\"><path fill=\"none\" stroke=\"#000000\" d=\"", group);
		  has_been_positioned = 0;
		  group++;
		}
	      break;
	      case END_STROKE:
		{
		  fprintf (file, "\" /></g>\n");
		}
		break;
	      case NEW_LAYER:
		{
		  fprintf (file, 
			   "</g>\n<g inkscape:label=\"Layer %d\" inkscape:"
			   "groupmode=\"layer\" id=\"layer%d\">\n", 
			   layer, layer);
		  layer++;
		}
		break;
	      }

	    free (s);
	    s = NULL;
	  }
	  break;
	  /*------------------------------------------------------------------.
	   | COORDINATE DESCRIPTOR                                            |
	   '------------------------------------------------------------------*/
	case TYPE_COORDINATE:
	  {
	    dt_coordinate* c = (dt_coordinate *)e->data;
	    char type = 'M';

	    if (has_been_positioned > 0)
	      {
		type = 'L';
	      }
	    else
	      has_been_positioned = 1;

	    fprintf (file, " %c %f %f", type, c->x / 10.0, c->y / 10.0);

	    free (c);
	    c = NULL;
	  }
	  break;
	case TYPE_PRESSURE:
	  {
	    dt_pressure* p = (dt_pressure *)e->data;
	    //fprintf (file, "    <!-- Pressure: %d -->\n", p->pressure);

	    free (p);
	    p = NULL;
	  }
	  break;
	case TYPE_TILT:
	  {
	    dt_tilt* t = (dt_tilt *)e->data;
	    //fprintf (file, " L%u,%u", t->x, t->y);

	    free (t);
	    t = NULL;
	  }
	  break;
	default:
	  {
	    free (e->data);
	    e->data = NULL;
	  }
	  break;
	}

      free (data->data);
      data = data->next;
    }

  fprintf (file, "</g>\n</svg>");
  fclose (file);

  return 0;
}
