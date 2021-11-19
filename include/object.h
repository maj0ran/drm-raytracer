#pragma once

#include "color.h"
#include "ray.h"
#include "texture.h"
#include <stdbool.h>
#include <stddef.h>

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
    bool is_textured;
    union {
        Color color;
        Texture texture;
    };
    Surface surface;
} Element;

typedef struct ElementInterface {
    const char *(*print)(struct Element *o);
    bool (*intersect)(struct Element *o, Ray *ray, float *intersect);
    Vec (*surface_normal)(struct Element *o, Point *hit_point);
    TextureCoords (*texture_coords)(struct Element *o, Point *hit_point);
} ElementInterface;

const char *print(struct Element *o);
bool intersect(struct Element *o, Ray *ray, float *intersect);
Vec surface_normal(struct Element *o, Point *hit_point);
TextureCoords texture_coords(struct Element *o, Point *hit_point);
void set_color(struct Element *o, Color surface_color);
void set_texture(struct Element *o, Texture surface_texture);

typedef struct Intersection {
    float distance;
    Element *object;
} Intersection;
