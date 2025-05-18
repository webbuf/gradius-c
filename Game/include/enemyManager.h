#pragma once
#include "baseTypes.h"
#include "powerupManager.h"
#include "projectileManager.h"
#include "object.h"
#include "background.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct enemyManager_t EnemyManager;
typedef struct enemyInfo_t	  EnemyInfo;	//struct for holding spawn point information

EnemyManager* emNew(const char* filename, PowerupManager* powerupSpawner, ProjectileManager* projectileSpawner, Background* back, Object* target, uint32_t startingTime);

void emDelete(EnemyManager* dom);

void spawnEnemy(EnemyManager* em, int enemyId, Coord2D pos);

uint32_t emGetTime(EnemyManager* em);

bool emGetWon(EnemyManager* em);

#ifdef __cplusplus
}
#endif