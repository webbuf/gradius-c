#pragma once
#include "baseTypes.h"
#ifdef __cplusplus
extern "C" {
#endif

void shapeDrawCircle(float radius, float x, float y, uint8_t r, uint8_t g, uint8_t b, bool filled);
void shapeDrawLine(float startX, float startY, float endX, float endY, uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif