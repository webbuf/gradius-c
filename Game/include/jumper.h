#pragma once
#include "baseTypes.h"
#include "enemy.h"
#include "projectileManager.h"
#include "powerupManager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jumper_t Jumper;

Enemy* jumperNew(Coord2D pos, ProjectileManager* pm, PowerupManager* pum, Object* target, bool hasPowerup);

#ifdef __cplusplus
}
#endif