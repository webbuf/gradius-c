#include <stdlib.h>
#include <Windows.h>
#include <gl/GLU.h>
#include <assert.h>

#include "baseTypes.h"
#include "vicViper.h"
#include "field.h"
#include "input.h"
#include "projectiles.h"
#include "hitbox.h"
#include "sprite.h"
#include "animation.h"
#include "projectileManager.h"
#include "follower.h"
#include "soundInfo.h"

#define VIC_START_U 0.0273f
#define VIC_START_V 1 - 0.0444f	
#define VIC_END_U	0.1455f
#define VIC_END_V	1 - 0.1333f //v is bottom up, oops
//ship texture coordinates

#define VIC_SPRITE_GAP 0.1409f
//spritesheet is kind of a gallery - have to traverse negative space

#define VIC_SPAWN_X 100
#define VIC_SPAWN_Y 360

#define VIC_HITBOX_WIDTH  27
#define VIC_HITBOX_HEIGHT 12

#define VIC_SPRITE_WIDTH  39
#define VIC_SPRITE_HEIGHT 18
//hitbox and sprite aren't the same size
//these are divided by two for ease

#define VIC_STARTING_VELOCITY 150
#define FIRE_TIMER			  500
#define SPECIAL_FIRE_TIMER	  2000
#define MAX_POWERUP_LEVEL	  6
#define MAX_FOLLOWERS		  2
#define FOLLOWER_SPAWN_OFFSET 30
#define FOLLOWER_SHOOT_OFFSET 24

#define SHIELD_HEALTH		  3
#define SHIELD_WIDTH		  12
#define SHIELD_HEIGHT		  24
#define SHIELD_U_START		  0.5591f
#define SHIELD_V_START		  1 - 0.1926f
#define SHIELD_U_END		  0.5955f
#define SHIELD_V_END		  1 - 0.3111f
#define SHIELD_GAP		      0.05f
#define SHIELD_WEAK_U_START   0.7591f
#define SHIELD_WEAK_U_END     0.7955f

#define EXPLOSION_WIDTH  42
#define EXPLOSION_HEIGHT 22.5f

#define EXPLOSION_U_START  0.4318f
#define EXPLOSION_V_START  1 - 0.037f
#define EXPLOSION_U_END    0.5591f
#define EXPLOSION_V_END    1 - 0.1481f
#define EXPLOSION_GAP	   0.1363f
#define EXPLOSION_LIFETIME 332

typedef struct vic_t {
	Object obj;

	Bounds2D playArea;
	ProjectileManager* spawner;

	float baseVelocity;	//amount to change our velocity by to move - game has no acceleration

	bool firing;
	uint32_t fireTimer;	//allows for holding down fire key without shooting one million projectiles
	uint32_t specialFireTimer;

	PowerupInfo power;
	Follower** followers;

	uint8_t currentState;

	Animation* anim;
	Animation* shield;
	Animation* weakShield;

	Animation* deathExplosion;
	uint32_t deathTimer;
}VicViper;

static void _vicUpdate(Object* obj, uint32_t milliseconds);
static void _vicDraw(Object* obj);
static bool _vicDoCollisions(Object* obj, Object* other);
static void _vicStayInBounds(Object* obj);
static void _vicCallback(Object* obj, uint8_t tag);
static void _initPowerupInfo(PowerupInfo* pi, uint8_t startingPowerLevel);
static void _shootBase(VicViper* vic);
static void _shootSpecial(VicViper* vic);
static void _shootProjectile(VicViper* vic, uint8_t proj, Coord2D pos);
static void _vicTriggerCallback(int32_t index);
//declare private functions

static VicViperCB _playSound = NULL;

static ObjVtable _vicVtable = {
	_vicDraw,
	_vicUpdate,
	PLAYER,
	_vicDoCollisions,
	_vicCallback
};

