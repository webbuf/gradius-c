#include <assert.h>

#include "boss.h"
#include "enemy.h"
#include "hitbox.h"
#include "projectiles.h"
#include "scoreManager.h"

#define BOSS_SPRITE_WIDTH 72
#define BOSS_HITBOX_WIDTH 36
#define BOSS_HEIGHT		  72
#define BOSS_U_START      0.0474f
#define BOSS_V_START	  1 - 0.449f
#define BOSS_U_END        0.2543f
#define BOSS_V_END        1 - 0.9388f
//every object has sprite and size defines like this

#define BOSS_BASE_HEALTH     100
#define BOSS_Y_VELOCITY      200
#define BOSS_SPAWN_VELOCITY -120
//velocity defines also common

#define SPAWN_CUTOFF		 800
#define TOP_Y_BOUNCE		 150
#define BOTTOM_Y_BOUNCE		 580
//screen space bounds for changing its movement

#define BOSS_INNER_PROJ_X_OFFSET  81
#define BOSS_INNER_PROJ_Y_OFFSET  21
#define BOSS_OUTER_PROJ_X_OFFSET  42
#define BOSS_OUTER_PROJ_Y_OFFSET  60
//spawn offsets from center of sprite for projectiles

typedef struct boss_t {
	Enemy e;

	uint8_t state;	//spawning or fighting
	uint8_t health;
	ProjectileManager* spawn;	//anything that can spawn a projectile has a pointer to the manager to tell it to spawn
} Boss;

enum bossStates {
	BOSS_SPAWNING,
	BOSS_ACTIVE
};

//static object funcs
static void _bossUpdate(Object* obj, uint32_t milliseconds);
static void _bossDraw(Object* obj);
static bool _bossDoCollisions(Object* obj, Object* other);
static void _bossCallback(Object* obj, uint8_t tag);
static void _dummyDelete(Enemy* e);

//spawn projectiles
static void _bossShoot(Boss* b);


static ObjVtable _bossTable = {
	_bossDraw,
	_bossUpdate,
	BOSS,
	_bossDoCollisions,
	_bossCallback
};

//spawns a new boss at the position, given the projectile manager and a pointer to the player
Boss* bossNew(ProjectileManager* pm, Coord2D pos, Object* target) {
	assert(target != NULL);
	Boss* b = malloc(sizeof(Boss));
	if (b != NULL) {
		b->health = BOSS_BASE_HEALTH;
		b->spawn = pm;
		b->state = BOSS_SPAWNING;

		Coord2D vel = { BOSS_SPAWN_VELOCITY, 0 };
		//at spawn we're moving forward

		Bounds2D hitbox = {
			{pos.x - BOSS_HITBOX_WIDTH, pos.y - BOSS_HEIGHT},
			{pos.x + BOSS_HITBOX_WIDTH, pos.y + BOSS_HEIGHT}
		};

		Bounds2D spriteSize = {
			{-BOSS_SPRITE_WIDTH, -BOSS_HEIGHT},
			{BOSS_SPRITE_WIDTH, BOSS_HEIGHT}
		};

		Bounds2D uv = {
			{BOSS_U_START, BOSS_V_START},
			{BOSS_U_END, BOSS_V_END}
		};
		//boilerplate setting up hitboxes and sprite information

		Animation* anim = animationNew(bossSprites, spriteSize, uv, 0, 0.5f, 1, 0);

		enemyInit(&b->e, &_bossTable, target, pos, hitbox, anim, _dummyDelete, 10000);
		//enemy init is given a dummy delete function just because something needs to go in the slot
		//because the game ends when the boss dies it's tracked differently, so the enemy delete doesn't get called on it
		//but since it inherits from enemy it needs to have on

		b->e.obj.velocity = vel;
		//tbh i'm not 100% sure why i didn't let the enemy init take a velocity

		b->e.eTable->tag = BOSS;
		//enemy table tags default to enemy, so in this case we need to change it to boss
	}
	return b;
}

//boss behavior every frame
static void _bossUpdate(Object* obj, uint32_t milliseconds) {
	Boss* b = (Boss*)obj;
	b->e.eTable->update(obj, milliseconds);
	//move

	if (b->state == BOSS_SPAWNING) {
		if (obj->position.x <= SPAWN_CUTOFF) {
			b->state = BOSS_ACTIVE;
			obj->velocity.x = 0;
			obj->velocity.y = BOSS_Y_VELOCITY;
		}
		//once we hit the spawn cutoff, go into battle mode
	}

	if (b->state == BOSS_ACTIVE) {
		if (obj->position.y <= TOP_Y_BOUNCE) {
			_bossShoot(b);
			obj->velocity.y = BOSS_Y_VELOCITY;
		}

		else if (obj->position.y >= BOTTOM_Y_BOUNCE) {
			_bossShoot(b);
			obj->velocity.y = -BOSS_Y_VELOCITY;
		}
		//we don't need to track whether we're moving up or down, just to reverse if we hit our bounds on either side
		//hardcode directions instead of *= -1 to never have a situation where it's bouncing up and down
		//that shouldn't ever happen anyway since it moves at a constant velocity, but better safe than sorry
	}
}

//draws the boss
static void _bossDraw(Object* obj) {
	Boss* b = (Boss*)obj;
	animationDrawFrame(b->e.anim, obj->position, 0);
	//only has one frame, using this due to enemy architecture requiring an anim
}

//doesn't collide with anything itself, but needed to game structure
static bool _bossDoCollisions(Object* obj, Object* other) {
	return false;
}

//boss behavior for when it's collided with
static void _bossCallback(Object* obj, uint8_t tag) {
	if (tag == PROJECTILE) {
		Boss* b = (Boss *) obj;
		b->health -= 1;	//decrease health by one per projectile it's hit by
		if (b->health <= 0) {	//since we decrement by 1 i don't think we can ever skip 0 but why add the possibility
			b->e.shouldDelete = true;
			increaseScore(scoreCounter, b->e.scoreValue);	//set score in here rather than in delete because otherwise it would increment score on despawn even if you didn't kill it
		}
	}
}

//deletes the boss. has no special dynamic memory, so just call the normal enemy delete on it
void bossDelete(Boss* b) {
	enemyDelete(&b->e);
}

//spawns the boss's projectile at its current position
static void _bossShoot(Boss* b) {
	Coord2D pos = b->e.obj.position;	//get current position
	pos.x -= BOSS_INNER_PROJ_X_OFFSET;
	pos.y += BOSS_INNER_PROJ_Y_OFFSET;

	spawnProjectile(b->spawn, PROJ_BOSS, pos);

	pos.y -= 2 * BOSS_INNER_PROJ_Y_OFFSET;	//mirror around the center

	spawnProjectile(b->spawn, PROJ_BOSS, pos);

	pos = b->e.obj.position;	//reset to now move by the inner offset
	pos.x -= BOSS_OUTER_PROJ_X_OFFSET;
	pos.y += BOSS_OUTER_PROJ_Y_OFFSET;

	spawnProjectile(b->spawn, PROJ_BOSS, pos);

	pos.y -= 2*BOSS_OUTER_PROJ_Y_OFFSET;	//mirror

	spawnProjectile(b->spawn, PROJ_BOSS, pos);
}

//getter for observer to know if it should delete
bool bossShouldDelete(Boss* b) {
	return b->e.shouldDelete;
}

//relic of architecture: boss gets deleted differently because it's related to game flow but this needs to be here
void _dummyDelete(Enemy* e) {
	return;
}