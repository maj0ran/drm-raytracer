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

#define BACKGROUND                                                             \
    (Color) { .r = 0.0, .g = 0.0, .b = 0.0 }
#define BLACK                                                                  \
    (Color) { .r = 0.0, .g = 0.0, .b = 0.0 }
#define RED                                                                    \
    (Color) { .r = 1.0, .g = 0.0, .b = 0.0 }
#define GREEN                                                                  \
    (Color) { .r = 0.0, .g = 1.0, .b = 0.0 }
#define BLUE                                                                   \
    (Color) { .r = 0.0, .g = 0.0, .b = 1.0 }
#define YELLOW                                                                 \
    (Color) { .r = 1.0, .g = 1.0, .b = 0.0 }
#define GREY                                                                   \
    (Color) { .r = 0.3, .g = 0.3, .b = 0.3 }
#define WHITE                                                                  \
    (Color) { .r = 1.0, .g = 1.0, .b = 1.0 }
