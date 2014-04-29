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

#ifndef DATATYPES_CONFIGURATION_H
#define DATATYPES_CONFIGURATION_H

typedef struct
{
  double width;
  double height;
  char* measurement;
} dt_page_dimensions;

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

#endif//DATATYPES_CONFIGURATION_H
