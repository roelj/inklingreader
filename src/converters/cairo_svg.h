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

#ifndef CONVERTERS_CAIRO_SVG_H
#define CONVERTERS_CAIRO_SVG_H

#include <cairo.h>
#include <cairo-svg.h>
#include <glib.h>

int co_write_cairo_svg_file (const char* filename, GSList* data);
void co_display_cairo_surface (cairo_t* cr, GSList* data);

#endif//CONVERTERS_CAIRO_SVG_H
