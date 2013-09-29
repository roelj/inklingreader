#ifndef DATATYPES_STATISTICS_H
#define DATATYPES_STATISTICS_H

/*----------------------------------------------------------------------------.
 | FILE STATISTICS DEFINITION                                                 |
 '----------------------------------------------------------------------------*/
typedef struct {
  int objects;
  int strokes;
  int coordinates;
  int pressure;
  int tilt;
  int unknown;
} dt_statistics;

#endif//DATATYPES_STATISTICS_H
