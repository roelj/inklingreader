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
 | This type acts as a wrapper for elements in a list. The list type provides |
 | a void* to the data. To be able to recognize which type is being stored,   |
 | the TYPE_ definition provides the answer. All possible types that are      |
 | defined above.                                                             |
 '----------------------------------------------------------------------------*/

typedef struct
{
  unsigned char type;
  void* data;
} dt_element;

#endif//DATATYPES_ELEMENT_H
