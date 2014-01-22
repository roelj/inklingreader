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

#include "straight_lines.h"
#include "../datatypes/element.h"
#include <malloc.h>

/* This variable controls the strength of the filter. */
#define DEVIATION 25

/*----------------------------------------------------------------------------.
 | OPT_STRAIGHT_LINES_FILTER                                                  |
 | Removes points that seem to be in a line. The end result is a smooth line. |
 '----------------------------------------------------------------------------*/
dt_coordinate*
opt_straight_lines_filter (GSList* current)
{
  GSList* list = current;
  dt_coordinate* current_co = NULL;
  if (list != NULL)
    {
      current_co = (dt_coordinate *)((dt_element *)list->data)->data;
      dt_coordinate* next_co = NULL;

      while (list->next != NULL)
	{
	  list = list->next;
	  dt_element* e = (dt_element *)list->data;
	  if (e->type == TYPE_COORDINATE)
	    {
	      next_co = (dt_coordinate *)((dt_element *)list->data)->data;
	      current_co->x = (next_co->x + current_co->x) / 2;
	    }
	  else if (e->type == BLOCK_STROKE)
	    {
	      break;
	    }
	}
    }

  free (current_co);
  current_co = NULL;
  return NULL;
}
