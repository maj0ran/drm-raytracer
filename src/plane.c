#include "plane.h"

#include <malloc.h>
#include <memory.h>

Object *plane_create(Point p, Vec n) {
  static const ObjectInterface vtable = {.print = plane_print,
                                         .intersect = plane_intersect};

  static Object base = {.vtable = &vtable};
  Plane *plane = malloc(sizeof(*plane));
  plane->origin = p;
  plane->normal = n;
  memcpy(&plane->base, &base, sizeof(struct Object));
  return &plane->base;
}

bool plane_intersect(struct Object *o, Ray *ray, float *intersect) {
  Plane *p = (Plane *)o;
  float denom = v_dot(&p->normal, &ray->direction);
  if (denom > 1e-6) {
    Vec v = v_sub(&p->origin, &ray->origin);
    float distance = v_dot(&v, &p->normal) / denom;
    if (distance >= 0.0) {
      *intersect = distance;
      return true;
    }
  }
  return false;
}

const char *plane_print(__attribute__((unused)) struct Object *o) {
  return "PLANE";
}
