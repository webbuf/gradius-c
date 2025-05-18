#pragma once
#include "baseTypes.h"
#include "object.h"
#include "animation.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct enemy_t Enemy;

typedef void (*EnemyDeleteFunction)(Enemy*);

enum Enemies {
	ENEMY_SPINNER,
	ENEMY_CHASER,
	ENEMY_SINNER,
	ENEMY_SINNER_POWERUP,
	ENEMY_SHOOTER,
	ENEMY_SHOOTER_POWERUP,
	ENEMY_JUMPER,
	ENEMY_JUMPER_POWERUP,
	ENEMY_STOMPER,
	ENEMY_STOMPER_POWERUP,
	ENEMY_SPAWN_WALL,
	ENEMY_DESPAWN_WALL,
	ENEMY_BOSS
};
//enum for spawning in the right enemy

typedef struct enemy_t {
	Object obj;
	ObjVtable* eTable;	//for polymorphism: the "parent" functions

	Object* target;		//object* for the player so we know where the player is --> could theoretically track anything, but it's always the player

	Animation* anim;

	int scoreValue;
	bool shouldDelete;
	EnemyDeleteFunction deleteFunction;
}Enemy;

void enemyInit(Enemy* e, ObjVtable* o, Object* target, Coord2D p, Bounds2D h, Animation* a, EnemyDeleteFunction df, int score);

void enemyDelete(Enemy* e);

#ifdef __cplusplus
}
#endif