#include "sphere.h"
#include "object.h"
#include "vector.h"
#include <malloc.h>
#include <math.h>
#include <memory.h>

const char *sphere_print(__attribute__((unused)) struct Element *o) {
    return "SPHERE";
}

bool sphere_intersect(struct Element *o, Ray *ray, float *intersect) {
    Sphere *s = (Sphere *)o;
    Vec hypotenuse = v_sub(&s->center, &ray->origin);
    float adj_len2 = v_dot(&hypotenuse, &ray->direction);
    float opp_len2 = v_dot(&hypotenuse, &hypotenuse) - adj_len2 * adj_len2;
    float radius2 = s->radius * s->radius;

    if (opp_len2 > radius2) {
        return false;
    }

    float thc = sqrtf(radius2 - opp_len2);
    float t0 = adj_len2 - thc;
    float t1 = adj_len2 + thc;

    if (t0 < 0.0 && t1 < 0.0) {
        return false;
    } else if (t0 < 0.0) {
        *intersect = t1;
        return true;
    } else if (t1 < 0.0) {
        *intersect = t0;
        return true;
    } else {
        if (t0 < t1)
            *intersect = t0;
        else
            *intersect = t1;
        return true;
    }
}

Vec sphere_surface_normal(Element *o, Point *hit_point) {
    Sphere *s = (Sphere *)o;

    Vec normal = v_sub(hit_point, &s->center);
    return v_normalize(&normal);
}

struct Element *sphere_create(Vec *position, float radius, Color *color) {
    static const ElementInterface vtable = {.print = sphere_print,
                                           .intersect = sphere_intersect,
                                           .surface_normal =
                                               sphere_surface_normal};

    static Element base = {.vtable = &vtable};
    base.color = *color;

    Sphere *sphere = malloc(sizeof(*sphere));
    memcpy(&sphere->base, &base, sizeof(struct Element));

    sphere->center = *position;
    sphere->radius = radius;

    return &sphere->base;
}