//spawns player
VicViper* vicNew(Bounds2D level, ProjectileManager* pm, uint8_t startingPowerLevel) {
	assert(startingPowerLevel < 6);
	VicViper* vic = malloc(sizeof(VicViper));
	if (vic != NULL) {

		Bounds2D bounds = {
						{VIC_SPAWN_X - VIC_HITBOX_WIDTH, VIC_SPAWN_Y - VIC_HITBOX_HEIGHT},
						{VIC_SPAWN_X + VIC_HITBOX_WIDTH, VIC_SPAWN_Y + VIC_HITBOX_HEIGHT}
		};
		//spawns in same spot, always same size, so this can be handled entirely with defined constants

		Coord2D pos = boundsGetCenter(&bounds);
		Coord2D vel = { 0, 0 };
		objInit(&vic->obj, &_vicVtable, pos, vel, bounds);

		vic->playArea = level;
		vic->baseVelocity = VIC_STARTING_VELOCITY;	
		vic->firing = false;
		vic->spawner = pm;
		vic->followers = malloc(sizeof(Follower*) * MAX_FOLLOWERS);
		vic->currentState = VIC_STATE_ALIVE;
		for (int i = 0; i < MAX_FOLLOWERS; i++) {
			vic->followers[i] = NULL;
		}
		//create follower array and zero it out

		_initPowerupInfo(&vic->power, startingPowerLevel);

		Bounds2D vicSpriteSize = {
			{-VIC_SPRITE_WIDTH, -VIC_SPRITE_HEIGHT},
			{VIC_SPRITE_WIDTH, VIC_SPRITE_HEIGHT}
		};

		Bounds2D vicUV = { 
			{VIC_START_U, VIC_START_V},
			{VIC_END_U, VIC_END_V}
		};

		Bounds2D shieldSize = {
			{-SHIELD_WIDTH, -SHIELD_HEIGHT},
			{SHIELD_WIDTH, SHIELD_HEIGHT}
		};

		Bounds2D shieldUV = {
			{SHIELD_U_START, SHIELD_V_START},
			{SHIELD_U_END, SHIELD_V_END}
		};

		Bounds2D weakShieldUV = {
			{SHIELD_WEAK_U_START, SHIELD_V_START},
			{SHIELD_WEAK_U_END, SHIELD_V_END}
		};

		//gets animations for the forcefield

		vic->anim = animationNew(playerSprites, vicSpriteSize, vicUV, VIC_SPRITE_GAP, 0, 3, 0);
		vic->shield = animationNew(playerSprites, shieldSize, shieldUV, SHIELD_GAP, 0, 4, 0);
		vic->weakShield = animationNew(playerSprites, shieldSize, weakShieldUV, SHIELD_GAP, 0, 4, 0);

		Bounds2D explosionSize = {
			{-EXPLOSION_WIDTH, -EXPLOSION_HEIGHT},
			{EXPLOSION_WIDTH, EXPLOSION_HEIGHT}
		};

		Bounds2D explosionUV = {
			{EXPLOSION_U_START, EXPLOSION_V_START},
			{EXPLOSION_U_END, EXPLOSION_V_END}
		};

		vic->deathExplosion = animationNew(playerSprites, explosionSize, explosionUV, EXPLOSION_GAP, 0.5f, 4, 0);
		vic->deathTimer = 0;
	}
	return vic;
}

//free player memory and all related animations + followers
void vicDelete(VicViper* vic) {
	objDeinit(&vic->obj);

	animationDelete(vic->anim);
	animationDelete(vic->shield);
	animationDelete(vic->weakShield);
	animationDelete(vic->deathExplosion);
	for (int i = 0; i < MAX_FOLLOWERS; i++) {
		if (vic->followers[i] != NULL) {
			followerDelete(vic->followers[i]);
		}
	}
	free(vic->followers);
	free(vic);
}

