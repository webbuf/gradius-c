#pragma once
#include "enemy.h"
#include "baseTypes.h"
#include "powerupManager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spinner_t Spinner;
typedef struct spinnerBuddy_t SpinnerBuddy;

Enemy* spinnerNew(Coord2D pos, SpinnerBuddy* track, Object* target);
SpinnerBuddy* spinnerBuddyNew(PowerupManager* spawner);

#ifdef __cplusplus
}
#endif