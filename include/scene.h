#pragma once

#include "object.h"
#include "sunlight.h"
#include <stddef.h>
#include <malloc.h>
#include "drm_helper.h"

typedef struct Scene {
    size_t capacity;
    size_t size;

    Object **objects;
    Sunlight sunlight;

} Scene;

void scene_init(Scene *scene, size_t capacity);
void set_sunlight(Scene *scene, Vec *sun_direction, float sun_intensity);
int scene_add_object(Scene *scene, Object *obj);
Intersection trace_ray(Scene *scene, Ray *ray);
void render(Scene *scene, struct drm_dev *dev);
Color cast_ray(Scene *scene,  Ray *ray, uint8_t depth); 

Color get_color(Scene *scene, Ray *ray, Intersection *i, uint8_t depth);
Color shadow_diffuse(Scene *scene, Ray *ray, Intersection *i);
