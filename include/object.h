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

typedef struct Object {
    const struct ObjectInterface *const vtable;
    Color color;
    Surface surface;
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
