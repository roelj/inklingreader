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
 * @file   parsers/wpi.h
 * @brief  A parser to convert a WPI file to a better useable format.
 * @author Roel Janssen
 */

/**
 * @namespace parsers
 * In this namespace you can find data parsers.
 * 
 * @note The prefix for this namespace has been shortened to "p_".
 */

#ifndef PARSERS_WPI_H
#define PARSERS_WPI_H

#include <glib.h>

/**
 * This function decodes the WPI format and creates a list of the data using
 * the available datatypes.
 *
 * @param filename The filename to parse.
 * @return A pointer to a GSList containing the parsed data.
 */
GSList* p_wpi_parse (const char* filename, unsigned short* seconds);

/**
 * This function cleans up the data that was created using p_wpi_parse().
 *
 * @param data A pointer to a GSList created by p_wpi_parse().
 */
void p_wpi_cleanup (GSList* data);

#endif//PARSERS_WPI_H
