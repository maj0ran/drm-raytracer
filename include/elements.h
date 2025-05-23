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

/*************
 * Base
 *************/

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

/*************
 * Plane
 *************/

typedef struct Plane {
  struct Element base;
  Point origin;
  Vec normal;
} Plane;

Element *plane_create(Point *p, Vec *n, Color *c);
const char *plane_print(__attribute__((unused)) struct Element *o);
bool plane_intersect(struct Element *o, Ray *ray, float *intersect);
Vec plane_surface_normal(Element *o, Vec *hit_point);
TextureCoords plane_texture_coords(struct Element *o, Point *hit_point);

/*************
 * Sphere
 *************/

typedef struct Sphere {
  struct Element base;
  Point center;
  float radius;
} Sphere;

struct Element *sphere_create(Vec *position, float radius, Color *color);
const char *sphere_print(__attribute__((unused)) struct Element *o);
bool sphere_intersect(struct Element *o, Ray *ray, float *intersect);
Vec sphere_surface_normal(struct Element *o, Point *hit_point);
TextureCoords sphere_texture_coords(Element *o, Point *hit_point);
