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
 * @file   high/conversion.h
 * @brief  High-level functions for converting files.
 * @author Roel Janssen
 */

/**
 * @namespace high
 * In this namespace you can find functions that combine several other 
 * functions.
 * 
 * @note The prefix for this namespace is "high_".
 */

#ifndef HIGH_CONVERSION_H
#define HIGH_CONVERSION_H

#include <glib.h>
#include "../datatypes/configuration.h"

/**
 * This function handles exporting a file. It looks at the file extension to figure out
 * how to export. When 'svg_data' is not NULL, it will be used to export, otherwise 
 * 'data' will be used.
 *
 * @param data      Data parsed with p_wpi_parse().
 * @param svg_data  A string of SVG data. This has preference over 'data'.
 * @param to        The filename to export to.
 * @param settings  Pass along the user's custom settings.
 */
void high_export_to_file (GSList* data, const char* svg_data, const char* to, dt_configuration* settings);

/**
 * This function exports all non-hidden files in a directory to SVGs.
 * @param path      The directory with WPI files to convert.
 * @param settings  Pass along the user's custom settings.
 */
void high_convert_directory (const char* path, dt_configuration* settings);

/**
 * This function merges two WPI files. It puts each layer in a seperate layer.
 * @param first  The document to merge to.
 * @param second The document to add to the first.
 */
void high_merge_wpi_files (const char* first, const char* second);

#endif//DATATYPES_CONVERSION_H
