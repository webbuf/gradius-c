#include <Windows.h>
#include <gl/GLU.h>
#include <assert.h>
#include <math.h>

#include "projectiles.h"
#include "SOIL.h"
#include "object.h"
#include "shape.h"
#include "hitbox.h"
#include "sprite.h"
#include "animation.h"

#define BASE_PROJECTILE_VELOCITY 1400
#define BASE_PROJECTILE_START_U  0.1591f
#define BASE_PROJECTILE_END_U    0.1955f
#define BASE_PROJECTILE_START_V  1 - 0.3926f
#define BASE_PROJECTILE_END_V    1 - 0.4148f
#define BASE_PROJECTILE_WIDTH	 15
#define BASE_PROJECTILE_HEIGHT   5
//use for determining collision

#define MISSILE_VELOCITY	 100
#define MISSILE_Y_VELOCITY	 400
#define MISSILE_END_VELOCITY 500
#define MISSILE_START_U		 0.0409f
#define MISSILE_START_V		 1 - 0.363f
#define MISSILE_END_U		 0.0773f
#define MISSILE_END_V		 1 - 0.4222f
#define MISSILE_GAP			 0.0591f
#define MISSILE_SIZE		 12

#define DOUBLE_VELOCITY		700
#define DOUBLE_START_U		0.2136f
#define DOUBLE_START_V		1 - 0.3778f
#define DOUBLE_END_U		0.2455f
#define DOUBLE_END_V		1 - 0.4296f
#define DOUBLE_SIZE			12

#define LASER_VELOCITY		1800
#define LASER_START_U		0.2636f
#define LASER_START_V		1 - 0.3926f
#define LASER_END_U			0.3364f
#define LASER_END_V			1 - 0.4074f
#define LASER_WIDTH			24
#define LASER_HEIGHT		3

#define BOSS_PROJ_WIDTH		24
#define BOSS_PROJ_HEIGHT    6
#define BOSS_PROJ_START_U   0.3405f
#define BOSS_PROJ_START_V   1 - 0.4898f
#define BOSS_PROJ_END_U     0.4095f
#define BOSS_PROJ_END_V     1 - 0.5306f
#define BOSS_PROJ_VELOCITY  -380

#define ENEMY_PROJ_RADIUS		 6
#define ENEMY_PROJ_START_U		 0.8464f
#define ENEMY_PROJ_START_V		 1 - 0.2313f
#define ENEMY_PROJ_END_U		 0.8638f
#define ENEMY_PROJ_END_V		 1 - 0.2761f
#define ENEMY_PROJ_GAP			 0.0261f
#define ENEMY_PROJ_BASE_VELOCITY 250

typedef void (*ProjectileDeleteFunction)(Projectile*);

typedef struct proj_t {
	Object obj;

	Bounds2D hitbox;
	ObjVtable* ptable;

	bool shouldDelete;
	ProjectileDeleteFunction deleteFunction;
	//we don't want to delete inside the projectile update itself, since we don't know the exact structure for the delete if we're in a parent class
	//so we mark it for delete and then delete inside the child
}Projectile;

typedef struct baseProj_t {
	Projectile proj;

	Sprite* sprite;
	bool hitBoss;
}BaseProjectile;
//this is basically just a projectile, but I don't want to ever have a raw projectile in the game

typedef struct missile_t {
	Projectile proj;

	Animation* anim;
	Bounds2D levelBounds;
	bool onGround;
}Missile;

typedef struct enemyProj_t {
	Projectile proj;

	Animation* anim;
}EnemyProjectile;

//all the projectiles are in this one file instead of it being spread out
static void _projUpdate(Object* obj, uint32_t milliseconds);
static void _projDraw(Object* obj);
static bool _projDoCollisions(Object* obj, Object* other);
static void _projCallback(Object* obj, uint8_t tag);

static void _BPUpdate(Object* obj, uint32_t milliseconds);
static void _BPDraw(Object* obj);
static void _BPDelete(Projectile* proj);

static void _missileUpdate(Object* obj, uint32_t milliseconds);
static void _missileDraw(Object* obj);
static void _missileDelete(Projectile* proj);
static bool _missileDoCollisions(Object* obj, Object* other);

static bool _laserDoCollisions(Object* obj, Object* other);

static void _enemyProjUpdate(Object* obj, uint32_t milliseconds);
static void _enemyProjDraw(Object* obj);
static void _enemyProjDelete(Projectile* proj);
static void _enemyProjCallback(Object* obj, uint8_t tag);

static void _bossProjCallback(Object* obj, uint8_t tag);

