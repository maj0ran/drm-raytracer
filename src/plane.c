#include "plane.h"

#include <malloc.h>
#include <memory.h>
#include <sys/cdefs.h>

Object *plane_create(Point *p, Vec *n, Color *c) {
    static const ObjectInterface vtable = {
        .print = plane_print,
        .intersect = plane_intersect,
        .surface_normal = plane_surface_normal,
    };

    static Object base = {.vtable = &vtable};
    base.color = *c;
    Plane *plane = malloc(sizeof(*plane));
    plane->origin = *p;
    plane->normal = *n;
    memcpy(&plane->base, &base, sizeof(struct Object));
    return &plane->base;
}

Vec plane_surface_normal(Object *o, __attribute__((unused)) Vec *hit_point) {
    Plane *p = (Plane *)o;
    return p->normal;
}

bool plane_intersect(struct Object *o, Ray *ray, float *intersect) {
    Plane *p = (Plane *)o;
    float denom = v_dot(&p->normal, &ray->direction);
    if (denom > 0.01) {
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
