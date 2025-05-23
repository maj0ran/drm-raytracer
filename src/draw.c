#include "draw.h"
#include <memory.h>

void clear(struct drm_dev *dev) {
  uint16_t w = dev->buf.width;
  uint16_t h = dev->buf.height;
  uint8_t b = dev->buf.bpp / 8;

  memset(dev->buf.map, 0, w * h * b);
}

void plot(struct drm_dev *dev, uint16_t x, uint16_t y, Color *color) {
  uint8_t bytes = dev->buf.bpp / 8;
  uint32_t off = dev->buf.stride * y + x * bytes;
  uint8_t r = (uint8_t)(color->r * 255);
  uint8_t g = (uint8_t)(color->g * 255);
  uint8_t b = (uint8_t)(color->b * 255);
  *(uint32_t *)&dev->buf.map[off] = (r << 16) | (g << 8) | b;
}
