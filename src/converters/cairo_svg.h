#ifndef CONVERTERS_CAIRO_SVG_H
#define CONVERTERS_CAIRO_SVG_H

#include <cairo.h>
#include <cairo-svg.h>
#include <glib.h>

int co_write_cairo_svg_file (const char* filename, GSList* data);
void co_display_cairo_surface (cairo_t* cr, GSList* data);

#endif//CONVERTERS_CAIRO_SVG_H
