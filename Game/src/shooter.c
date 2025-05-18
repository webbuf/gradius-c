#include "shooter.h"
#include "object.h"

#include <math.h>
#include <stdio.h>

#define SHOOTER_WIDTH  21
#define SHOOTER_HEIGHT 22.5f

#define SHOOTER_U_START 0.4203f
#define SHOOTER_V_START 1 - 0.0299f
#define SHOOTER_U_END   0.4609f
#define SHOOTER_V_END   1 - 0.1418f
#define SHOOTER_GAP     0.0464f

#define SHOOTER_POWER_U_START 0.5739f
#define SHOOTER_POWER_U_END   0.6145f

#define SCROLL_SPEED  -80
#define SCREEN_HEIGHT 768

#define LOW_ANGLE_CUTOFF 20
#define MID_ANGLE_CUTOFF 60

#define SHOOT_TIMER 2500

typedef struct shooter_t {
	Enemy e;

	uint32_t shootCooldown;
	bool hasPowerup;
	ProjectileManager* projSpawn;
	PowerupManager* powerSpawn;
}Shooter;

static void _shooterUpdate(Object* obj, uint32_t milliseconds);
static void _shooterDraw(Object* obj);
static bool _shooterDoCollisions(Object* obj, Object* other);
static void _shooterCallback(Object* obj, uint8_t tag);
static void _shooterDelete(Enemy* e);

static ObjVtable _shooterTable = {
	_shooterDraw,
	_shooterUpdate,
	ENEMY,
	_shooterDoCollisions,
	_shooterCallback
};

enum shooterSprites {
	SPRITE_MID_ANGLE,
	SPRITE_HIGH_ANGLE,
	SPRITE_LOW_ANGLE
};
//enum to avoid confusion on specifying specific sprites

//initializes a new shooter with what it needs and if it has a powerup or not
Enemy* shooterNew(Coord2D pos, ProjectileManager* pm, PowerupManager* pum, Object* target, bool hasPowerup) {
	Shooter* s = malloc(sizeof(Shooter));
	if (s != NULL) {
		Coord2D vel = { SCROLL_SPEED, 0 };
		Bounds2D hitbox = {
			{pos.x - SHOOTER_WIDTH, pos.y - SHOOTER_HEIGHT},
			{pos.x + SHOOTER_WIDTH, pos.y + SHOOTER_HEIGHT},
		};

		Bounds2D spriteSize = {
			{-SHOOTER_WIDTH, -SHOOTER_HEIGHT},
			{SHOOTER_WIDTH, SHOOTER_HEIGHT},
		};

		Animation* anim;

		if (hasPowerup) {
			Bounds2D uv = {
				{SHOOTER_POWER_U_START, SHOOTER_V_START},
				{SHOOTER_POWER_U_END, SHOOTER_V_END},
			};

			anim = animationNew(enemySprites, spriteSize, uv, SHOOTER_GAP, 0.5f, 3, 0);
		}
		else {
			Bounds2D uv = {
				{SHOOTER_U_START, SHOOTER_V_START},
				{SHOOTER_U_END, SHOOTER_V_END},
			};

			anim = animationNew(enemySprites, spriteSize, uv, SHOOTER_GAP, 0.5f, 3, 0);
		}

		enemyInit(&s->e, &_shooterTable, target, pos, hitbox, anim, _shooterDelete, 100);
		s->powerSpawn = pum;
		s->projSpawn = pm;
		s->hasPowerup = hasPowerup;
		s->e.obj.velocity = vel;
		s->shootCooldown = 0;
	}
	return &s->e;
}

//moves and checks to see if it should shoot
static void _shooterUpdate(Object* obj, uint32_t milliseconds) {
	Shooter* s = (Shooter*)obj;

	s->e.eTable->update(obj, milliseconds);
	s->shootCooldown += milliseconds;

	if (s->shootCooldown >= SHOOT_TIMER) {
		s->shootCooldown = 0;
		spawnProjectileEnemy(s->projSpawn, obj->position, s->e.target->position);
	}
	//once timer's up, reset cooldown and shoot

	if (obj->position.x < -SHOOTER_WIDTH) {
		s->e.shouldDelete = true;
	}
	//delete offscreen
}

//draw at right angle given relationship to player
static void _shooterDraw(Object* obj) {
	Shooter* s = (Shooter*)obj;
	bool playerToLeft = false;
	if (s->e.target->position.x < obj->position.x) {
		playerToLeft = true;
	}

	bool playerBelow = false;
	if (obj->position.y < (SCREEN_HEIGHT / 2)) {
		playerBelow = true;
	}

	Coord2D enemyPos = obj->position;
	Coord2D playerPos = s->e.target->position;

	float angle = 0;
	if (playerBelow) {
		angle = atan2f(playerPos.y - enemyPos.y, playerPos.x - enemyPos.x);
	}

	else {
		angle = atan2f(enemyPos.y - playerPos.y, enemyPos.x - playerPos.x);
	}

	angle *= (180 / 3.141529f);

	if((playerBelow && playerToLeft) || (!playerBelow && !playerToLeft)){
		angle = (180 - angle);
	}
	//get angle to player between 0 and 90 degrees

	uint8_t frame = SPRITE_HIGH_ANGLE;

	if (angle < LOW_ANGLE_CUTOFF) {
		frame = SPRITE_LOW_ANGLE;
	}

	else if (angle < MID_ANGLE_CUTOFF) {
		frame = SPRITE_MID_ANGLE;
	}
	//choose sprite based on intensity of angle

	animationDrawFrameInverted(s->e.anim, obj->position, frame, playerToLeft, playerBelow);

}

//dummy
static bool _shooterDoCollisions(Object* obj, Object* other) {
	return false;
}

//die and spawn powerup
static void _shooterCallback(Object* obj, uint8_t tag) {
	Shooter* s = (Shooter*)obj;

	if (s->hasPowerup) {
		spawnPowerup(s->powerSpawn, obj->position);
	}

	s->e.eTable->callback(obj, tag);
}

//no special memory to free
static void _shooterDelete(Enemy* e) {
	return;
}