#pragma once

#include "object.h"
#include "vector.h"

typedef struct Plane {
    struct Element base;
    Point origin;
    Vec normal;
} Plane;

Element *plane_create(Point *p, Vec *n, Color *c);
const char *plane_print(__attribute__((unused)) struct Element *o);
bool plane_intersect(struct Element *o, Ray *ray, float *intersect);
Vec plane_surface_normal(Element *o, Vec *hit_point);
TextureCoords plane_texture_coords(struct Element *o, Point *hit_point);
