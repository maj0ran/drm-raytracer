#include "object.h"

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

