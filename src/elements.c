#include "elements.h"
#include <malloc.h>
#include <math.h>
#include <memory.h>

/*************
 * Base
 *************/

const char *print(struct Element *o) { return o->vtable->print(o); }

bool intersect(struct Element *o, Ray *ray, float *intersect) {
    return o->vtable->intersect(o, ray, intersect);
}
Vec surface_normal(struct Element *o, Point *hit_point) {
    return o->vtable->surface_normal(o, hit_point);
}

TextureCoords texture_coords(struct Element *o, Point *hit_point) {
    return o->vtable->texture_coords(o, hit_point);
}

void set_color(struct Element *o, Color surface_color) {
    o->color = surface_color;
    o->is_textured = false;
}

void set_texture(struct Element *o, Texture surface_texture) {
    o->texture = surface_texture;
    o->is_textured = true;
}

/*************
 * Plane
 *************/

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
    if (is_equal(&p->normal, &z)) {
        x_axis = v_cross(&p->normal, &y);
    } else {
        x_axis = v_cross(&p->normal, &z);
    }
    Vec y_axis = v_cross(&p->normal, &x_axis);
    Vec hit_vec = v_sub(hit_point, &p->origin);

    TextureCoords coords = {.x = v_dot(&hit_vec, &x_axis),
                            .y = v_dot(&hit_vec, &y_axis)};
    return coords;
}

const char *plane_print(__attribute__((unused)) struct Element *o) {
    return "PLANE";
}
const char *sphere_print(__attribute__((unused)) struct Element *o) {
    return "SPHERE";
}

/*************
 * Sphere
 *************/

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

TextureCoords sphere_texture_coords(Element *o, Point *hit_point) {
    Sphere *s = (Sphere *)o;

    TextureCoords coords;
    Vec hit_vec = v_sub(hit_point, &s->center);

    double x = (1.0 + atan2(hit_vec.z, hit_vec.x) / M_PI) * 0.5;

    double y = fabs(acos(hit_vec.y / s->radius) / M_PI);

    coords.x = x * o->texture.width;
    coords.y = y * o->texture.height;
    return coords;
}

struct Element *sphere_create(Vec *position, float radius, Color *color) {
    static const ElementInterface vtable = {
        .print = sphere_print,
        .intersect = sphere_intersect,
        .surface_normal = sphere_surface_normal,
        .texture_coords = sphere_texture_coords,
    };

    static Element base = {.vtable = &vtable};
    base.color = *color;

    Sphere *sphere = malloc(sizeof(*sphere));
    memcpy(&sphere->base, &base, sizeof(struct Element));

    sphere->center = *position;
    sphere->radius = radius;

    return &sphere->base;
}
