#pragma once
#include "baseTypes.h"
#include "enemy.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct chaser_t Chaser;

Enemy* chaserNew(Coord2D pos, Object* target);

//enemy deletes are handled from within: therefore their delete functions are private

#ifdef __cplusplus
}
#endif