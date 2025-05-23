#include "color.h"
#include <stddef.h>
typedef struct _Texture {
  size_t width;
  size_t height;

  Color *data;
} Texture;

typedef struct {
  size_t x;
  size_t y;
} TextureCoords;

Texture gen_checkboard_texture(size_t size);
void free_texture(Texture *tex);
