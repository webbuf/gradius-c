#include "explosion.h"
#include "object.h"
#include "animation.h"
#include "soundInfo.h"

#define EXPLOSION_WIDTH   22.5f
#define EXPLOSION_HEIGHT  24
#define EXPLOSION_START_U 0.0174f
#define EXPLOSION_START_V 1 - 0.8358f
#define EXPLOSION_END_U   0.0609f
#define EXPLOSION_END_V   1 - 0.9552f
#define EXPLOSION_GAP     0.0435f

#define EXPLOSION_LIFETIME 332
#define SCROLL_SPEED	  -80	
//moves at this speed to stay static

typedef struct explosion_t {
	Object obj;

	Animation* anim;
	bool shouldDelete;
	uint16_t deleteTimer;	//only lives so long and then it goes away
}Explosion;

//da object functions
static void _explosionUpdate(Object* obj, uint32_t milliseconds);
static void _explosionDraw(Object* obj);
static void _explosionTriggerCallback(int32_t index);

//sound callback
static ExplosionCB _playSound = NULL;

static ObjVtable _explosionTable = {
	_explosionDraw,
	_explosionUpdate,
	NOCOLLIDE,
	dummyCollide,
	dummyCallback
};

//makes a new explosion at the given position
Explosion* explosionNew(Coord2D pos) {
	Explosion* e = malloc(sizeof(Explosion));
	if (e != NULL) {
		_explosionTriggerCallback(SOUND_ENEMY_DIE);

		Coord2D vel = { SCROLL_SPEED, 0 };
		Bounds2D hit = {
			{pos.x - EXPLOSION_WIDTH, pos.y - EXPLOSION_HEIGHT},
			{pos.x + EXPLOSION_WIDTH, pos.y + EXPLOSION_HEIGHT}
		};
		objInit(&e->obj, &_explosionTable, pos, vel, hit);

		e->shouldDelete = false;
		e->deleteTimer = 0;

		Bounds2D spriteSize = {
			{ -EXPLOSION_WIDTH,  -EXPLOSION_HEIGHT},
			{EXPLOSION_WIDTH, EXPLOSION_HEIGHT}
		};

		Bounds2D spriteUV = {
			{EXPLOSION_START_U, EXPLOSION_START_V},
			{EXPLOSION_END_U, EXPLOSION_END_V}
		};
		e->anim = animationNew(enemySprites, spriteSize, spriteUV, EXPLOSION_GAP, -0.1f, 4, 0);
	}
	return e;
}

//update function
static void _explosionUpdate(Object* obj, uint32_t milliseconds) {
	Explosion* e = (Explosion*)obj;
	e->deleteTimer += milliseconds;
	if (e->deleteTimer > EXPLOSION_LIFETIME) {
		e->shouldDelete = true;
	}
	animationUpdateTimer(e->anim, milliseconds);
	objDefaultUpdate(obj, milliseconds);
	//updates life an anim timers
}

//draws animation at position
static void _explosionDraw(Object* obj) {
	Explosion* e = (Explosion*)obj;
	animationDraw(e->anim, obj->position);
}

//free object and related memory
void explosionDelete(Explosion* e) {
	objDeinit(&e->obj);
	animationDelete(e->anim);
	free(e);
}

//getter for observer
bool shouldDeleteExplosion(Explosion* e) {
	return e->shouldDelete;
}

//play sound if it's good
static void _explosionTriggerCallback(int32_t index) {
	if (_playSound != NULL) {
		_playSound(index);
	}
}

//functions for setting sound callback
void explosionSetCB(ExplosionCB callback) {
	_playSound = callback;
}

void explosionClearCB() {
	_playSound = NULL;
}