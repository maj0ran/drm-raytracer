#pragma once

#include "color.h"
#include "ray.h"
#include <stdbool.h>

typedef struct Object {
  const struct ObjectInterface *const vtable;
  Color color;
} Object;

typedef struct ObjectInterface {
  const char *(*print)(struct Object *o);
  bool (*intersect)(struct Object *o, Ray *ray, float *intersect);
  Vec (*surface_normal)(struct Object *o, Point *hit_point);
} ObjectInterface;

const char *print(struct Object *o);
bool intersect(struct Object *o, Ray *ray, float *intersect);
Vec surface_normal(struct Object *o, Point *hit_point);

typedef struct Intersection {
    float distance;
    Object *object;
} Intersection;
