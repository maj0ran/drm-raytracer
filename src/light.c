#include "light.h"
#include "vector.h"
#include <malloc.h>
#include <math.h>
#include <memory.h>

float intensity(struct Light *l, Vec *hit_point) {
    return l->vtable->intensity(l, hit_point);
}

float distance(struct Light *l, Vec *hit_point) {
    return l->vtable->distance(l, hit_point);
}

Vec direction(Light *l, Vec *hit_point) {
    return l->vtable->direction(l, hit_point);
}

float directional_light_intensity(Light *l,
                                  __attribute__((unused)) Vec *hit_point) {
    return l->intensity;
}

float directional_light_distance(__attribute__((unused)) Light *l,
                                 __attribute__((unused)) Vec *hit_point) {
    return INFINITY;
}

Vec directional_light_direction(Light *l,
                                __attribute__((unused)) Vec *hit_point) {
    DirectionalLight *light = (DirectionalLight *)l;
    return light->direction;
}

Light *directional_light_create(Vec direction, Color color, float intensity) {
    static const struct LightInterface vtable = {
        .intensity = directional_light_intensity,
        .distance = directional_light_distance,
        .direction = directional_light_direction,
    };

    static Light base = {.vtable = &vtable};
    base.color = color;
    base.intensity = intensity;

    DirectionalLight *l = malloc(sizeof(*l));
    memcpy(&l->base, &base, sizeof(Light));
    l->direction = v_normalize(&direction);

    return &l->base;
}

float point_light_intensity(Light *l, Vec *hit_point) {
    PointLight *light = (PointLight *)l;
    Vec distance_vector = v_sub(&light->position, hit_point);
    float r2 = v_len2(&distance_vector);

    return l->intensity / (4.0 * M_PI * r2);
}

float point_light_distance(Light *l, Vec *hit_point) {
    PointLight *light = (PointLight *)l;
    return v_dist(hit_point, &light->position);
}

Vec point_light_direction(Light *l, Vec *hit_point) {
    PointLight *light = (PointLight *)l;
    Vec direction = v_sub(hit_point, &light->position);
    return v_normalize(&direction);
}

Light *point_light_create(Vec position, Color color, float intensity) {
    static const struct LightInterface vtable = {
        .intensity = point_light_intensity,
        .distance = point_light_distance,
        .direction = point_light_direction,
    };

    static Light base = {.vtable = &vtable};
    base.color = color;
    base.intensity = intensity;

    PointLight *l = malloc(sizeof(*l));
    memcpy(&l->base, &base, sizeof(Light));
    l->position = position;

    return &l->base;
}
