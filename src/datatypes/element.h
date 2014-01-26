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

#ifndef DATATYPES_ELEMENT_H
#define DATATYPES_ELEMENT_H

#include "coordinate.h"
#include "stroke.h"
#include "pressure.h"
#include "tilt.h"

/* Definitions of datatypes. */
#define TYPE_STROKE     0
#define TYPE_COORDINATE 1
#define TYPE_TILT       2
#define TYPE_PRESSURE   3

/* Definitions for markers of the WPI file format. */
#define BLOCK_STROKE     241
#define BLOCK_COORDINATE 97
#define BLOCK_PRESSURE   100
#define BLOCK_TILT       101
#define BEGIN_STROKE     1
#define END_STROKE       0
#define NEW_LAYER        128

/*----------------------------------------------------------------------------.
 | ELEMENT                                                                    |
 | This struct contains the common element in all datatypes defined here:     |
 | a specifier for which type of element the struct is. In a list, these      |
 | structs are all pointers to void. To know which type a struct is, the      |
 | 'type' field is used.                                                      |
 '----------------------------------------------------------------------------*/

typedef struct
{
  unsigned char type;
} dt_element;

#endif//DATATYPES_ELEMENT_H
