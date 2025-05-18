#include "sinner.h"

#include <math.h>

#define SINNER_RADIUS  24
#define SINNER_START_U 0.0203f
#define SINNER_START_V 1 - 0.3507f
#define SINNER_END_U   0.0667f
#define SINNER_END_V   1 - 0.4701f
#define SINNER_GAP     0.058f

#define SINNER_POWER_START_U 0.258f
#define SINNER_POWER_END_U   0.3043f

#define SINNER_VELOCITY  400
#define WAVELENGTH_MODIFIER 300
#define AMPLITUDE_MODIFIER  1.0f

typedef struct sinner_t {
	Enemy e;

	uint32_t timeAlive;
	bool hasPowerup;
	PowerupManager* spawn;
}Sinner;

static void _sinnerUpdate(Object* obj, uint32_t milliseconds);
static void _sinnerDraw(Object* obj);
static bool _sinnerDoCollisions(Object* obj, Object* other);
static void _sinnerCallback(Object* obj, uint8_t tag);
static void _sinnerDelete(Enemy* e);

static ObjVtable _sinnerTable = {
	_sinnerDraw,
	_sinnerUpdate,
	ENEMY,
	_sinnerDoCollisions,
	_sinnerCallback
};

//allocate sinner and everything it needs
Enemy* sinnerNew(Coord2D pos, Object* target, PowerupManager* pm, bool hasPowerup) {
	Sinner* s = malloc(sizeof(Sinner));
	if (s != NULL) {
		s->timeAlive = 0;
		s->hasPowerup = hasPowerup;
		s->spawn = pm;

		Bounds2D hit = { 
			{pos.x - SINNER_RADIUS, pos.y - SINNER_RADIUS},
			{pos.x + SINNER_RADIUS, pos.y + SINNER_RADIUS}
		};

		Bounds2D worldSize = {
			{-SINNER_RADIUS, -SINNER_RADIUS},
			{SINNER_RADIUS, SINNER_RADIUS}
		};

		Animation* anim = NULL;

		if (hasPowerup) {
			Bounds2D uv = {
				{SINNER_POWER_START_U, SINNER_START_V},
				{SINNER_POWER_END_U, SINNER_END_V}
			};

			anim = animationNew(enemySprites, worldSize, uv, SINNER_GAP, 0.5f, 4, 0);
		}

		else {
			Bounds2D uv = {
				{SINNER_START_U, SINNER_START_V},
				{SINNER_END_U, SINNER_END_V}
			};

			anim = animationNew(enemySprites, worldSize, uv, SINNER_GAP, 0.5f, 4, 0);
		}

		enemyInit(&s->e, &_sinnerTable, target, pos, hit, anim, _sinnerDelete, 100);
		s->e.obj.velocity.x = -SINNER_VELOCITY;
	}
	return &s->e;
}

//moves it
static void _sinnerUpdate(Object* obj, uint32_t milliseconds) {
	Sinner* s = (Sinner*)obj;
	s->timeAlive += milliseconds;
	animationUpdateTimer(s->e.anim, milliseconds);

	s->e.obj.velocity.y = (float) cos(s->timeAlive / WAVELENGTH_MODIFIER) * SINNER_VELOCITY * AMPLITUDE_MODIFIER;
	//calc: moves in a sin wave, so set y velocity to cos(x) since that's the derivative
	//also have a wavelength and amplitude so i can tweak the wave
	s->e.eTable->update(obj, milliseconds);
}

//draw animation at position
static void _sinnerDraw(Object* obj) {
	Sinner* s = (Sinner*)obj;
	animationDraw(s->e.anim, obj->position);
}

//no collide
static bool _sinnerDoCollisions(Object* obj, Object* other) {
	return false;
}

//die and spawn powerup
static void _sinnerCallback(Object* obj, uint8_t tag) {
	Sinner* s = (Sinner*)obj;

	if (s->hasPowerup) {
		spawnPowerup(s->spawn, obj->position);
	}

	s->e.eTable->callback(obj, tag);
}

//no special delete
static void _sinnerDelete(Enemy* e) {
	return;
}