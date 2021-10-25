#include "scene.h"
#include "color.h"
#include "draw.h"
#include "sphere.h"
#include <math.h>

#define EPS 0.01

void scene_init(Scene *scene, size_t capacity) {
    scene->objects = malloc(sizeof(Object *) * capacity);
    scene->capacity = capacity;
    scene->size = 0;
}

int scene_add_object(Scene *scene, Object *obj) {
    if (scene->size == scene->capacity)
        return -1;

    scene->objects[scene->size] = obj;
    scene->size++;

    return 0;
}

void set_sunlight(Scene *scene, Vec *sun_direction, float sun_intensity) {
    scene->sunlight.direction = *sun_direction;
    scene->sunlight.intensity = sun_intensity;
}

Color get_color(Scene *scene, Ray *ray, Intersection *i) {
    Point hit_point = v_mul_s(&ray->direction, i->distance);
    hit_point = v_add(&ray->origin, &hit_point);
    Color c = shadow_diffuse(scene, ray, i);

    return c;
}

Color shadow_diffuse(Scene *scene, Ray *ray, Intersection *i) {
    Color c = {.r = 0.0, .g = 0.0, .b = 0.0};
    Vec dir_to_light = v_neg(&scene->sunlight.direction);
    float light_intensity;
    float light_power;

    Point hit_point = v_mul_s(&ray->direction, i->distance);
    hit_point = v_add(&ray->origin, &hit_point);
    Vec normal = surface_normal(i->object, &hit_point);
    Vec offset = v_mul_s(&normal, EPS);
    Ray shadow_ray = {.origin = v_add(&hit_point, &offset),
                      .direction = dir_to_light};

    Intersection shadow_intersect = trace_ray(scene, &shadow_ray);
    bool in_light = (shadow_intersect.object == NULL);
    if (in_light) {
        light_intensity = scene->sunlight.intensity;
    } else {
        light_intensity = 0.0;
    }
    light_power = v_dot(&normal, &dir_to_light) * light_intensity;
//    printf("%f\n", light_power);
    c = c_add(&c, &i->object->color);
    c = c_mul_s(&c, light_power);
    clamp(&c);
    return c;
}

Intersection trace_ray(Scene *scene, Ray *ray) {
    float distance;
    float nearest_intersection = INFINITY;
    Object *nearest_object = NULL;
    Intersection i;
    for (size_t obj = 0; obj < scene->size; obj++) {
        Object *o = scene->objects[obj];
        if (intersect(o, ray, &distance)) {
            if (distance < nearest_intersection) {
                nearest_intersection = distance;
                nearest_object = o;
            }
        }
    }
    i.distance = nearest_intersection;
    i.object = nearest_object;
    return i;
}

void render(Scene *scene, struct drm_dev *dev) {
    // starting point of the rays
    Vec origin = {.x = 960, .y = 540, .z = -600};
    // One Ray through each pixel
    for (int i = 0; i < 1920; i++) {
        for (int j = 0; j < 1080; j++) {
            Vec target = {.x = i, .y = j, .z = 0};
            Vec direction = v_sub(&target, &origin);
            direction = v_normalize(&direction);
            Ray ray = {origin, direction};
            Intersection intersection = trace_ray(scene, &ray);
            if (intersection.object != NULL) {
                Color c = get_color(scene, &ray, &intersection);
                plot(dev, i, j, &c);
            }
        }
    }
}
