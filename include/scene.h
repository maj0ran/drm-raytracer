#pragma once

#include "drm_helper.h"
#include "elements.h"
#include "light.h"
#include <malloc.h>
#include <stddef.h>

typedef struct Scene {
  size_t capacity;
  size_t size;

  Element **objects;
  Light_list *lights;
} Scene;

void scene_init(Scene *scene, size_t capacity);
int scene_add_object(Scene *scene, Element *obj);
Intersection trace_ray(Scene *scene, Ray *ray);
void render(Scene *scene, struct drm_dev *dev);
Color cast_ray(Scene *scene, Ray *ray, uint8_t depth);

Color get_color(Scene *scene, Ray *ray, Intersection *i, uint8_t depth);
Color shadow_diffuse(Scene *scene, Ray *ray, Intersection *i);

int scene_add_light(Scene *scene, Light *l);
float fresnel(Ray *ray, Vec *surface_normal, float index);
