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
 * @file   converters/svg.h
 * @brief  A set of functions to convert parsed data to an SVG.
 * @author Roel Janssen
 */

#ifndef CONVERTERS_SVG_H
#define CONVERTERS_SVG_H

#include <glib.h>
#include "../datatypes/configuration.h"

/**
 * This function converts parsed data to an SVG file.
 * @param filename The path of the file to write to.
 * @param data     The parsed data (see p_wpi_parse()).
 * @param settings User-defined settings that affect the output.
 * @return 0 when everything went fine, 1 when something went wrong.
 */
int co_svg_create_file (const char* filename, GSList* data, dt_configuration* settings);

/**
 * This function converts parsed data to a string.
 * @param data     The parsed data (see p_wpi_parse()).
 * @param title    The title of the document or NULL for no title.
 * @param settings User-defined settings that affect the output.
 * @return A string containing SVG data.
 */
char* co_svg_create (GSList* data, const char* title, dt_configuration* settings);

#endif//CONVERTERS_SVG_H
