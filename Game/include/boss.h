#pragma once
#include "projectileManager.h"
#include "baseTypes.h"
#include "object.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct boss_t Boss;

Boss* bossNew(ProjectileManager* pm, Coord2D pos, Object* target);
void bossDelete(Boss* b);

//dynamic enemies are deleted with an observer pattern: their owning managers check to see if their "should delete" bool is true, at which point they're deleted. that bool is private, so a getter
bool bossShouldDelete(Boss* b);

#ifdef __cplusplus
}
#endif