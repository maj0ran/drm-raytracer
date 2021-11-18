#pragma once

#include "drm_helper.h"
#include "object.h"
#include "sunlight.h"
#include <malloc.h>
#include <stddef.h>

typedef struct Scene {
    size_t capacity;
    size_t size;

    Object **objects;

    Sunlight sunlight;
    Spotlight_list *spotlights;
} Scene;

void scene_init(Scene *scene, size_t capacity);
void set_sunlight(Scene *scene, Vec *sun_direction, float sun_intensity);
int scene_add_object(Scene *scene, Object *obj);
Intersection trace_ray(Scene *scene, Ray *ray);
void render(Scene *scene, struct drm_dev *dev);
Color cast_ray(Scene *scene, Ray *ray, uint8_t depth);

Color get_color(Scene *scene, Ray *ray, Intersection *i, uint8_t depth);
Color shadow_diffuse(Scene *scene, Ray *ray, Intersection *i);

int scene_add_spotlight(Scene *scene, Spotlight *l);
float fresnel(Scene *scene, Ray *ray, Vec *surface_normal, float index);