//moves player and takes in input
static void _vicUpdate(Object* obj, uint32_t milliseconds) {
	VicViper* vic = (VicViper*)obj;
	if (vic->currentState == VIC_STATE_ALIVE) {
		float horizontalVelocity = 0;
		float verticalVelocity = 0;

		animationUpdateTimer(vic->shield, milliseconds);
		animationUpdateTimer(vic->weakShield, milliseconds);

		if (inputKeyPressed(VK_LEFT)) {
			horizontalVelocity -= vic->baseVelocity;
		}
		if (inputKeyPressed(VK_RIGHT)) {
			horizontalVelocity += vic->baseVelocity;
		}
		if (inputKeyPressed(VK_UP)) {
			verticalVelocity -= vic->baseVelocity;
		}
		if (inputKeyPressed(VK_DOWN)) {
			verticalVelocity += vic->baseVelocity;
		}
		//not elseif for a reason
		//alows to move diagonal and if you hold up + down or left + right, don't get any movement along that axis

		vic->specialFireTimer += milliseconds;

		if (vic->firing) {	//basically if we were pressing z last frame
			if (inputKeyPressed('Z')) {
				vic->fireTimer += milliseconds;	//get another frame closer to shooting
				if (vic->fireTimer >= FIRE_TIMER) {	//been 30 frames since last shot
					_shootBase(vic);
					//spawn projectile, reset timer
				}
				if (vic->specialFireTimer > SPECIAL_FIRE_TIMER) {
					_shootSpecial(vic);
				}
				//just calls the functions, we use the powerup info in the function to choose what to actually shoot
			}

			else {
				vic->firing = false;	//if we stopped holding z, flip the bool to false
			}

		}

		else {
			if (inputKeyPressed('Z')) {
				_shootBase(vic);
				if (vic->specialFireTimer > SPECIAL_FIRE_TIMER) {
					_shootSpecial(vic);
				}
			}
			//pressing down z spawns projectile, puts us in hold mode
			//wait, doesn't this mean you can spam z to shoot faster?
			//that's called tech. and it's in the real game
		}

		//banks and activates powerup on x press
		if (inputKeyPressed('X')) {
			if (vic->power.powerupLevel != 0) {
				_vicTriggerCallback(SOUND_BANK_POWERUP);
				switch (vic->power.powerupLevel) {
				case 1:
					vic->baseVelocity += VIC_STARTING_VELOCITY;
					vic->power.speedLevel += 1;
					for (int i = 0; i < MAX_FOLLOWERS; i++) {
						if (vic->followers[i] != NULL) {
							followerSpeedUp(vic->followers[i]);
						}
						//speed up followers to keep pace
					}
					break;

				case 2:
					vic->power.hasMissile = true;
					break;
					//just set to true
				case 3:
					vic->power.hasDouble = true;
					if (vic->power.hasLaser == true) {
						vic->power.hasLaser = false;
					}
					break;

				case 4:
					vic->power.hasLaser = true;
					if (vic->power.hasDouble == true) {
						vic->power.hasDouble = false;
					}
					break;
					//mutually exclusive, so wipe the other once

				case 5:
					if (vic->power.followerCount < MAX_FOLLOWERS) {	//only let you do it if you have less than two
						vic->power.followerCount += 1;
						if (vic->followers[0] == NULL) {
							Coord2D followerSpawnPos = obj->position;
							followerSpawnPos.x -= FOLLOWER_SPAWN_OFFSET;
							vic->followers[0] = followerNew(obj, followerSpawnPos, vic->power.speedLevel);
							//if we have no followers, make one and have it follow you
						}
						else if (vic->followers[1] == NULL) {
							Coord2D followerSpawnPos = vic->followers[0]->obj.position;
							followerSpawnPos.x -= FOLLOWER_SPAWN_OFFSET;
							vic->followers[1] = followerNew(&vic->followers[0]->obj, followerSpawnPos, vic->power.speedLevel);
							//if we have a follower, make one and have it follow the first one
						}
					}
					break;
					//can't ever lose them so having it be this hardcoded is fine

				case 6:
					vic->power.shieldHealth = SHIELD_HEALTH;
					break;

				default:
					break;
				}
				vic->power.powerupLevel = 0;
			}
		}

		obj->velocity.x = horizontalVelocity;
		obj->velocity.y = verticalVelocity;

		objDefaultUpdate(obj, milliseconds);

		_vicStayInBounds(&vic->obj);
		//move
	}
	
	else if(vic->currentState == VIC_STATE_DYING){
		animationUpdateTimer(vic->deathExplosion, milliseconds);
		vic->deathTimer += milliseconds;
		if (vic->deathTimer >= EXPLOSION_LIFETIME) {
			vic->currentState = VIC_STATE_DEAD;
		}
		//if we're dying, update the explosion animation
	}
}

