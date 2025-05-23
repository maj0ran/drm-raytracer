#include "ray.h"

inline Ray new_ray(Vec *origin, Vec *direction) {
  Ray ray;
  ray.origin = *origin;
  ray.direction = *direction;

  return ray;
}
