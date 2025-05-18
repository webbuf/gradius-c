#pragma once
#include "Object.h"
#include "baseTypes.h"
#include "enemy.h"
#include "powerupManager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sinner_t Sinner;

Enemy* sinnerNew(Coord2D pos, Object* target, PowerupManager* pm, bool hasPowerup);

#ifdef __cplusplus
}
#endif