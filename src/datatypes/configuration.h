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
 * @file   datatypes/configuration.h
 * @brief  User-defined configuration options are packed into a single datatype
 *         so it can be passed along using a single parameter.
 * @author Roel Janssen
 * @namespace datatypes
 */

#ifndef DATATYPES_CONFIGURATION_H
#define DATATYPES_CONFIGURATION_H

/**
 * This struct is used to describe the page dimensions.
 */
typedef struct
{
  double width;
  double height;
  char* measurement;
  char* orientation;
} dt_page_dimensions;

/**
 * This struct contains all configuration options that a user can configure on
 * run-time.
 */
typedef struct
{
  unsigned char type;
  int num_colors;
  double pressure_factor;
  char** colors;
  char* background;
  dt_page_dimensions page;
  char* config_location;
} dt_configuration;

/**
 * This struct is used to describe preset page dimensions.
 */
typedef struct {
  char* name;
  double width;
  double height;
  char* measurement;
} dt_preset_dimensions;

/**
 * This function parses colors from a string.
 * @param data   A string to parse.
 * @param config A dt_configuration structure to store the parsed data to.
 */
void dt_configuration_parse_colors (const char* data, dt_configuration* config);

/**
 * This function properly cleans up allocated memory of a dt_configuration.
 * @param config A dt_configuration to clean up.
 */
void dt_configuration_cleanup (dt_configuration* config);

/**
 * This function sets configuration options from a config file.
 * @param filename The filename of the configuration file to parse.
 * @param config   A dt_configuration structure to store the parsed data to.
 */
void dt_configuration_parse (const char* filename, dt_configuration* config);

/**
 * This function parses dimensions from a string.
 * @param data   A string to parse.
 * @param config A dt_configuration structure to store the parsed data to.
 */
void dt_configuration_parse_dimensions (const char* data, dt_configuration* config);

/**
 * This function parses a preset dimension and sets the right page dimensions.
 * @param data   A string to parse.
 * @param config A dt_configuration structure to store the parsed data to.
 */
void dt_configuration_parse_preset_dimensions (const char* data, dt_configuration* config);

#endif//DATATYPES_CONFIGURATION_H
