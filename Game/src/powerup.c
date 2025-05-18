#include "powerup.h"
#include "object.h"
#include "animation.h"
#include "scoreManager.h"

#define POWERUP_WIDTH	24
#define POWERUP_HEIGHT	21

#define POWERUP_U_START	0.0318f
#define POWERUP_V_START	1 - 0.1926f
#define POWERUP_U_END	0.1045f
#define POWERUP_V_END	1 - 0.2963f
#define POWERUP_GAP		0.0909f

#define SCROLL_SPEED	-80
#define POWERUP_VALUE	500

typedef struct powerup_t {
	Object obj;

	Animation* anim;
	bool shouldDelete;
}Powerup;

static void _powerupUpdate(Object* obj, uint32_t milliseconds);
static void _powerupDraw(Object* obj);
static void _powerupCallback(Object* obj, uint8_t tag);

static ObjVtable _powerupTable = {
	_powerupDraw,
	_powerupUpdate,
	POWERUP,
	dummyCollide,		//will never collide with anything, just get collided with
	_powerupCallback
};

//spawns the powerup at the position
Powerup* powerupNew(Coord2D pos) {
	Powerup* p = malloc(sizeof(Powerup));

	if (p != NULL) {
		Coord2D vel = { SCROLL_SPEED, 0 };
		Bounds2D hitbox = {
			{pos.x - POWERUP_WIDTH, pos.y - POWERUP_HEIGHT},
			{pos.x + POWERUP_WIDTH, pos.y + POWERUP_HEIGHT}
		};

		objInit(&p->obj, &_powerupTable, pos, vel, hitbox);

		Bounds2D spriteSize = { 
			{-POWERUP_WIDTH, -POWERUP_WIDTH},
			{POWERUP_WIDTH, POWERUP_HEIGHT}
		};

		Bounds2D uv = {
			{POWERUP_U_START, POWERUP_V_START},
			{POWERUP_U_END, POWERUP_V_END}
		};

		p->anim = animationNew(playerSprites, spriteSize, uv, POWERUP_GAP, 0.4f, 3, 0);
		p->shouldDelete = false;
	}

	return p;
}

//getter for observer
bool powerupGetDelete(Powerup* p) {
	return p->shouldDelete;
}

//free powerup and related memory
void powerupDelete(Powerup* p) {
	animationDelete(p->anim);
	
	objDeinit(&p->obj);
	free(p);
}

//scroll powerup
static void _powerupUpdate(Object* obj, uint32_t milliseconds) {
	Powerup* p = (Powerup*)obj;
	animationUpdateTimer(p->anim, milliseconds);
	objDefaultUpdate(obj, milliseconds);
	if (obj->position.x < -POWERUP_WIDTH) {
		p->shouldDelete = true;
	}
}

//draw powerup at position
static void _powerupDraw(Object* obj) {
	Powerup* p = (Powerup*)obj;
	animationDraw(p->anim, obj->position);
}

//increase score and delete on collide
static void _powerupCallback(Object* obj, uint8_t tag) {
	if (tag == PLAYER) {
		Powerup* p = (Powerup*)obj;
		increaseScore(scoreCounter, POWERUP_VALUE);
		p->shouldDelete = true;
	}
}