#pragma once

#include "object.h"
#include "vector.h"

typedef struct Sphere {
    struct Object base;
    Point center;
    float radius;
} Sphere;

struct Object *sphere_create(Vec *position, float radius, Color *color);
const char *sphere_print(__attribute__((unused)) struct Object *o);
bool sphere_intersect(struct Object *o, Ray *ray, float *intersect);
Vec sphere_surface_normal(struct Object *o, Point *hit_point);
