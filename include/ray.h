#pragma once

#include "vector.h"

typedef struct {
  Point origin;
  Vec direction;
} Ray;

Ray new_ray(Vec *origin, Vec *direction);
