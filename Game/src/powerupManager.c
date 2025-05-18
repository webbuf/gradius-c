#include "powerupManager.h"
#include "powerup.h"
#include "Object.h"

#include <stdlib.h>

typedef struct powerupManager_t {
	Object obj;

	Powerup** powerups;
}PowerupManager;

static uint8_t powerupCount = 12;

static void _powerupManagerUpdate(Object* obj, uint32_t milliseconds);
static void _powerupManagerDraw(Object* obj);

static ObjVtable _pmTable = {
    _powerupManagerDraw,
    _powerupManagerUpdate,
    NOCOLLIDE,
    dummyCollide,
    dummyCallback
};

//find valid spot and spawn the powerup
void spawnPowerup(PowerupManager* pm, Coord2D pos) {
    int index = -1;

    for (unsigned int i = 0; i < powerupCount; i++) {
        if (pm->powerups[i] == NULL) {
            index = i;
            break;
        }
    }

    pm->powerups[index] = powerupNew(pos);
    
}

//make new powerup manager and null out all relevant memory
PowerupManager* powerupManagerNew() {
    PowerupManager* pm = malloc(sizeof(PowerupManager));

    if (pm != NULL) {
        pm->powerups = malloc(sizeof(Powerup*) * powerupCount);

        if (pm->powerups != NULL) {
            for (unsigned int i = 0; i < powerupCount; i++) {
                pm->powerups[i] = NULL;
            }
        }

        Coord2D vel = { 0 , 0 };
        Coord2D pos = { 0, 0 };
        Bounds2D hit = { vel, pos };

        objInit(&pm->obj, &_pmTable, vel, pos, hit);
    }

    return pm;
}

//free powerup manager and any existing powerups
void powerupManagerDelete(PowerupManager* pm) {

    objDeinit(&pm->obj);

    for (int i = 0; i < powerupCount; i++) {
        if (pm->powerups[i] != NULL) {
            powerupDelete(pm->powerups[i]);
        }
    }

    free(pm->powerups);

    free(pm);
}

//monitor powerups to see if they need deletion
static void _powerupManagerUpdate(Object* obj, uint32_t milliseconds) {
    PowerupManager* pm = (PowerupManager*)obj;
    for (int i = 0; i < powerupCount; i++) {
        if (pm->powerups[i] != NULL) {
            if (powerupGetDelete(pm->powerups[i]) == true) {
                powerupDelete(pm->powerups[i]);
                pm->powerups[i] = NULL;
            }
        }
    }
}

static void _powerupManagerDraw(Object* obj) {
    return;
}