static ObjVtable _projVtable = {
	_projDraw,
	_projUpdate,
	PROJECTILE,
	_projDoCollisions,
	_projCallback
};

static ObjVtable _BPVtable = {
	_BPDraw,
	_BPUpdate,
	PROJECTILE,
	_projDoCollisions,
	_projCallback
};

static ObjVtable _missileTable = {
	_missileDraw,
	_missileUpdate,
	PROJECTILE,
	_missileDoCollisions,
	_projCallback
};

static ObjVtable _laserTable = {
	_BPDraw,
	_BPUpdate,
	PROJECTILE,
	_laserDoCollisions,
	_projCallback
};

static ObjVtable _enemyProjTable = {
	_enemyProjDraw,
	_enemyProjUpdate,
	ENEMY_PROJECTILE,
	dummyCollide,
	_enemyProjCallback
};

static ObjVtable _bossProjTable = {
	_BPDraw,
	_BPUpdate,
	ENEMY_PROJECTILE,
	dummyCollide,
	_bossProjCallback
};
//vtables giving the specific functions that they need

//spawn base projectile at the position
Projectile* baseProjectileNew(Coord2D pos) {
	BaseProjectile* baseProj = malloc(sizeof(BaseProjectile));
	if (baseProj != NULL) {
		Coord2D vel = { BASE_PROJECTILE_VELOCITY, 0 };	//always just move forward at a constant speed

		Bounds2D hitbox = {
			{pos.x - BASE_PROJECTILE_WIDTH, pos.y - BASE_PROJECTILE_HEIGHT},
			{pos.x + BASE_PROJECTILE_WIDTH, pos.y + BASE_PROJECTILE_HEIGHT}
		};

		objInit(&baseProj->proj.obj, &_BPVtable, pos, vel, hitbox);

		baseProj->proj.obj.position = pos;
		baseProj->proj.obj.velocity = vel;
		baseProj->proj.ptable = &_projVtable;
		baseProj->proj.shouldDelete = false;	//defaults to true oops
		baseProj->proj.deleteFunction = _BPDelete;

		baseProj->proj.hitbox = hitbox;
		baseProj->hitBoss = false;

		//having a hitbox for this might be redundant because the sprite is the same size
		//but I want my collision system to be consistent, which means everything is gonna get a 
		//bounding box. I think the unoptimization of moving 4 floats an enemy is a hit we can take

		Bounds2D spriteSize = {
			{-BASE_PROJECTILE_WIDTH, -BASE_PROJECTILE_HEIGHT},
			{BASE_PROJECTILE_WIDTH, BASE_PROJECTILE_HEIGHT}
		};

		Bounds2D spriteUV = {
			{BASE_PROJECTILE_START_U, BASE_PROJECTILE_START_V},
			{BASE_PROJECTILE_END_U, BASE_PROJECTILE_END_V}
		};

		baseProj->sprite = spriteNew(playerSprites, spriteSize, spriteUV, 0.5f);
	}
	return &baseProj->proj;
}

//spawn double: same fields as a base but it has a new sprite and moves on 2 axes
Projectile* doubleNew(Coord2D pos) {
	BaseProjectile* baseProj = malloc(sizeof(BaseProjectile));
	if (baseProj != NULL) {
		Coord2D vel = { DOUBLE_VELOCITY, -DOUBLE_VELOCITY };	//always just move forward at a constant speed

		Bounds2D hitbox = {
			{pos.x - DOUBLE_SIZE, pos.y - DOUBLE_SIZE},
			{pos.x + DOUBLE_SIZE, pos.y + DOUBLE_SIZE}
		};

		objInit(&baseProj->proj.obj, &_BPVtable, pos, vel, hitbox);

		baseProj->proj.obj.position = pos;
		baseProj->proj.obj.velocity = vel;
		baseProj->proj.ptable = &_projVtable;
		baseProj->proj.shouldDelete = false;	//defaults to true oops
		baseProj->proj.deleteFunction = _BPDelete;

		baseProj->proj.hitbox = hitbox;
		baseProj->hitBoss = false;

		//having a hitbox for this might be redundant because the sprite is the same size
		//but I want my collision system to be consistent, which means everything is gonna get a 
		//bounding box. I think the unoptimization of moving 4 floats an enemy is a hit we can take

		Bounds2D spriteSize = {
			{-DOUBLE_SIZE, -DOUBLE_SIZE},
			{DOUBLE_SIZE, DOUBLE_SIZE}
		};

		Bounds2D spriteUV = {
			{DOUBLE_START_U, DOUBLE_START_V},
			{DOUBLE_END_U, DOUBLE_END_V}
		};

		baseProj->sprite = spriteNew(playerSprites, spriteSize, spriteUV, 0.5f);
	}
	return &baseProj->proj;

}

