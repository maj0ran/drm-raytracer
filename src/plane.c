#include "plane.h"

#include <malloc.h>
#include <memory.h>

const char *plane_print(__attribute__((unused)) struct Object *o) {
  return "PLANE";
}

Object *plane_create(Point p, Vec n) {
  static const ObjectInterface vtable = {.print = plane_print};

  static Object base = {.vtable = &vtable};
  Plane *plane = malloc(sizeof(*plane));
  plane->origin = p;
  plane->normal = n;
  memcpy(&plane->base, &base, sizeof(struct Object));
  return &plane->base;
}
