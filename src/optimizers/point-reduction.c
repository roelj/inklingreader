#include "point-reduction.h"
#include "../datatypes/coordinate.h"
#include "../datatypes/element.h"

static int
opt_in_between (dt_coordinate* first, dt_coordinate* second, dt_coordinate* third, float factor)
{
  float outer_slope = (first->x - third->x) / (first->y - third->y);
  float inner_slope = (first->x - second->x) / (first->y - second->y);

  float lower = outer_slope * 1 - factor;
  float upper = outer_slope * 1 + factor;

  if ((inner_slope > lower && inner_slope < upper)
      || (inner_slope < lower && inner_slope > upper))
    return 1;

  return 0;
}

int
opt_point_reduction_apply (GSList* data)
{
  GSList* head = data;

  dt_coordinate* first = NULL;
  dt_coordinate* second = NULL;
  dt_coordinate* third = NULL;
  
  while (data != NULL)
    {
      dt_element* e = (dt_element *)data->data;
      if (e->type == TYPE_COORDINATE)
	{
	  if (third == NULL)
	    {
	      if (first == NULL) first = (dt_coordinate *)data->data;
	      else if (second == NULL) second = (dt_coordinate *)data->data;
	      else if (third == NULL) third = (dt_coordinate *)data->data;
	    }
	  else
	    {
	      if (opt_in_between (first, second, third, 0.1))
		head = g_slist_remove (head, second);

	      second = third;
	      third = NULL;
	    }
	}

      /* Reset the optimization when it's the beginning or end of a stroke. */
      if (e->type == TYPE_STROKE)
	{
	  first = NULL;
	  second = NULL;
	  third = NULL;
	}

      data = data->next;
    }

  return 1;
}
