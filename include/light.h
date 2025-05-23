#pragma once

#include "color.h"
#include "vector.h"

typedef struct Light {
  struct LightInterface const *vtable;

  Color color;
  float intensity;
} Light;

struct LightInterface {
  float (*intensity)(Light *l, Vec *hit_point);
  float (*distance)(Light *l, Vec *hit_point);
  Vec (*direction)(Light *l, Vec *hit_point);
};

float intensity(Light *l, Vec *hit_point);
float distance(Light *l, Vec *hit_point);
Vec direction(Light *l, Vec *hit_point);

typedef struct DirectionalLight {
  Light base;
  Vec direction;
} DirectionalLight;

typedef struct PointLight {
  Light base;
  Color color;
  Point position;
} PointLight;

typedef struct Light_list {
  Light *light;
  struct Light_list *next;
} Light_list;

Light *directional_light_create(Vec direction, Color color, float intensity);
float directional_light_intensity(Light *l, Vec *hit_point);
float directional_light_distance(Light *l, Vec *hit_point);
Vec directional_light_direction(Light *l, Vec *hit_point);

Light *point_light_create(Vec position, Color color, float intensity);
float point_light_intensity(Light *l, Vec *hit_point);
float point_light_distance(Light *l, Vec *hit_point);
Vec point_light_direction(Light *l, Vec *hit_point);
