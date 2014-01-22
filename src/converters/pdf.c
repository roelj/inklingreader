#include "pdf.h"
#include <librsvg/rsvg.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <string.h>

#define A4_WIDTH 595.0
#define A4_HEIGHT 842.0

/*----------------------------------------------------------------------------.
 | CO_PDF_EXPORT_TO_FILE                                                      |
 | This function handles the exporting to PDF using Cairo. Returns 0 when     |
 | everything goes fine, eturns 1 if something went wrong.                    |
 '----------------------------------------------------------------------------*/
int
co_pdf_export_to_file (const char* filename, const char* svg_data)
{
  RsvgHandle* handle = rsvg_handle_new_from_data ((unsigned char*)svg_data, 
						  strlen (svg_data), NULL);

  return co_pdf_export_to_file_from_handle (filename, handle);
}

/*----------------------------------------------------------------------------.
 | CO_PDF_EXPORT_TO_FILE_FROM_HANDLE                                          |
 | This function handles the exporting to PDF using Cairo. Returns 0 when     |
 | everything goes fine, eturns 1 if something went wrong.                    |
 '----------------------------------------------------------------------------*/
int
co_pdf_export_to_file_from_handle (const char* filename, RsvgHandle* handle)
{
  int status = 0;

  cairo_surface_t* surface = NULL;
  surface = cairo_pdf_surface_create (filename, A4_WIDTH, A4_HEIGHT);

  cairo_t* cr = cairo_create (surface);
  rsvg_handle_render_cairo (handle, cr);
  cairo_surface_show_page (surface);
  cairo_surface_finish (surface);

  cairo_destroy (cr);
  cairo_surface_destroy (surface);

  return status;
}
