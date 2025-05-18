#pragma once
#include "baseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct explosion_t Explosion;

typedef void (*ExplosionCB)(int32_t);

Explosion* explosionNew(Coord2D pos);

void explosionDelete(Explosion* e);

bool shouldDeleteExplosion(Explosion* e);

void explosionSetCB(ExplosionCB callback);
void explosionClearCB();
//explosions play sounds, so they have an audio callback

#ifdef __cplusplus
}
#endif