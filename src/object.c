#include "object.h"

const char *print(struct Object *o) { return o->vtable->print(o); }

bool intersect(struct Object *o, Ray *ray, float *intersect) {
    return o->vtable->intersect(o, ray, intersect);
}
Vec surface_normal(struct Object *o, Point *hit_point) {
    return o->vtable->surface_normal(o, hit_point);
}
