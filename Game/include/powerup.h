#pragma once
#include "baseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct powerup_t Powerup;

Powerup* powerupNew(Coord2D pos);

void powerupDelete(Powerup* p);

bool powerupGetDelete(Powerup* p);

#ifdef __cplusplus
}
#endif