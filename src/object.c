#include "object.h"

const char *print(struct Element *o) { return o->vtable->print(o); }

bool intersect(struct Element *o, Ray *ray, float *intersect) {
    return o->vtable->intersect(o, ray, intersect);
}
Vec surface_normal(struct Element *o, Point *hit_point) {
    return o->vtable->surface_normal(o, hit_point);
}