//collide with the guys
static bool _vicDoCollisions(Object* obj, Object* other) {
	VicViper* vic = (VicViper*)obj;
	if (vic->currentState == VIC_STATE_ALIVE) {
		if (other->vtable->tag == ENEMY || other->vtable->tag == BOSS || other->vtable->tag == ENEMY_PROJECTILE) {
			bool didCollide = colliding(&obj->hitbox, &other->hitbox);

			if (didCollide) {
				if (vic->power.shieldHealth > 0) {
					vic->power.shieldHealth -= 1;
				}
				else {
					_vicTriggerCallback(SOUND_PLAYER_DIE);
					vic->currentState = VIC_STATE_DYING;
				}
			}
			//if we hit a meanie, lose a health if we have a shield and die otherwise
			return didCollide;
		}

		//if we hit a powerup, increase our level by one
		else if (other->vtable->tag == POWERUP) {
			bool didCollide = colliding(&obj->hitbox, &other->hitbox);

			if (didCollide) {
				vic->power.powerupLevel = min(vic->power.powerupLevel + 1, MAX_POWERUP_LEVEL);
			}

			return didCollide;
		}

		//if we hit the wall you just fucking die
		else if (other->vtable->tag == TERRAIN) {
			bool didCollide = colliding(&obj->hitbox, &other->hitbox);

			if (didCollide) {
				_vicTriggerCallback(SOUND_PLAYER_DIE);
				vic->currentState = VIC_STATE_DYING;
			}

			return didCollide;
		}

		else {
			return false;
		}
	}
	else {
		return false;
	}
}

//pushes player back in bounds
static void _vicStayInBounds(Object* obj) {
	VicViper* vic = (VicViper*)obj;

	float topLeftX = obj->hitbox.topLeft.x;
	float topLeftY = obj->hitbox.topLeft.y;
	float botRightX = obj->hitbox.botRight.x;
	float botRightY = obj->hitbox.botRight.y;

	if (topLeftY < vic->playArea.topLeft.y) {
		updateHitbox(&obj->hitbox, 0, vic->playArea.topLeft.y - topLeftY, 1000);
		//need to correct by the amount we're outside of the bounds, and remove the movement we did
	}
	
	if (topLeftX < vic->playArea.topLeft.x) {
		updateHitbox(&obj->hitbox, vic->playArea.topLeft.x - topLeftX, 0, 1000);
	}
	if (botRightY > vic->playArea.botRight.y) {
		updateHitbox(&obj->hitbox, 0, vic->playArea.botRight.y - botRightY, 1000);
	}
	if (botRightX > vic->playArea.botRight.x) {
		updateHitbox(&obj->hitbox, vic->playArea.botRight.x - botRightX, 0, 1000);
	}
	//case for every side of the screen
	//later for accuracy getting too high or too low makes you explode
}

