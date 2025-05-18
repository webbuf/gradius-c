#include "projectileManager.h"
#include "spinner.h"
#include "enemy.h"
#include "projectiles.h"
#include "soundInfo.h"

//ngl this is the same file as the powerup manager

typedef struct projectileManager_t {
    Object obj;

    Bounds2D level;
    Projectile** projectiles;
}ProjectileManager;

static uint8_t projectileCount = 24;

static void _pmUpdate(Object* obj, uint32_t milliseconds);
static void _pmDraw(Object* obj);
static void _pmTriggerCallback(int32_t id);

static ProjectileManagerCB _playSound = NULL;

static ObjVtable _pmTable = {
    _pmDraw,
    _pmUpdate,
    NOCOLLIDE,
    dummyCollide,
    dummyCallback
};

void spawnProjectile(ProjectileManager* pm, int projectileId, Coord2D pos) {
    int index = -1;

    for (unsigned int i = 0; i < projectileCount; i++) {
        if (pm->projectiles[i] == NULL) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        Projectile* p = NULL;
        switch (projectileId) {
            //spawn the right projectile per the id
        case PROJ_BASE:
            _pmTriggerCallback(SOUND_BASIC_SHOOT);
            p = baseProjectileNew(pos);
            break;
        case PROJ_DOUBLE:
            _pmTriggerCallback(SOUND_BASIC_SHOOT);
            p = doubleNew(pos);
            break;
        case PROJ_MISSILE:
            _pmTriggerCallback(SOUND_MISSILE_SHOOT);
            p = missileNew(pos, pm->level);
            break;
        case PROJ_LASER:
            _pmTriggerCallback(SOUND_BASIC_SHOOT);
            p = laserNew(pos);
            break;
        case PROJ_BOSS:
            _pmTriggerCallback(SOUND_BOSS_SHOOT);
            p = bossProjectileNew(pos);
        default:
            break;
        }
        pm->projectiles[index] = p;
    }
}

void spawnProjectileEnemy(ProjectileManager* pm, Coord2D pos, Coord2D playerPos) {
    int index = -1;

    for (unsigned int i = 0; i < projectileCount; i++) {
        if (pm->projectiles[i] == NULL) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        _pmTriggerCallback(SOUND_BOSS_SHOOT);
        pm->projectiles[index] = enemyProjectileNew(pos, playerPos);
    }
}

ProjectileManager* pmNew(Bounds2D level) {
    ProjectileManager* pm = malloc(sizeof(ProjectileManager));

    if (pm != NULL) {
        pm->projectiles = malloc(sizeof(Projectile*) * projectileCount);

        if (pm->projectiles!= NULL) {
            for (unsigned int i = 0; i < projectileCount; i++) {
                pm->projectiles[i] = NULL;
            }
        }

        Coord2D vel = { 0 , 0 };
        Coord2D pos = { 0, 0 };
        Bounds2D hit = { vel, pos };

        pm->level = level;
        objInit(&pm->obj, &_pmTable, vel, pos, hit);
    }

    return pm;
}

void pmDelete(ProjectileManager* pm) {

    objDeinit(&pm->obj);

    for (int i = 0; i < projectileCount; i++) {
        if (pm->projectiles[i] != NULL) {
            projectileDelete(pm->projectiles[i]);
        }
    }

    free(pm->projectiles);

    free(pm);
}

static void _pmUpdate(Object* obj, uint32_t milliseconds) {
    ProjectileManager* pm = (ProjectileManager*)obj;
    for (int i = 0; i < projectileCount; i++) {
        if (pm->projectiles[i] != NULL) {
            if (projectileGetDelete(pm->projectiles[i]) == true) {
                projectileDelete(pm->projectiles[i]);
                pm->projectiles[i] = NULL;
            }
        }
    }
}

static void _pmDraw(Object* obj) {
    return;
}

static void _pmTriggerCallback(int32_t id) {
    if (_playSound != NULL) {
        _playSound(id);
    }
}

void pmSetCB(ProjectileManagerCB callback) {
    _playSound = callback;
}

void pmClearCB() {
    _playSound = NULL;
}