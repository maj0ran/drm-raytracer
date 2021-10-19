#include "sphere.h"
#include <malloc.h>
#include <memory.h>

const char *sphere_print(__attribute__((unused)) struct Object *o) {
  return "SPHERE";
}

struct Object *sphere_create() {
  static const ObjectInterface vtable = {.print = sphere_print};

  static Object base = {.vtable = &vtable};
  Sphere *sphere = malloc(sizeof(*sphere));
  memcpy(&sphere->base, &base, sizeof(struct Object));
  return &sphere->base;
}
