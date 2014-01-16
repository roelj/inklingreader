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


#include "cairo_svg.h"
#include "../datatypes/element.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define A4_WIDTH 595
#define A4_HEIGHT 842

int
co_write_cairo_svg_file (const char* filename, GSList* data)
{
  /*--------------------------------------------------------------------------.
   | SET UP CAIRO                                                             |
   '--------------------------------------------------------------------------*/
  cairo_surface_t* surface;
  cairo_t* cr;

  /* Create an SVG surface. */
  surface = cairo_svg_surface_create (filename, A4_WIDTH, A4_HEIGHT);  
  cr = cairo_create (surface);

  /* Make sure the data will be written to a file. */
  cairo_surface_show_page (surface);

  co_display_cairo_surface (cr, data);

  /* Write to the file. */
  cairo_surface_finish (surface);

  /* Clean up. */
  cairo_destroy (cr);
  cairo_surface_destroy (surface);

  return 0;
}

void 
co_display_cairo_surface (cairo_t* cr, GSList* data)
{
  /*--------------------------------------------------------------------------.
   | COUNTING VARIABLES                                                       |
   '--------------------------------------------------------------------------*/
  unsigned int group = 0;
  unsigned int layer = 1;
  //unsigned char has_been_positioned = 0;

  GSList* data_head = data;

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
		group++;
	      break;
	      case END_STROKE:
		cairo_stroke (cr);
		break;
	      case NEW_LAYER:
		layer++;
		break;
	      }

	    //free (s); s = NULL;
	  }
	  break;
	  /*------------------------------------------------------------------.
	   | COORDINATE DESCRIPTOR                                            |
	   '------------------------------------------------------------------*/
	case TYPE_COORDINATE:
	  {
	    dt_coordinate* c = (dt_coordinate *)e->data;
	    cairo_line_to (cr, c->x, c->y);
	    //free (c); c = NULL;
	  }
	  break;
	case TYPE_PRESSURE:
	  {
	    //dt_pressure* p = (dt_pressure *)e->data;
	    //free (p); p = NULL;
	  }
	  break;
	case TYPE_TILT:
	  {
	    //dt_tilt* t = (dt_tilt *)e->data;
	    //free (t); t = NULL;
	  }
	  break;
	default:
	  {
	    //free (e->data);
	    //e->data = NULL;
	  }
	  break;
	}

      //free (data->data);
      data = data->next;
    }

  data = data_head;
}
