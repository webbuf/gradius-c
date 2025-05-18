#pragma once
#include "enemy.h"
#include "baseTypes.h"
#include "projectileManager.h"
#include "powerupManager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct shooter_t Shooter;

Enemy* shooterNew(Coord2D pos, ProjectileManager* pm,  PowerupManager* pum, Object* target, bool hasPowerup);

#ifdef __cplusplus
}
#endif