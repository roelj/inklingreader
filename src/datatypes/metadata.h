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
 * @file   datatypes/metadata.h
 * @brief  A struct to store metadata about a file.
 * @author Roel Janssen
 * @namespace datatypes
 */

#ifndef DATATYPES_METADATA_H
#define DATATYPES_METADATA_H

typedef struct
{
  int num_layers;
  int num_seconds;
  GSList* layer_timings;
} dt_metadata;

#endif//DATATYPES_METADATA_H
