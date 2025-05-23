#include "vector.h"
#include <math.h>

Vec v_add(Vec *v1, Vec *v2) {
  Vec result = {
      .x = v1->x + v2->x,
      .y = v1->y + v2->y,
      .z = v1->z + v2->z,
  };
  return result;
}

Vec v_sub(Vec *v1, Vec *v2) {
  Vec result = {
      .x = v1->x - v2->x,
      .y = v1->y - v2->y,
      .z = v1->z - v2->z,
  };
  return result;
}

Vec v_mul(Vec *v1, Vec *v2) {
  Vec result = {
      .x = v1->x * v2->x,
      .y = v1->y * v2->y,
      .z = v1->z * v2->z,
  };
  return result;
}

Vec v_add_s(Vec *v1, float scalar) {
  Vec result = {
      .x = v1->x + scalar,
      .y = v1->y + scalar,
      .z = v1->z + scalar,
  };
  return result;
}

Vec v_sub_s(Vec *v1, float scalar) {
  Vec result = {
      .x = v1->x - scalar,
      .y = v1->y - scalar,
      .z = v1->z - scalar,
  };
  return result;
}

Vec v_mul_s(Vec *v1, float scalar) {
  Vec result = {
      .x = v1->x * scalar,
      .y = v1->y * scalar,
      .z = v1->z * scalar,
  };
  return result;
}

Vec v_neg(Vec *v) {
  Vec result = {
      .x = -v->x,
      .y = -v->y,
      .z = -v->z,
  };
  return result;
}

float v_len2(Vec *v) { return v->x * v->x + v->y * v->y + v->z * v->z; }

float v_len(Vec *v) { return sqrtf(v_len2(v)); }

float v_dot(Vec *v1, Vec *v2) {
  return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

float v_dist(Vec *v1, Vec *v2) {
  Vec diff = v_sub(v1, v2);
  return v_len(&diff);
}

Vec v_normalize(Vec *v) {
  float len = v_len(v);
  Vec result = {
      .x = v->x / len,
      .y = v->y / len,
      .z = v->z / len,
  };
  return result;
}

Vec v_cross(Vec *a, Vec *b) {
  Vec result;
  result.x = a->y * b->z - a->z * b->y;
  result.y = a->z * b->x - a->x * a->z;
  result.z = a->x * b->y - a->y * b->x;

  return result;
}

bool is_equal(Vec *a, Vec *b) {
  return a->x == b->x && a->y == b->y && a->z == b->z;
}