//new missile - different struct because it has states
Projectile* missileNew(Coord2D pos, Bounds2D level) {
	Missile* m = malloc(sizeof(Missile));
	if (m != NULL) {
		Coord2D vel = { MISSILE_VELOCITY, MISSILE_Y_VELOCITY };	//always just move forward at a constant speed

		Bounds2D hitbox = {
			{pos.x - MISSILE_SIZE, pos.y - MISSILE_SIZE},
			{pos.x + MISSILE_SIZE, pos.y + MISSILE_SIZE}
		};

		objInit(&m->proj.obj, &_missileTable, pos, vel, hitbox);

		m->proj.obj.position = pos;
		m->proj.obj.velocity = vel;
		m->proj.ptable = &_projVtable;
		m->proj.shouldDelete = false;	//defaults to true oops
		m->proj.deleteFunction = _missileDelete;

		m->proj.hitbox = hitbox;

		//having a hitbox for this might be redundant because the sprite is the same size
		//but I want my collision system to be consistent, which means everything is gonna get a 
		//bounding box. I think the unoptimization of moving 4 floats an enemy is a hit we can take

		Bounds2D spriteSize = {
			{-MISSILE_SIZE, -MISSILE_SIZE},
			{MISSILE_SIZE, MISSILE_SIZE}
		};

		Bounds2D spriteUV = {
			{MISSILE_START_U, MISSILE_START_V},
			{MISSILE_END_U, MISSILE_END_V}
		};

		m->anim = animationNew(playerSprites, spriteSize, spriteUV, MISSILE_GAP, 0.5f, 2, 0);
		m->levelBounds = level;
		m->onGround = false;
	}
	return &m->proj;
}

//new laser : also a base projectile. it has a different collision function to change its behavior
Projectile* laserNew(Coord2D pos) {
	BaseProjectile* l = malloc(sizeof(BaseProjectile));
	if (l != NULL) {
		Coord2D vel = { LASER_VELOCITY, 0 };	//always just move forward at a constant speed

		Bounds2D hitbox = {
			{pos.x - LASER_WIDTH, pos.y - LASER_HEIGHT},
			{pos.x + LASER_WIDTH, pos.y + LASER_HEIGHT}
		};

		objInit(&l->proj.obj, &_laserTable, pos, vel, hitbox);

		l->proj.obj.position = pos;
		l->proj.obj.velocity = vel;
		l->proj.ptable = &_projVtable;
		l->proj.shouldDelete = false;	//defaults to true oops
		l->proj.deleteFunction = _BPDelete;

		l->proj.hitbox = hitbox;

		l->hitBoss = false;

		//having a hitbox for this might be redundant because the sprite is the same size
		//but I want my collision system to be consistent, which means everything is gonna get a 
		//bounding box. I think the unoptimization of moving 4 floats an enemy is a hit we can take

		Bounds2D spriteSize = {
			{-LASER_WIDTH, -LASER_HEIGHT},
			{LASER_WIDTH, LASER_HEIGHT}
		};

		Bounds2D spriteUV = {
			{LASER_START_U, LASER_START_V},
			{LASER_END_U, LASER_END_V}
		};

		l->sprite = spriteNew(playerSprites, spriteSize, spriteUV, 0.5f);
	}
	return &l->proj;
}

//new boss projectile: also a base, uses tags to hit player instead
Projectile* bossProjectileNew(Coord2D pos) {
	BaseProjectile* baseProj = malloc(sizeof(BaseProjectile));
	if (baseProj != NULL) {
		Coord2D vel = { BOSS_PROJ_VELOCITY, 0 };	//always just move forward at a constant speed

		Bounds2D hitbox = {
			{pos.x - BOSS_PROJ_WIDTH, pos.y - BOSS_PROJ_HEIGHT},
			{pos.x + BOSS_PROJ_WIDTH, pos.y + BOSS_PROJ_HEIGHT}
		};

		objInit(&baseProj->proj.obj, &_bossProjTable, pos, vel, hitbox);

		baseProj->proj.obj.position = pos;
		baseProj->proj.obj.velocity = vel;
		baseProj->proj.ptable = &_projVtable;
		baseProj->proj.shouldDelete = false;	//defaults to true oops
		baseProj->proj.deleteFunction = _BPDelete;

		baseProj->proj.hitbox = hitbox;
		baseProj->hitBoss = false;

		//having a hitbox for this might be redundant because the sprite is the same size
		//but I want my collision system to be consistent, which means everything is gonna get a 
		//bounding box. I think the unoptimization of moving 4 floats an enemy is a hit we can take

		Bounds2D spriteSize = {
			{-BOSS_PROJ_WIDTH, -BOSS_PROJ_HEIGHT},
			{BOSS_PROJ_WIDTH, BOSS_PROJ_HEIGHT}
		};

		Bounds2D spriteUV = {
			{BOSS_PROJ_START_U, BOSS_PROJ_START_V},
			{BOSS_PROJ_END_U, BOSS_PROJ_END_V}
		};

		baseProj->sprite = spriteNew(bossSprites, spriteSize, spriteUV, 0.5f);
	}
	return &baseProj->proj;
}

