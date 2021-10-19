#pragma once

#include "common.h"
#include "object.h"

typedef struct Sphere {
  struct Object base;
  Point center;
  float radius;
} Sphere;

struct Object *sphere_create();
const char *sphere_print(__attribute__((unused)) struct Object *o);
