#pragma once
#include "baseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct projectileManager_t ProjectileManager;

typedef void (*ProjectileManagerCB)(int32_t);

ProjectileManager* pmNew(Bounds2D level);

void spawnProjectile(ProjectileManager* em, int projectileId, Coord2D pos);
void spawnProjectileEnemy(ProjectileManager* pm, Coord2D pos, Coord2D playerPos);
//spawn projectile enemy has to be separate because it needs two positions unfortunately

void pmDelete(ProjectileManager* dom);

void pmSetCB(ProjectileManagerCB callback);
void pmClearCB();

#ifdef __cplusplus
}
#endif