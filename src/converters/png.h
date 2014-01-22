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


#ifndef CONVERTERS_PNG_H
#define CONVERTERS_PNG_H

#include <glib.h>
#include <librsvg/rsvg.h>

int co_png_export_to_file (const char* filename, const char* svg_data);
int co_png_export_to_file_from_handle (const char* filename, RsvgHandle* handle);

#endif//CONVERTERS_PNG_H
