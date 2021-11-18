#pragma once

#include <stdint.h>

typedef struct {
    float r, g, b;
} Color;

Color c_add(Color *c1, Color *c2);
Color c_mul(Color *c1, Color *c2);

Color c_add_s(Color *c, float scalar);
Color c_mul_s(Color *c, float scalar);


void clamp(Color *c);
Color generate_random_color();


