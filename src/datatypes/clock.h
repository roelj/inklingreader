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
 * @file   datatypes/clock.h
 * @brief  A datatype to store clock data.
 * @author Roel Janssen
 * @namespace datatypes
 */

#ifndef DATATYPES_CLOCK_H
#define DATATYPES_CLOCK_H

/**
 * This value represents the time elapsed for each coordinate that was sent to
 * the receiver. The value has been obtained by drawing continously for a long
 * time period and averaging the values (I took sample data of 153 seconds).
 */
#define CLOCK_FREQUENCY 0.0065535852

/**
 * This struct contains the variables that can be extracted for clock data.
 */
typedef struct
{
  unsigned char type;
  unsigned short counter;
} dt_clock;

#endif//DATATYPES_CLOCK_H
