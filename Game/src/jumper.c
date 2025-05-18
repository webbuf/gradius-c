#include "jumper.h"
#include "hitbox.h"

#include "random.h"

#define JUMPER_RADIUS  24
#define JUMPER_START_U 0.4957f
#define JUMPER_START_V 1 - 0.3358f
#define JUMPER_END_U   0.542f
#define JUMPER_END_V   1 - 0.4552f
#define JUMPER_GAP     0.0522f

#define JUMPER_POWER_START_U 0.7275f
#define JUMPER_POWER_END_U   0.7739f

#define JUMPER_INIT_Y_VELOCITY  -450
#define JUMPER_X_VELOCITY		-250
#define JUMPER_GRAVITY			1000
#define JUMPER_SHOOT_COOLDOWN   4000

typedef struct jumper_t {
	Enemy e;

	float currentVelocity;
	bool hasPowerup;
	uint32_t shootTimer;
	ProjectileManager* projSpawn;
	PowerupManager* powerSpawn;
}Jumper;

//static object functions
static void _jumperUpdate(Object* obj, uint32_t milliseconds);
static void _jumperDraw(Object* obj);
static bool _jumperDoCollisions(Object* obj, Object* other);
static void _jumperCallback(Object* obj, uint8_t);
static void _jumperDelete(Enemy* e);

static ObjVtable _shooterTable = {
	_jumperDraw,
	_jumperUpdate,
	ENEMY,
	_jumperDoCollisions,
	_jumperCallback
};

//makes jumper as position and tells it what it needs to know about, as well as if it has a powerup or not
Enemy* jumperNew(Coord2D pos, ProjectileManager* pm, PowerupManager* pum, Object* target, bool hasPowerup) {
	Jumper* j = malloc(sizeof(Jumper));
	if (j != NULL) {
		Coord2D vel = { JUMPER_X_VELOCITY, JUMPER_INIT_Y_VELOCITY };
		Bounds2D hitbox = {
			{pos.x - JUMPER_RADIUS, pos.y - JUMPER_RADIUS},
			{pos.x + JUMPER_RADIUS, pos.y + JUMPER_RADIUS},
		};

		Bounds2D spriteSize = {
			{-JUMPER_RADIUS, -JUMPER_RADIUS},
			{JUMPER_RADIUS, JUMPER_RADIUS},
		};

		Animation* anim;

		if (hasPowerup) {
			Bounds2D uv = {
				{JUMPER_POWER_START_U, JUMPER_START_V},
				{JUMPER_POWER_END_U, JUMPER_END_V},
			};

			anim = animationNew(enemySprites, spriteSize, uv, JUMPER_GAP, 0.5f, 4, 0);
		}
		else {
			Bounds2D uv = {
				{JUMPER_START_U, JUMPER_START_V},
				{JUMPER_END_U, JUMPER_END_V},
			};

			anim = animationNew(enemySprites, spriteSize, uv, JUMPER_GAP, 0.5f, 4, 0);
		}

		enemyInit(&j->e, &_shooterTable, target, pos, hitbox, anim, _jumperDelete, 100);
		j->powerSpawn = pum;
		j->projSpawn = pm;
		j->hasPowerup = hasPowerup;
		j->e.obj.velocity = vel;
		j->shootTimer = 0;
		j->currentVelocity = JUMPER_INIT_Y_VELOCITY;
	}
	return &j->e;
}

//moves jumper every frame
static void _jumperUpdate(Object* obj, uint32_t milliseconds) {
	Jumper* j = (Jumper*)obj;

	j->e.eTable->update(obj, milliseconds);
	j->currentVelocity += JUMPER_GRAVITY * ((float)milliseconds / 1000);
	j->e.obj.velocity.y = j->currentVelocity;
	j->shootTimer += milliseconds;
	//moves the jumper around and effects its velocity by the amount of milliseconds that pass

	animationUpdateTimer(j->e.anim, milliseconds);

	//if the jump cooldown is long enough, shoot the projectile and reset it
	if (j->shootTimer >= JUMPER_SHOOT_COOLDOWN) {
		j->shootTimer = 0;
		spawnProjectileEnemy(j->projSpawn, obj->position, j->e.target->position);
	}

	//delete offscreen
	if (obj->position.x < -JUMPER_RADIUS) {
		j->e.shouldDelete = true;
	}
}

//draws the jumper at the position
static void _jumperDraw(Object* obj) {
	Jumper* j = (Jumper*)obj;
	animationDraw(j->e.anim, obj->position);
}

//hey the jumper is the only enemy that collides with things! it hits the ground :)
static bool _jumperDoCollisions(Object* obj, Object* other) {
	if (other->vtable->tag != TERRAIN) {
		return false;
	}

	else {
		Jumper* j = (Jumper*)obj;
		bool didCollide = colliding(&obj->hitbox, &other->hitbox);
		if (didCollide) {
			j->currentVelocity = JUMPER_INIT_Y_VELOCITY;
			j->e.obj.velocity.y = JUMPER_INIT_Y_VELOCITY;

			int direction = randGetInt(0, 2);
			float xVelocity = JUMPER_X_VELOCITY;
			if (direction == 0 && obj->position.x < 500) {	//chooses a random direction to move in, but if we're on the right side of the screen go left so they don't just leave immediately
				xVelocity *= -1;
			}
			j->e.obj.velocity.x = xVelocity;

			updateHitbox(&j->e.obj.hitbox, 0, -10, 1000);
		}
		return didCollide;
		//if it hits the ground, reset the jump velocity and move it out of the ground
	}
}

//collision callback die and spawn powerup
static void _jumperCallback(Object* obj, uint8_t tag) {
	Jumper* j = (Jumper*)obj;

	if (j->hasPowerup) {
		spawnPowerup(j->powerSpawn, obj->position);
	}

	j->e.eTable->callback(obj, tag);
}

//doesn't need to do anything special to delete
static void _jumperDelete(Enemy* e) {
	return;
}