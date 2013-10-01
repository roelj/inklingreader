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
    "<?xml version=\"1.0\" standalone=\"no\"?>\n" 
    "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"
    "  \"http://www.w3.org/Graphics/SVG/2.2/DTD/svg11.dtd\">\n"
    "<svg width=\"210mm\" height=\"297mm\" version=\"1.1\""
    "  xmlns=\"http://www.w3.org/2000/svg\">\n";

  fwrite (header, strlen(header), 1, file);

  /*--------------------------------------------------------------------------.
   | WRITE DATA POINTS                                                        |
   '--------------------------------------------------------------------------*/
  while (data != NULL)
    {
      dt_element* e = (dt_element *)data->data;
      switch (e->type)
	{
	case TYPE_STROKE:
	  {
	    dt_stroke* s = (dt_stroke *)e->data;
	    fprintf (file, "  <!-- Stroke information: %u -->\n", s->value);
	  }
	  break;
	case TYPE_COORDINATE:
	  {
	    dt_coordinate* c = (dt_coordinate *)e->data;
	    fprintf (file, 
		     "  <rect x=\"%dpt\" y=\"%dpt\" width=\"1cm\" "
		     "height=\"1cm\" />\n", c->x, c->y);
	  }
	  break;
	}


      free (data->data);
      data = data->next;
    }

  fwrite ("</svg>", 6, 1, file);
  fclose (file);

  return 0;
}