//completely separate from other projectile news, uses two points to calculate a trajectory instead of moving forward
Projectile* enemyProjectileNew(Coord2D pos, Coord2D playerPos) {
	EnemyProjectile* enemyProj = malloc(sizeof(EnemyProjectile));
	if (enemyProj != NULL) {

		bool playerToLeft = false;
		if (playerPos.x < pos.x) {
			playerToLeft = true;
		}

		bool playerBelow = false;
		if (playerPos.y > pos.y) {
			playerBelow = true;
		}

		float angle = 0;
		if (playerBelow) {
			angle = atan2f(playerPos.y - pos.y, playerPos.x - pos.x);	//angle between two points
		}

		else {
			angle = atan2f(pos.y - playerPos.y, pos.x - playerPos.x);
		}

		angle *= (180 / 3.141529f);	//degree conversion

		if ((playerBelow && playerToLeft) || (!playerBelow && !playerToLeft)) {
			angle = (180 - angle);	//clamp between 0 and 90
		}

		angle *= (3.141592f / 180);	//back to radians for sin and cos

		float xVelocity = ENEMY_PROJ_BASE_VELOCITY * cosf(angle);
		if (playerToLeft) {
			xVelocity *= -1;
		}

		float yVelocity = ENEMY_PROJ_BASE_VELOCITY * sinf(angle);
		if (!playerBelow) {
			yVelocity *= -1;
		}
		//move in the right directions now that we've found magnitude

		Coord2D vel = { xVelocity, yVelocity };	//always just move forward at a constant speed

		Bounds2D hitbox = {
			{pos.x - ENEMY_PROJ_RADIUS, pos.y - ENEMY_PROJ_RADIUS},
			{pos.x + ENEMY_PROJ_RADIUS, pos.y + ENEMY_PROJ_RADIUS}
		};

		objInit(&enemyProj->proj.obj, &_enemyProjTable, pos, vel, hitbox);

		enemyProj->proj.obj.position = pos;
		enemyProj->proj.obj.velocity = vel;
		enemyProj->proj.ptable = &_projVtable;
		enemyProj->proj.shouldDelete = false;	//defaults to true oops
		enemyProj->proj.deleteFunction = _enemyProjDelete;

		enemyProj->proj.hitbox = hitbox;

		//having a hitbox for this might be redundant because the sprite is the same size
		//but I want my collision system to be consistent, which means everything is gonna get a 
		//bounding box. I think the unoptimization of moving 4 floats an enemy is a hit we can take

		Bounds2D spriteSize = {
			{-ENEMY_PROJ_RADIUS, -ENEMY_PROJ_RADIUS},
			{ENEMY_PROJ_RADIUS, ENEMY_PROJ_RADIUS}
		};

		Bounds2D spriteUV = {
			{ENEMY_PROJ_START_U, ENEMY_PROJ_START_V},
			{ENEMY_PROJ_END_U, ENEMY_PROJ_END_V}
		};

		enemyProj->anim = animationNew(enemySprites, spriteSize, spriteUV, ENEMY_PROJ_GAP, 0.5f, 2, 0);
	}
	return &enemyProj->proj;
}

//free projectile and related memory
void projectileDelete(Projectile* p) {
	p->deleteFunction(p);

	objDeinit(&p->obj);
	free(p);
}

void _BPDelete(Projectile* p) {
	BaseProjectile* baseProj = (BaseProjectile*)p;
	spriteDelete(baseProj->sprite);
}

//free missile because it has an anim instead of a sprite
void _missileDelete(Projectile* p) {
	Missile* m = (Missile*)p;
	animationDelete(m->anim);
}

