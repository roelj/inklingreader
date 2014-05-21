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
 * @file   converters/json.h
 * @brief  A set of functions to convert parsed data to JSON.
 * @author Roel Janssen
 */

#ifndef CONVERTERS_JSON_H
#define CONVERTERS_JSON_H

#include <glib.h>

/**
 * This function converts parsed data to a JSON document.
 * @param filename The path of the file to write to.
 * @param data The parsed data (see p_wpi_parse()).
 * @return 0 when everything went fine, 1 when something went wrong.
 */
int co_json_create_file (const char* filename, GSList* data);

/**
 * This function creates JSON-formatted output from parsed data 
 * @param data  The parsed data (see p_wpi_parse()).
 * @param title The document's title or NULL for no title.
 * @return A dynamically allocated JSON-formatted string.
 */
char* co_json_create (GSList* data, const char* title);

#endif//CONVERTERS_JSON_H
