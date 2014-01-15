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

  /* Write the data to the file. */
  cairo_surface_show_page (surface);

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
		  has_been_positioned = 0;
		  group++;
		}
	      break;
	      case END_STROKE:
		cairo_stroke (cr);
		break;
	      case NEW_LAYER:
		layer++;
		break;
	      }

	    free (s); s = NULL;
	  }
	  break;
	  /*------------------------------------------------------------------.
	   | COORDINATE DESCRIPTOR                                            |
	   '------------------------------------------------------------------*/
	case TYPE_COORDINATE:
	  {
	    dt_coordinate* c = (dt_coordinate *)e->data;

	    if (has_been_positioned == 0)
	      {
		cairo_line_to (cr, c->x, c->y);
		printf ("cairo_move_to (%f, %f)\r\n", c->x, c->y);
	      }
	    else
	      {
		has_been_positioned = 1;
		cairo_line_to (cr, c->x, c->y);
		printf ("cairo_line_to (%f, %f)\r\n", c->x, c->y);
	      }

	    free (c);
	    c = NULL;
	  }
	  break;
	case TYPE_PRESSURE:
	  {
	    dt_pressure* p = (dt_pressure *)e->data;
	    free (p);
	    p = NULL;
	  }
	  break;
	case TYPE_TILT:
	  {
	    dt_tilt* t = (dt_tilt *)e->data;
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

  /* Clean up. */
  cairo_surface_finish (surface);
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
		  has_been_positioned = 0;
		  group++;
		}
	      break;
	      case END_STROKE:
		cairo_stroke (cr);
		break;
	      case NEW_LAYER:
		layer++;
		break;
	      }

	    free (s); s = NULL;
	  }
	  break;
	  /*------------------------------------------------------------------.
	   | COORDINATE DESCRIPTOR                                            |
	   '------------------------------------------------------------------*/
	case TYPE_COORDINATE:
	  {
	    dt_coordinate* c = (dt_coordinate *)e->data;

	    if (has_been_positioned == 0)
	      cairo_line_to (cr, c->x, c->y);
	    else
	      {
		has_been_positioned = 1;
		cairo_line_to (cr, c->x, c->y);
	      }

	    free (c);
	    c = NULL;
	  }
	  break;
	case TYPE_PRESSURE:
	  {
	    dt_pressure* p = (dt_pressure *)e->data;
	    free (p);
	    p = NULL;
	  }
	  break;
	case TYPE_TILT:
	  {
	    dt_tilt* t = (dt_tilt *)e->data;
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
}
