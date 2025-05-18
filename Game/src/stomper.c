#include <math.h>

#include "stomper.h"
#include "object.h"

#define STOMPER_RADIUS  24
#define STOMPER_START_U 0.3536f
#define STOMPER_START_V 1 - 0.194f
#define STOMPER_END_U   0.4f
#define STOMPER_END_V   1 - 0.3134f
#define STOMPER_GAP	    0.0522f

#define STOMPER_POWER_START_U 0.5217f
#define STOMPER_POWER_END_U   0.5681f

#define STOMPER_VELOCITY	   250.0f
#define STOMPER_STOMP_DISTANCE 350
#define START_CHASE_CUTOFF	   100
#define SHOOT_COOLDOWN		   1000

#define SCREEN_HEIGHT		   768
#define SCROLL_SPEED		   -80

typedef struct stomper_t {
	Enemy e;

	float remainingDistance;
	uint32_t shootTimer;
	Sprite* shootingSprite;

	bool hasPowerup;
	ProjectileManager* projSpawn;
	PowerupManager* powerSpawn;
}Stomper;

static void _stomperUpdate(Object* obj, uint32_t milliseconds);
static void _stomperDraw(Object* obj);
static bool _stomperDoCollisions(Object* obj, Object* other);
static void _stomperCallback(Object* obj, uint8_t tag);
static void _stomperDelete(Enemy* e);

static ObjVtable _stomperTable = {
	_stomperDraw,
	_stomperUpdate,
	ENEMY,
	_stomperDoCollisions,
	_stomperCallback
};

//spawn a stomper at the position and let it know about everything it needs, including powerup
Enemy* stomperNew(Coord2D pos, ProjectileManager* pm, PowerupManager* pum, Object* target, bool hasPowerup) {
	Stomper* s = malloc(sizeof(Stomper));
	if (s != NULL) {
		Coord2D vel = { STOMPER_VELOCITY, 0 };

		Bounds2D hitbox = {
			{pos.x - STOMPER_RADIUS, pos.y - STOMPER_RADIUS},
			{pos.x + STOMPER_RADIUS, pos.y + STOMPER_RADIUS}
		};

		Bounds2D spriteSize = {
			{-STOMPER_RADIUS, -STOMPER_RADIUS},
			{STOMPER_RADIUS, STOMPER_RADIUS}
		};

		Animation* anim;

		if (hasPowerup) {
			Bounds2D uv = {
				{STOMPER_POWER_START_U, STOMPER_START_V},
				{STOMPER_POWER_END_U, STOMPER_END_V}
			};
			anim = animationNew(enemySprites, spriteSize, uv, STOMPER_GAP, 0.5f, 2, 0);

			Bounds2D shootingSpriteUV = {
				{STOMPER_POWER_START_U + 2 * STOMPER_GAP, STOMPER_START_V},
				{STOMPER_POWER_END_U + 2 * STOMPER_GAP, STOMPER_END_V}
			};

			s->shootingSprite = spriteNew(enemySprites, spriteSize, shootingSpriteUV, 0.5f);
		}

		else {
			Bounds2D uv = {
				{STOMPER_START_U, STOMPER_START_V},
				{STOMPER_END_U, STOMPER_END_V}
			};
			anim = animationNew(enemySprites, spriteSize, uv, STOMPER_GAP, 0.5f, 2, 0);

			Bounds2D shootingSpriteUV = {
				{STOMPER_START_U + 2 * STOMPER_GAP, STOMPER_START_V},
				{STOMPER_END_U + 2 * STOMPER_GAP, STOMPER_END_V}
			};

			s->shootingSprite = spriteNew(enemySprites, spriteSize, shootingSpriteUV, 0.5f);
		}

		enemyInit(&s->e, &_stomperTable, target, pos, hitbox, anim, _stomperDelete, 100);
		s->projSpawn = pm;
		s->powerSpawn = pum;
		s->hasPowerup = hasPowerup;
		s->remainingDistance = STOMPER_STOMP_DISTANCE;
		s->shootTimer = 0;

	}
	return &s->e;
}

//update with movement
static void _stomperUpdate(Object* obj, uint32_t milliseconds) {
	Stomper* s = (Stomper*)obj;
	if (s->remainingDistance <= 0) {	//if we aren't chasing the player (we're shooting)
		s->e.obj.velocity.x = SCROLL_SPEED;
		s->e.eTable->update(obj, milliseconds);
		s->shootTimer += milliseconds;
		//if we don't have a target, "stay still" (scroll at the scroll speed)
		if (s->shootTimer >= SHOOT_COOLDOWN) {
			//wait for the timer to go off, then shoot
			spawnProjectileEnemy(s->projSpawn, obj->position, s->e.target->position);
			s->shootTimer = 0;

			//after we shoot, check to see if we're too far from the player, and then walk toward them if we need to
			if (fabs(obj->position.x - s->e.target->position.x) > START_CHASE_CUTOFF) {
				s->remainingDistance = STOMPER_STOMP_DISTANCE;
				if (obj->position.x > s->e.target->position.x) {
					s->e.obj.velocity.x = -STOMPER_VELOCITY;
				}

				else {
					s->e.obj.velocity.x = STOMPER_VELOCITY;
				}
			}
		}
	}

	else {
		s->e.eTable->update(obj, milliseconds);
		animationUpdateTimer(s->e.anim, milliseconds);
		s->remainingDistance -= (float) fabs(STOMPER_VELOCITY * ((float)milliseconds / 1000));
	}
}

//draw the stomper at the location
static void _stomperDraw(Object* obj) {
	Stomper* s = (Stomper*)obj;
	bool onCeiling = obj->position.y < (SCREEN_HEIGHT / 2);
	bool playerToLeft = obj->position.x > s->e.target->position.x;

	if (s->remainingDistance <= 0) {
		spriteDrawInverted(s->shootingSprite, obj->position, playerToLeft, onCeiling);
	}

	else {
		animationDrawInverted(s->e.anim, obj->position, playerToLeft, onCeiling);
	}
}

//nothing
static bool _stomperDoCollisions(Object* obj, Object* other) {
	return false;
}

static void _stomperCallback(Object* obj, uint8_t tag) {
	Stomper* s = (Stomper*)obj;

	if (s->hasPowerup) {
		spawnPowerup(s->powerSpawn, obj->position);
	}

	s->e.eTable->callback(obj, tag);
}

//free the guy -> he has a sprite as well so free that
static void _stomperDelete(Enemy* e) {
	Stomper* s = (Stomper*)e;
	spriteDelete(s->shootingSprite);
}
