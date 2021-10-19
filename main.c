#include <stdio.h>

#include "plane.h"
#include "sphere.h"

int main() {

  Object *sphere = sphere_create();

  Point p = {.x = 5, .y = 2, .z = 4};
  Vec n = {.x = 3, .y = 2, .z = 7};
  Object *plane = plane_create(p, n);
  printf("%d\n", ((Plane *)plane)->normal.x);
  printf("%s\n", print(sphere));
  printf("%s\n", print(plane));
  return 0;
}
