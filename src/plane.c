#include "plane.h"
#include "object.h"

#include <malloc.h>
#include <memory.h>
#include <sys/cdefs.h>

Element *plane_create(Point *p, Vec *n, Color *c) {
    static const ElementInterface vtable = {
        .print = plane_print,
        .intersect = plane_intersect,
        .surface_normal = plane_surface_normal,
        .texture_coords = plane_texture_coords,
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

TextureCoords plane_texture_coords(struct Element *o, Point *hit_point) {
    Plane *p = (Plane *)o;

    Vec y = {.x = 0.0, .y = 1.0, .z = 0.0};
    Vec z = {.x = 0.0, .y = 0.0, .z = 1.0};
    Vec x_axis;
//        printf("%f %f %f : %f %f %f\n", p->normal.x, p->normal.y, p->normal.z, z.x, z.y, z.z);
    if (is_equal(&p->normal, &z)) {
        x_axis = v_cross(&p->normal, &y);
    } else {
        x_axis = v_cross(&p->normal, &z);
    }
    Vec y_axis = v_cross(&p->normal, &x_axis);
    Vec hit_vec = v_sub(hit_point, &p->origin);

    TextureCoords coords = {.x = v_dot(&hit_vec, &x_axis),
                            .y = v_dot(&hit_vec, &y_axis)};
//    printf("%d %d\n", coords.x, coords.y);
    return coords;
}

const char *plane_print(__attribute__((unused)) struct Element *o) {
    return "PLANE";
}
