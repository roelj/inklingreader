#ifndef DATATYPES_STROKE_H
#define DATATYPES_STROKE_H

#include "coordinate.h"

/*----------------------------------------------------------------------------.
 | STROKE DATATYPE DEFINITION                                                 |
 '----------------------------------------------------------------------------*/
typedef struct
{
  dt_coordinate pen;
  dt_coordinate tilt;
  int pressure;
} dt_stroke;

#endif//DATATYPES_STROKE_H
