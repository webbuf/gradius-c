#pragma once
#include "baseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void updateHitbox(Bounds2D* hitbox, float x, float y, uint32_t milliseconds);

bool colliding(Bounds2D* h1, Bounds2D* h2);

void drawHitbox(Bounds2D* h);

#ifdef __cplusplus
}
#endif