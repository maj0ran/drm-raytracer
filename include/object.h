#pragma once

#include "color.h"
#include "ray.h"
#include <stdbool.h>

typedef enum SURFACE_TYPE {
    Diffuse,
    Reflective,
    Refractive,
} SurfaceType;

typedef struct _Surface {
    SurfaceType type;
    union {
        float reflectivity;
        struct {
            float index;
            float transparency;
        };
    };
} Surface;

typedef struct Element {
    const struct ElementInterface *const vtable;
    Color color;
    Surface surface;
} Element;

typedef struct ElementInterface {
    const char *(*print)(struct Element *o);
    bool (*intersect)(struct Element *o, Ray *ray, float *intersect);
    Vec (*surface_normal)(struct Element *o, Point *hit_point);
} ElementInterface;

const char *print(struct Element *o);
bool intersect(struct Element *o, Ray *ray, float *intersect);
Vec surface_normal(struct Element *o, Point *hit_point);

typedef struct Intersection {
    float distance;
    Element *object;
} Intersection;
