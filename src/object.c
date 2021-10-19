#include "object.h"

const char *print(struct Object *o) { return o->vtable->print(o); }
