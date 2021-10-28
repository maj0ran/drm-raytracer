#pragma once

#include "vector.h"
#include "color.h"

typedef struct Sunlight {
    Vec direction;
    float intensity;
} Sunlight;

typedef struct Spotlight {
    Color color;
    Point position;
    float intensity;
} Spotlight;

typedef struct Spotlight_list {
    Spotlight light;
    struct Spotlight_list *next;
} Spotlight_list;
