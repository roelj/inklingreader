#ifndef DATATYPES_ELEMENT_H
#define DATATYPES_ELEMENT_H

#include "coordinate.h"
#include "stroke.h"

#define TYPE_STROKE     0
#define TYPE_COORDINATE 1

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
