#pragma once

#include "color.h"
#include "drm_helper.h"

void clear(struct drm_dev *dev);
void plot(struct drm_dev *dev, uint16_t x, uint16_t y, Color *color);
