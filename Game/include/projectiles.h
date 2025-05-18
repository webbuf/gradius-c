#pragma once

#include "baseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct proj_t Projectile;	//abstract class

typedef struct baseProj_t BaseProjectile;

typedef struct missile_t Missile;

enum Projectiles {
	PROJ_BASE,
	PROJ_DOUBLE,
	PROJ_MISSILE,
	PROJ_LASER,
	PROJ_BOSS,
	PROJ_ENEMY
};
//enum for spawns

Projectile* baseProjectileNew(Coord2D pos);
Projectile* doubleNew(Coord2D pos);
Projectile* missileNew(Coord2D pos, Bounds2D level);
Projectile* laserNew(Coord2D pos);
Projectile* bossProjectileNew(Coord2D pos);
Projectile* enemyProjectileNew(Coord2D pos, Coord2D playerPos);

void projectileDelete(Projectile* p);

bool projectileGetDelete(Projectile* p);

#ifdef __cplusplus
}
#endif