//move forward and move if outside of screen bounds
static void _projUpdate(Object* obj, uint32_t milliseconds) {
	Projectile* proj = (Projectile*)obj;
	objDefaultUpdate(obj, milliseconds);
	//move

	if (obj->position.x < 0 || obj->position.x > 1024 || obj->position.y < 0 || obj->position.y > 768) {
		((Projectile*)obj)->shouldDelete = true;
	}
	//we need to despawn if offscreen, but doing it in here might lead to issues if other projectiles have other stuff
	//i haven't made them yet, so I don't know if they will
}

static void _projDraw(Object* obj) {
	return;
}
//this is here just because it has to exist

static void _BPUpdate(Object* obj, uint32_t milliseconds) {
	BaseProjectile* bp = (BaseProjectile*)obj;
	bp->proj.ptable->update(obj, milliseconds);
}
//base projectile basically just calls projectile stuff, being the simplest

//just moves forward, special missile behavior handled in collision
static void _missileUpdate(Object* obj, uint32_t milliseconds) {
	Missile* m = (Missile*)obj;
	m->proj.ptable->update(obj, milliseconds);
}

//draw sprite
static void _BPDraw(Object* obj) {
	BaseProjectile* bp = (BaseProjectile*)obj;

	spriteDraw(bp->sprite, obj->position);
}

//draw missile using correct anim frame
static void _missileDraw(Object* obj) {
	Missile* m = (Missile*)obj;
	animationDrawFrame(m->anim, obj->position, m->onGround);
	//will draw first sprite (0) if not grounded, will draw grounded sprite otherwise
}

//collision function, can hit a enemy or boss
static bool _projDoCollisions(Object* obj, Object* other) {
	if ((int) other->vtable->tag != (int) ENEMY && (int)other->vtable->tag != (int)BOSS) {
		return false;
	}

	bool didCollide = colliding(&obj->hitbox, &other->hitbox);

	Projectile* proj = (Projectile*)obj;

	if (didCollide) {
		proj->shouldDelete = true;	//deletes itself once it hits an enemy
	}

	return didCollide;
}

//missile collision. can also hit the ground
static bool _missileDoCollisions(Object* obj, Object* other) {
	Missile* m = (Missile*)obj;
	if (((int)other->vtable->tag != (int)TERRAIN)) {
		return _projDoCollisions(obj, other);
	}

	else if(!m->onGround){
		bool didCollide = colliding(&obj->hitbox, &other->hitbox);
		if (didCollide) {
			Coord2D newVel = { MISSILE_END_VELOCITY, 0 };
			obj->position.y = other->hitbox.topLeft.y;
			obj->velocity = newVel;
			m->onGround = true;
		}
		//on the ground hit, zero out the y velocity and change the sprite
		return didCollide;
	}

	else{
		return false;
	}
}

//laser collision
static bool _laserDoCollisions(Object* obj, Object* other) {
	if ((int)other->vtable->tag == (int)ENEMY){
		bool didCollide = colliding(&obj->hitbox, &other->hitbox);

		return didCollide;
	}
	//don't delete on hitting an enemy to go through them

	//if we hit a boss, we basically turn the laser off so they don't multi hit the boss
	else if ((int)other->vtable->tag == (int)BOSS) {
		BaseProjectile* baseProj = (BaseProjectile*)obj;
		if (!baseProj->hitBoss) {
			bool didCollide = colliding(&obj->hitbox, &other->hitbox); 
			if (didCollide) {
				baseProj->hitBoss = true;
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

//move and update anim timer
static void _enemyProjUpdate(Object* obj, uint32_t milliseconds) {
	EnemyProjectile* ep = (EnemyProjectile*)obj;
	ep->proj.ptable->update(obj, milliseconds);
	animationUpdateTimer(ep->anim, milliseconds);
}

//draw basic anim
static void _enemyProjDraw(Object* obj) {
	EnemyProjectile* ep = (EnemyProjectile*)obj;
	animationDraw(ep->anim, obj->position);
}

//delete and free anim
static void _enemyProjDelete(Projectile* proj) {
	EnemyProjectile* ep = (EnemyProjectile*) proj;
	animationDelete(ep->anim);
}

//delete on callback, since enmy projectiles don't hit but get hit
static void _enemyProjCallback(Object* obj, uint8_t tag) {
	EnemyProjectile* ep = (EnemyProjectile*)obj;
	ep->proj.shouldDelete = true;
}

//no callback, since they hit and not get hit
static void _projCallback(Object* obj, uint8_t tag) {
	return;
}

//getter for observer
bool projectileGetDelete(Projectile* p) {
	return p->shouldDelete;
}

//you get it
static void _bossProjCallback(Object* obj, uint8_t tag) {
	BaseProjectile* bp = (BaseProjectile*)obj;
	bp->proj.shouldDelete = true;
}