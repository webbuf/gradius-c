#pragma once
#include "baseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wall_t Wall;

Wall* wallNew(Coord2D pos);
void wallDelete(Wall* w);
bool shouldDeleteWall(Wall* w);
void wallStopLooping(Wall* w);

#ifdef __cplusplus
}
#endif