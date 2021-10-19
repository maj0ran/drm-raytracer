#pragma once

#include <stdint.h>

typedef struct Vec {
  uint32_t x, y, z;
} Vec;

typedef Vec Point;

typedef struct {
  uint32_t r, g, b;
} Color;
