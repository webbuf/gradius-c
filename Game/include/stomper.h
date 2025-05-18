#pragma once
#include "enemy.h"
#include "baseTypes.h"
#include "powerupManager.h"
#include "projectileManager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct stomper_t Stomper;

Enemy* stomperNew(Coord2D pos, ProjectileManager* pm, PowerupManager* pum, Object* target, bool hasPowerup);

#ifdef __cplusplus
}
#endif