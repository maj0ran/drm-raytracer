#include "plane.h"

#include <malloc.h>
#include <memory.h>
#include <sys/cdefs.h>

Element *plane_create(Point *p, Vec *n, Color *c) {
    static const ElementInterface vtable = {
        .print = plane_print,
        .intersect = plane_intersect,
        .surface_normal = plane_surface_normal,
    };

    static Element base = {.vtable = &vtable};
    base.color = *c;
    Plane *plane = malloc(sizeof(*plane));
    plane->origin = *p;
    plane->normal = *n;
    memcpy(&plane->base, &base, sizeof(struct Element));
    return &plane->base;
}

Vec plane_surface_normal(Element *o, __attribute__((unused)) Vec *hit_point) {
    Plane *p = (Plane *)o;
    return v_neg(&p->normal);
}

bool plane_intersect(struct Element *o, Ray *ray, float *intersect) {
    Plane *p = (Plane *)o;
    float denom = v_dot(&p->normal, &ray->direction);
    if (denom > 0.0001) {
        Vec v = v_sub(&p->origin, &ray->origin);
        float distance = v_dot(&v, &p->normal) / denom;
        if (distance >= 0.0) {
            *intersect = distance;
            return true;
        }
    }
    return false;
}

const char *plane_print(__attribute__((unused)) struct Element *o) {
    return "PLANE";
}
