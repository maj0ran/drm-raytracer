#include "texture.h"
#include <malloc.h>
#include <stdbool.h>

// size is here the length of one edge of a 2x2 checkboard in pixel
Texture gen_checkboard_texture(size_t size) {
  Texture tex;
  tex.height = size;
  tex.width = size;

  size_t half_edge = size / 2;
  size = size * size;
  Color *data = malloc(size * sizeof(Color));
  bool color_flip = false;
  uint32_t color_flip_count = 0;
  uint32_t i = 1;
  for (Color *c = data; c != data + size; c++, i++) {
    if (i == half_edge) {
      color_flip = !color_flip;
      color_flip_count++;
      i = 0;
    }
    if (color_flip_count == half_edge * 2) {
      color_flip = !color_flip;
      color_flip_count = 0;
    }

    if (!color_flip) {
      c->r = 0.9;
      c->g = 0.9;
      c->b = 0.9;
    } else {
      c->r = 0.2;
      c->g = 0.2;
      c->b = 0.2;
    }
  }
  tex.data = data;

  return tex;
}

void free_texture(Texture *tex) { free(tex->data); }
