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

/**
 * @file   converters/pdf.h
 * @brief  A set of functions to convert parsed data to a PDF.
 * @author Roel Janssen
 */

#ifndef CONVERTERS_PDF_H
#define CONVERTERS_PDF_H

#include <glib.h>
#include <librsvg/rsvg.h>

/**
 * This function converts SVG data to a PDF document.
 * @param filename The path of the file to write to.
 * @param svg_data The parsed data (see co_svg_create()).
 * @return 0 when everything went fine, 1 when something went wrong.
 */
int co_pdf_export_to_file (const char* filename, const char* svg_data);


/**
 * This function converts an RsvgHandle to a PDF document.
 * @param filename The filename to export to.
 * @param handle   An existing RsvgHandle to use for exporting.
 * @return 0 when everything when fine, 1 when something went wrong.
 */
int co_pdf_export_to_file_from_handle (const char* filename, RsvgHandle* handle);

#endif//CONVERTERS_PDF_H
