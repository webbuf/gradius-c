#pragma once
#include "baseTypes.h"
#include "projectileManager.h"
#include "object.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vic_t VicViper;

enum vicState {
	VIC_STATE_ALIVE,
	VIC_STATE_DYING,
	VIC_STATE_DEAD
};
//player sprites so the game manager know what's going on

typedef struct powerupInfo_t {
	uint8_t powerupLevel;
	bool hasMissile;
	bool hasDouble;
	bool hasLaser;
	uint8_t followerCount;
	uint8_t shieldHealth;
	uint8_t speedLevel;
}PowerupInfo;
//struct that holds all the information about what our current power level is
//used for both determining what happens in gameplay and drawing it

typedef void (*VicViperCB)(int32_t);

VicViper* vicNew(Bounds2D level, ProjectileManager* pm, uint8_t startingPowerLevel);	//don't need to give a bounds, since his size will be fixed
void vicDelete(VicViper* vic);

Coord2D getPosition(VicViper* vic);
PowerupInfo* getPowerupInfo(VicViper* vic);
Object* vicGetObject(VicViper* vic);
uint8_t vicGetState(VicViper* vic);
//lotta getters cause a lot hangs on what's going on with the player

void vicSetCB(VicViperCB callback);
void vicClearCB();

#ifdef __cplusplus
}
#endif