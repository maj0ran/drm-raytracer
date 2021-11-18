#pragma once

#include "object.h"
#include "vector.h"

typedef struct Sphere {
    struct Element base;
    Point center;
    float radius;
} Sphere;

struct Element *sphere_create(Vec *position, float radius, Color *color);
const char *sphere_print(__attribute__((unused)) struct Element *o);
bool sphere_intersect(struct Element *o, Ray *ray, float *intersect);
Vec sphere_surface_normal(struct Element *o, Point *hit_point);
