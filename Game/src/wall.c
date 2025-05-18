#include "wall.h"
#include "object.h"
#include "sprite.h"
#include "hitbox.h"

#include <stdio.h>

#define WALL_WIDTH         72
#define WALL_SPRITE_HEIGHT 19.5f
#define WALL_HITBOX_HEIGHT 11

#define WALL_START_U 0.1578f
#define WALL_START_V 1 - 0.8841f
#define WALL_END_U   0.2705f
#define WALL_END_V   1 - 0.9824f

#define SCROLL_SPEED  -80
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768
#define SPRITE_PAD    7
//not 100% why we need this but we do

typedef struct wall_t {
	Object obj;

	Sprite* s;
	bool looping;
	bool shouldDelete;
}Wall;

static void _wallUpdate(Object* obj, uint32_t milliseconds);
static void _wallDraw(Object* obj);
static bool _wallDoCollisions(Object* obj, Object* other);
static void _wallCallback(Object* obj, uint8_t tag);

static ObjVtable _wallTable = {
	_wallDraw,
	_wallUpdate,
	TERRAIN,
	_wallDoCollisions,
	_wallCallback
};

//makes a wall and tells it to loop
Wall* wallNew(Coord2D pos) {
	Wall* w = malloc(sizeof(Wall));
	if (w != NULL) {
		Coord2D vel = { SCROLL_SPEED, 0 };

		Bounds2D hit = {
			{pos.x - WALL_WIDTH, pos.y - WALL_HITBOX_HEIGHT},
			{pos.x + WALL_WIDTH, pos.y + WALL_HITBOX_HEIGHT}
		};

		objInit(&w->obj, &_wallTable, pos, vel, hit);

		Bounds2D spriteSize = {
			{-WALL_WIDTH - SPRITE_PAD, -WALL_SPRITE_HEIGHT},
			{WALL_WIDTH + SPRITE_PAD, WALL_SPRITE_HEIGHT}
		};

		Bounds2D uv = {
			{WALL_START_U, WALL_START_V},
			{WALL_END_U, WALL_END_V}
		};
		
		w->s = spriteNew(terrainSprites, spriteSize, uv, -0.1f);

		w->looping = true;
		w->shouldDelete = false;
	}
	return w;
}

//scrolls wall. if we're offscreen, move to the other side of it if we need to and die otherwise
static void _wallUpdate(Object* obj, uint32_t milliseconds) {
	Wall* w = (Wall*)obj;
	objDefaultUpdate(obj, milliseconds);

	if (obj->position.x < -WALL_WIDTH) {
		if (w->looping) {
			updateHitbox(&obj->hitbox, (SCREEN_WIDTH + WALL_WIDTH * 2 - SPRITE_PAD), 0, 1000);
		}
		else {
			w->shouldDelete = true;
		}
	}
}

//draw wall. inverts based off where it is in the level
static void _wallDraw(Object* obj) {
	Wall* w = (Wall*)obj;

	if (obj->position.y < SCREEN_HEIGHT / 2) {
		spriteDrawInverted(w->s, obj->position, X_NORMAL, Y_INVERTED);
	}
	else {
		spriteDraw(w->s, obj->position);
	}
	//drawHitbox(&obj->hitbox);
}

//gets collided with
static bool _wallDoCollisions(Object* obj, Object* other) {
	return false;
}

static void _wallCallback(Object* obj, uint8_t tag) {
	return;
}

//free wall and related memory
void wallDelete(Wall* w) {
	spriteDelete(w->s);
	objDeinit(&w->obj);
	free(w);
}

//for observer
bool shouldDeleteWall(Wall* w) {
	return w->shouldDelete;
}

//stop looping
void wallStopLooping(Wall* w) {
	w->looping = false;
}