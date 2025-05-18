#pragma once
#include "baseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct powerupManager_t PowerupManager;

PowerupManager* powerupManagerNew();

void spawnPowerup(PowerupManager* pm, Coord2D pos);

void powerupManagerDelete(PowerupManager* p);
//these are named inconsistently from the other managers because having pm functions and pum functions would be fucked

#ifdef __cplusplus
}
#endif