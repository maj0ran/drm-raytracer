#pragma once

typedef struct Object {
  const struct ObjectInterface *const vtable;
} Object;

typedef struct ObjectInterface {
  const char *(*print)(struct Object *o);
} ObjectInterface;

const char *print(struct Object *o);
