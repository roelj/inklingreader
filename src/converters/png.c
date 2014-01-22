#include "png.h"
#include <librsvg/rsvg.h>
#include <cairo.h>
#include <string.h>

#define A4_WIDTH 595.0
#define A4_HEIGHT 842.0

/*----------------------------------------------------------------------------.
 | CO_PNG_EXPORT_TO_FILE                                                      |
 | This function handles the exporting to PNG using Cairo. Returns 0 when     |
 | everything goes fine, eturns 1 if something went wrong.                    |
 '----------------------------------------------------------------------------*/
int
co_png_export_to_file (const char* filename, const char* svg_data)
{
  RsvgHandle* handle = rsvg_handle_new_from_data ((unsigned char*)svg_data, 
						  strlen (svg_data), NULL);

  return co_png_export_to_file_from_handle (filename, handle);
}

/*----------------------------------------------------------------------------.
 | CO_PNG_EXPORT_TO_FILE_FROM_HANDLE                                          |
 | This function handles the exporting to PNG using Cairo. Returns 0 when     |
 | everything goes fine, eturns 1 if something went wrong.                    |
 '----------------------------------------------------------------------------*/
int
co_png_export_to_file_from_handle (const char* filename, RsvgHandle* handle)
{
  int status = 0;

  cairo_surface_t* surface = NULL;
  surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 
					A4_WIDTH * 1.25, 
					A4_HEIGHT * 1.25);

  cairo_t* cr = cairo_create (surface);
  rsvg_handle_render_cairo (handle, cr);
  status = cairo_surface_write_to_png (surface, filename);

  cairo_destroy (cr);
  cairo_surface_destroy (surface);

  return status;
}
