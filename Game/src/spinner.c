#include "spinner.h"
#include "object.h"
#include "hitbox.h"

#include <stdlib.h>

#define SPINNER_RADIUS 19.5f

#define SPINNER_START_U 0.0174f
#define SPINNER_START_V 1 - 0.0373f
#define SPINNER_END_U	0.0551f
#define SPINNER_END_V	1 - 0.1343f
#define SPINNER_GAP		0.0609f

#define SPINNER_SPEED   450
#define SPINNER_SPEED_D 305

#define X_ONE_THIRD		341
#define TOTAL_X			1024
#define Y_HALF			384
#define TURN_THRESHOLD	5

typedef struct spinner_t {
	Enemy e;

	uint8_t state;
	SpinnerBuddy* tracker;
}Spinner;

typedef struct spinnerBuddy_t {
	uint8_t killedSpinners;
	uint8_t despawnedSpinners;

	PowerupManager* powerupSpawner;
}SpinnerBuddy;
//tracks if all 4 spinners in a group have died to spawn a powerup

enum spinnerStates {
	FORWARD,
	DIAGONAL,
	BACK
};
//spinners move through three phases while alive

static void _spinnerUpdate(Object* obj, uint32_t milliseconds);
static void _spinnerDraw(Object* obj);
static bool _spinnerDoCollisions(Object* obj, Object* other);
static void _spinnerDelete(Enemy* e);
static void _spinnerCallback(Object* obj, uint8_t tag);

static void _killSpinner(SpinnerBuddy* tracker, Coord2D pos);
static void _despawnSpinner(SpinnerBuddy* tracker);

static ObjVtable _spinnerTable = {
	_spinnerDraw,
	_spinnerUpdate,
	ENEMY,
	_spinnerDoCollisions,
	_spinnerCallback
};

//creates a spinner and lets it know about its buddy and target
Enemy* spinnerNew(Coord2D pos, SpinnerBuddy* track, Object* target) {
	Spinner* spin = malloc(sizeof(Spinner));
	if (spin != NULL) {
		spin->state = FORWARD;
		spin->tracker = track;

		Bounds2D hitbox = {
			{pos.x - SPINNER_RADIUS, pos.y - SPINNER_RADIUS},
			{pos.x + SPINNER_RADIUS, pos.y + SPINNER_RADIUS}
		};

		Bounds2D spinnerSize = {
			{-SPINNER_RADIUS, -SPINNER_RADIUS},
			{SPINNER_RADIUS, SPINNER_RADIUS}
		};

		Bounds2D spinnerUV = {
			{SPINNER_START_U, SPINNER_START_V},
			{SPINNER_END_U, SPINNER_END_V}
		};

		Animation* anim = animationNew(enemySprites, spinnerSize, spinnerUV, SPINNER_GAP, 0.1f, 3, 0);

		enemyInit(&spin->e, &_spinnerTable, target, pos, hitbox, anim, _spinnerDelete, 100);
		spin->e.obj.velocity.x = -SPINNER_SPEED;	//start in the forward state which does this
	}
	return &spin->e;
}

//make a tracker and set its count to nothing
SpinnerBuddy* spinnerBuddyNew(PowerupManager* spawner) {
	SpinnerBuddy* sb = malloc(sizeof(SpinnerBuddy));
	if (sb != NULL) {
		sb->despawnedSpinners = 0;
		sb->killedSpinners = 0;
		sb->powerupSpawner = spawner;
	}
	return sb;
}

//moves through three phases
static void _spinnerUpdate(Object* obj, uint32_t milliseconds) {
	Spinner* spin = (Spinner*)obj;
	animationUpdateTimer(spin->e.anim, milliseconds);
	switch (spin->state) {

	case FORWARD:
		if (obj->position.x < X_ONE_THIRD) {
			spin->state = DIAGONAL;
			obj->velocity.x = SPINNER_SPEED_D;
			if (obj->position.y < Y_HALF) {
				obj->velocity.y = SPINNER_SPEED_D;
			}
			else {
				obj->velocity.y = -SPINNER_SPEED_D;
			}
		}
		break;
		//moves to one third of screen and then goes up or down depending and where it is

	case DIAGONAL:
		if (abs((int) obj->position.y - (int) spin->e.target->position.y) < TURN_THRESHOLD) {
			obj->velocity.y = 0;
			obj->velocity.x = SPINNER_SPEED;
			spin->state = BACK;
		}
		break;
		//goes diagonal until they're levle with the player

	case BACK:
		if (obj->position.x > TOTAL_X + SPINNER_RADIUS) {
			spin->e.shouldDelete = true;
			return;
		}
		break;
		//turns around and despawns off the right

	default:
		break;
	}

	spin->e.eTable->update(obj, milliseconds);
}

//tell the spinner that this guy has despawned (but not died)
static void _spinnerDelete(Enemy* e) {
	_despawnSpinner(((Spinner*)e)->tracker);
}

//draw animation at the position
static void _spinnerDraw(Object* obj) {
	Spinner* spin = (Spinner*)obj;

	animationDraw(spin->e.anim, obj->position);
}

//no special for collisions
static bool _spinnerDoCollisions(Object* obj, Object* other) {
	return false;
}

//die and tell the spinner trakcer it's dead
static void _spinnerCallback(Object* obj, uint8_t tag) {
	Spinner* spin = (Spinner*)obj;
	_killSpinner(spin->tracker, obj->position);
	spin->e.eTable->callback(obj, tag);
}

//increment kill counter and spawn the powerup if it needs to
static void _killSpinner(SpinnerBuddy* tracker, Coord2D pos) {
	tracker->killedSpinners += 1;
	if (tracker->killedSpinners == 4) {
		spawnPowerup(tracker->powerupSpawner, pos);
	}
}

//increment despawn counter to see if we need to free the tracker
static void _despawnSpinner(SpinnerBuddy* tracker) {
	tracker->despawnedSpinners += 1;

	if (tracker->despawnedSpinners == 4) {
		free(tracker);
	}
}