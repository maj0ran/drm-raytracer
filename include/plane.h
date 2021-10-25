#pragma once

#include "object.h"
#include "vector.h"

typedef struct Plane {
  struct Object base;
  Point origin;
  Vec normal;
} Plane;

Object *plane_create(Point p, Vec n);
const char *plane_print(__attribute__((unused)) struct Object *o);
bool plane_intersect(struct Object *o, Ray *ray, float *intersect);
