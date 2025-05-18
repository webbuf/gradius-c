#include <assert.h>
#include <stdlib.h>

#include "chaser.h"

#define CHASER_WIDTH  21
#define CHASER_HEIGHT 24

#define CHASER_START_U 0.0144f
#define CHASER_START_V 1 - 0.194f
#define CHASER_END_U   0.0551f
#define CHASER_END_V   1 - 0.3134f
#define CHASER_GAP     0.0551f

#define CHASER_VELOCITY 175

#define CHASE_THRESHOLD   20
//minimum y distance to trigger going after the player

enum chserSprites {
	SPRITE_LOOK_DOWN,
	SPRITE_LOOK_FORWARD,
	SPRITE_LOOK_UP
};
//aliases for sprite indices to avoid magic numbers

typedef struct chaser_t {
	Enemy e;
}Chaser;

//static obj functions
static void _chaserUpdate(Object* obj, uint32_t milliseconds);
static void _chaserDraw(Object* obj);
static bool _chaserDoCollisions(Object* obj, Object* other);
static void _chaserCallback(Object* obj, uint8_t tag);
static void _chaserDelete(Enemy* e);

static ObjVtable _chaserTable = {
	_chaserDraw,
	_chaserUpdate,
	ENEMY,
	_chaserDoCollisions,
	_chaserCallback
};

//makes a new chaser at pos targetting player
Enemy* chaserNew(Coord2D pos, Object* target) {
	assert(target != NULL);
	Chaser* c = malloc(sizeof(Chaser));
	if (c != NULL){
		Bounds2D hit = {
			{pos.x - CHASER_WIDTH, pos.y - CHASER_HEIGHT},
			{pos.x + CHASER_WIDTH, pos.y + CHASER_HEIGHT}

		};

		Bounds2D spriteSize = {
			{-CHASER_WIDTH, -CHASER_HEIGHT},
			{CHASER_WIDTH, CHASER_HEIGHT}

		};

		Bounds2D uv = {
			{CHASER_START_U, CHASER_START_V},
			{CHASER_END_U, CHASER_END_V}
		};
		//hitbox / sprite boilerplate

		Animation* anim = animationNew(enemySprites, spriteSize, uv, CHASER_GAP, 0.5f, 3, 0);
		enemyInit(&c->e, &_chaserTable, target, pos, hit, anim, _chaserDelete, 100);

		c->e.obj.velocity.x = -CHASER_VELOCITY;
	}
	return &c->e;
}

static void _chaserUpdate(Object* obj, uint32_t milliseconds) {
	Chaser* c = (Chaser*)obj;
	//if the y distance is greater than our chase cutoff
	if (abs((int)obj->position.y - (int)c->e.target->position.y) > CHASE_THRESHOLD) {
		if (obj->position.y < c->e.target->position.y) {
			c->e.obj.velocity.y = CHASER_VELOCITY / 2.5f;
		}
		else {
			c->e.obj.velocity.y = -CHASER_VELOCITY / 2.5f;
		}
		//determine positive or negative based on relation to the player
	}

	else {
		c->e.obj.velocity.y = 0;
	}
	//if not in the cutoff, stop moving on the y
	//x velocity is constant no matter what, so it's never touched

	c->e.eTable->update(obj, milliseconds);
	//once we know what direction to move in, move

	if (c->e.obj.position.x < -CHASER_WIDTH) {
		c->e.shouldDelete = true;
	}
	//once we're totally offscreen (-width) despawn
}

//delete: no special memory for this, so it all gets freed in th enemy delete
static void _chaserDelete(Enemy* e) {
	return;
}

//draws enemy at current position
static void _chaserDraw(Object* obj) {
	Chaser* c = (Chaser*)obj;
	int spriteToDraw = -1;
	//init to an invalid sprite: will always get changed

	if (c->e.obj.velocity.y != 0) {
		if (c->e.obj.velocity.y > 0) {
			spriteToDraw = SPRITE_LOOK_DOWN;
		}
		else {
			spriteToDraw = SPRITE_LOOK_UP;
		}
	}
	else {
		spriteToDraw = SPRITE_LOOK_FORWARD;
	}
	//if above look down, if down look up, otherwise look forward

	animationDrawFrame(c->e.anim, obj->position, spriteToDraw);
}

//doesn't collide with anything, just with
static bool _chaserDoCollisions(Object* obj, Object* other) {
	return false;
}

//die when hit
static void _chaserCallback(Object* obj, uint8_t tag) {
	Chaser* c = (Chaser*)obj;
	c->e.eTable->callback(obj, tag);
	//enemy callback handles everything for this one, since it can't spawn a powerup
}

