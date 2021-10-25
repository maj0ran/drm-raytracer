#pragma once

#include <stdint.h>

typedef struct Vec {
  float x, y, z;
} Vec;

typedef Vec Point;

Vec v_add(Vec *v1, Vec *v2);
Vec v_sub(Vec *v1, Vec *v2);
Vec v_mul(Vec *v1, Vec *v2);
Vec v_mul_s(Vec *v1, float scalar);
Vec v_add_s(Vec *v1, float scalar);
Vec v_sub_s(Vec *v1, float scalar);
Vec v_neg(Vec *v);
float v_len2(Vec *v);
float v_len(Vec *v);
float v_dot(Vec *v1, Vec *v2);
Vec v_normalize(Vec *v);
