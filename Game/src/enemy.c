#include <assert.h>

#include "enemy.h"
#include "object.h"
#include "animation.h"
#include "baseTypes.h"
#include "hitbox.h"
#include "scoreManager.h"

static void _enemyUpdate(Object* obj, uint32_t milliseconds);
static void _enemyDraw(Object* obj);
static bool _enemyDoCollisions(Object* obj, Object* other);
static void _enemyCallback(Object* obj, uint8_t tag);

//default parent vtable every enemy has in addition to its own
static ObjVtable _enemyTable = {
	_enemyDraw,
	_enemyUpdate,
	ENEMY,
	_enemyDoCollisions,
	_enemyCallback
};

//initializes an enemy parent object given an enemy, vtable, player target, position, hitbox, animation, delete function, and score value on kill
void enemyInit(Enemy* e, ObjVtable* o, Object* target, Coord2D p, Bounds2D h, Animation* a, EnemyDeleteFunction df, int score){
	assert(e != NULL);
	assert(target != NULL);
	Coord2D vel = { 0, 0 };
	objInit(&e->obj, o, p, vel, h);

	e->eTable = &_enemyTable;
	e->anim = a;
	e->target = target;
	e->shouldDelete = false;
	e->deleteFunction = df;	//given special function to clear any memory specific to a given enemy
	e->scoreValue = score;
}

//default enemy update: moves
static void _enemyUpdate(Object* obj, uint32_t milliseconds) {
	Enemy* e = (Enemy*)obj;
	objDefaultUpdate(obj, milliseconds);
}

//by default draw does nothing, since most enemies have something special that influences how they're drawn and any kind of default would damage that
static void _enemyDraw(Object* obj) {
	return;
}

//by default, collided with
static bool _enemyDoCollisions(Object* obj, Object* other) {
	return false;
}

//frees all memeory related to enemy
void enemyDelete(Enemy* e) {
	e->deleteFunction(e);
	animationDelete(e->anim);	//have to clear our anim
	objDeinit(&e->obj);		    //kill underlying object
	free(e);				    //free enemy mem
}

//enemy callback: if hit, increase score and die
static void _enemyCallback(Object* obj, uint8_t tag) {
	if (tag == PLAYER || tag == PROJECTILE) {
		Enemy* e = (Enemy*)obj;
		increaseScore(scoreCounter, e->scoreValue);
		e->shouldDelete = true;
	}
}