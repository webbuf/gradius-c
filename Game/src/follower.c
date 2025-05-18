#include <math.h>

#include "follower.h"

#define FOLLOWER_WIDTH    24
#define FOLLOWER_HEIGHT   15
#define FOLLOWER_VELOCITY 150

#define FOLLOWER_START_U  0.35f
#define FOLLOWER_START_V  1 - 0.363f
#define FOLLOWER_END_U	  0.4227f
#define FOLLOWER_END_V    1 - 0.437f
#define FOLLOWER_GAP	  0.0773f

#define START_FOLLOW_DISTANCE	 40.0f
#define STOP_FOLLOW_DISTANCE	 20.0f
//has a longer range to start following then stop so they aren't just in once place the whole time, rather they actually follow you

static void _followerUpdate(Object* obj, uint32_t milliseconds);
static void _followerDraw(Object* obj);
static float _getDistance(Coord2D v1, Coord2D v2);

static ObjVtable _followerTable = {
	_followerDraw,
	_followerUpdate,
	NOCOLLIDE,
	dummyCollide,
	dummyCallback
};

//makes a new follower that follows the target at the given position. has a set speed range to stay in pace with the player no matter what speed they're currently at
//target isn't always the player, could be another follow target
Follower* followerNew(Object* target, Coord2D pos, uint8_t speedRange){
	Follower* f = malloc(sizeof(Follower));
	if (f != NULL) {
		Coord2D vel = { 0, 0 };

		Bounds2D hitbox = {
			{pos.x - FOLLOWER_WIDTH, pos.y - FOLLOWER_HEIGHT},
			{pos.x + FOLLOWER_WIDTH, pos.y + FOLLOWER_HEIGHT}
		};

		objInit(&f->obj, &_followerTable, pos, vel, hitbox);

		f->followTarget = target;

		Bounds2D spriteSize = {
			{ -FOLLOWER_WIDTH, -FOLLOWER_HEIGHT},
			{FOLLOWER_WIDTH, FOLLOWER_HEIGHT}
		};

		Bounds2D uv = {
			{FOLLOWER_START_U, FOLLOWER_START_V},
			{FOLLOWER_END_U, FOLLOWER_END_V}
		};

		f->velocity = FOLLOWER_VELOCITY * (1 + (float) speedRange);
		f->anim = animationNew(playerSprites, spriteSize, uv, FOLLOWER_GAP, 0.5f, 2, 0);
		f->followY = false;
		f->followX = false;
		//these determine wheter to move in the x and y directions
	}
	return f;
}

//update function that follows the target. player handles shooting the projectiles
static void _followerUpdate(Object* obj, uint32_t milliseconds) {
	Follower* f = (Follower*)obj;
	animationUpdateTimer(f->anim, milliseconds);

	Coord2D resetVelocity = { 0, 0 };
	obj->velocity = resetVelocity;

	if (_getDistance(f->obj.position, f->followTarget->position) > START_FOLLOW_DISTANCE) {
		if (fabs(f->obj.position.y - f->followTarget->position.y) > (START_FOLLOW_DISTANCE / sqrt(2))) {
			f->followY = true;
		}

		if (fabs(f->obj.position.x - f->followTarget->position.x) > (START_FOLLOW_DISTANCE / sqrt(2))) {
			f->followX = true;
		}
	}
	//determines if we're far enough away in the x or y directions to start following in those directions
	//has an overall large follow distance, and then if that's good enough determines the specific directions it needs to go in

	if (f->followY) {
		if (fabs(f->obj.position.y - f->followTarget->position.y) < STOP_FOLLOW_DISTANCE){
			f->followY = false;
		}

		if (f->obj.position.y < f->followTarget->position.y) {
			obj->velocity.y = f->velocity;
		}
		else {
			obj->velocity.y = -f->velocity;
		}
	}

	if (f->followX) {
		if (fabs(f->obj.position.x - f->followTarget->position.x) < STOP_FOLLOW_DISTANCE) {
			f->followX = false;
		}

		if (f->obj.position.x < f->followTarget->position.x) {
			obj->velocity.x = f->velocity;
		}
		else {
			obj->velocity.x = -f->velocity;
		}
	}
	//chooses direction to go based on relationship to the follow target

	objDefaultUpdate(&f->obj, milliseconds);
}

//draws follower at given position
static void _followerDraw(Object* obj) {
	Follower* f = (Follower*)obj;
	animationDraw(f->anim, obj->position);
}

//frees follower and related memory
void followerDelete(Follower* f) {
	objDeinit(&f->obj);
	animationDelete(f->anim);

	free(f);
}

//distance formula
static float _getDistance(Coord2D v1, Coord2D v2) {
	double xPortion = pow(v2.x - v1.x, 2);
	double yPortion = pow(v2.y - v1.y, 2);

	return (float) sqrt(xPortion + yPortion);
}

//speeds up follower when the player gets a speed powerup to keep pace
void followerSpeedUp(Follower* f) {
	f->velocity += FOLLOWER_VELOCITY;
}