//draw da player
static void _vicDraw(Object* obj) {
	VicViper* vic = (VicViper*)obj;

	//drawHitbox(&obj->hitbox);
	//draws our hitbox
	if (vic->currentState == VIC_STATE_ALIVE) {
		GLuint frame = 0;

		if (obj->velocity.y > 0) {
			frame = 1;
		}

		else if (obj->velocity.y < 0) {
			frame = 2;
		}
		//if we're alive, change the current frame to the one that looks like you need it

		animationDrawFrame(vic->anim, obj->position, frame);
		if (vic->power.shieldHealth > 0) {
			Coord2D shieldPos = obj->position;
			shieldPos.x += VIC_SPRITE_WIDTH + SHIELD_WIDTH;
			if (vic->power.shieldHealth > 1) {
				animationDraw(vic->shield, shieldPos);
			}
			else {
				animationDraw(vic->weakShield, shieldPos);
			}
		}
		//draw the shield if we have it
	}

	else if (vic->currentState == VIC_STATE_DYING) {
		animationDraw(vic->deathExplosion, obj->position);
	}
	//if we're dying, draw the explosion
}

//set powerup info to having nothing
static void _initPowerupInfo(PowerupInfo* pi, uint8_t startingPowerLevel) {
	pi->hasMissile = false;
	pi->hasDouble = false;
	pi->hasLaser = false;
	pi->followerCount = 0;
	pi->shieldHealth = 0;
	pi->powerupLevel = startingPowerLevel;
	pi->speedLevel = 0;
}

//getters
Coord2D getPosition(VicViper* vic) {
	return vic->obj.position;
}

static void _vicCallback(Object* obj, uint8_t tag) {
	return;
}

PowerupInfo* getPowerupInfo(VicViper* vic) {
	return &vic->power;
}

//if we have base shoot base, otherwise shoot projectile
static void _shootBase(VicViper* vic) {
	Coord2D firingPos = { vic->obj.position.x + VIC_SPRITE_WIDTH, vic->obj.position.y };
	uint8_t proctileToFire = 0;
	if (vic->power.hasLaser) {
		_shootProjectile(vic, PROJ_LASER, firingPos);
	}
	else{
		_shootProjectile(vic, PROJ_BASE, firingPos);
	}
	vic->fireTimer = 0;
	vic->firing = true;
}

//can do both of these if we have both. also they have special spawns
static void _shootSpecial(VicViper* vic) {
	if (vic->power.hasMissile) {
		Coord2D missilePos = { vic->obj.position.x, vic->obj.position.y + VIC_SPRITE_HEIGHT };
		_shootProjectile(vic, PROJ_MISSILE, missilePos);
	}

	if (vic->power.hasDouble) {
		Coord2D doublePos = { vic->obj.position.x + (VIC_SPRITE_WIDTH / 2), vic->obj.position.y - VIC_SPRITE_HEIGHT};
		_shootProjectile(vic, PROJ_DOUBLE, doublePos);
	}
	vic->specialFireTimer = 0;
}

//loop through our followers and spawn projectiles there too if we need to
static void _shootProjectile(VicViper* vic, uint8_t proj, Coord2D pos) {
	assert(proj < 4); //always spawn a valid projectile
	spawnProjectile(vic->spawner, proj, pos);
	for (int i = 0; i < MAX_FOLLOWERS; i++) {
		if (vic->followers[i] != NULL) {
			Coord2D followerPos = vic->followers[i]->obj.position;
			followerPos.x += FOLLOWER_SHOOT_OFFSET;
			spawnProjectile(vic->spawner, proj, followerPos);
		}
	}
}

//more getters
Object* vicGetObject(VicViper* vic) {
	return &vic->obj;
}

uint8_t vicGetState(VicViper* vic) {
	return vic->currentState;
}

void vicSetCB(VicViperCB callback) {
	_playSound = callback;
}

void vicClearCB() {
	_playSound = NULL;
}

static void _vicTriggerCallback(int32_t index) {
	if (_playSound != NULL) {
		_playSound(index);
	}
}