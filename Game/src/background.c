#include "background.h"
#include "object.h"
#include "sprite.h"
#include "hitbox.h"

#define BACKGROUND_START_U	    0
#define BACKGROUND_END_U	    0.5f
#define BACKGROUND_SCROLL_SPEED 0.04f
//game bg doesn't move, but shifts its uvs

#define MENU_SCROLL_SPEED  512
//main menu physically moves into position

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

typedef struct background_t {
	Object obj;

	Sprite* s;
	float scrollSpeed;
}Background;

enum backgroundState {
	BG_MENU,
	BG_GAME
};
//determines which background to make
//bit of a hack to do it like this instead of with polymorphism, but oh well

//static object functions
static void _backgroundUpdate(Object* obj, uint32_t milliseconds);
static void _menuUpdate(Object* obj, uint32_t milliseconds);
static void _backgroundDraw(Object* obj);

static ObjVtable _backgroundTable = {
	_backgroundDraw,
	_backgroundUpdate,
	NOCOLLIDE,
	dummyCollide,
	dummyCallback
};

static ObjVtable _menuTable = {
	_backgroundDraw,
	_menuUpdate,
	NOCOLLIDE,
	dummyCollide,
	dummyCallback
};
//can give the background a different vtable based on the enum - so it's kind of like really bad polymorphism

//makes a new background given what state the game is currently in
Background* backgroundNew(uint8_t state) {
	Background* b = malloc(sizeof(Background));
	if (b != NULL) {
		if (state == BG_MENU) {
			Coord2D menuSpawn = { -SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };	//spawn just offscreen - i do this a lot
			Coord2D menuVelocity = { MENU_SCROLL_SPEED, 0 };

			Bounds2D menuBounds = {
				{-SCREEN_WIDTH, 0}, 
				{0, SCREEN_HEIGHT}
			};
			//whole screen

			Bounds2D menuSize = {
				{-SCREEN_WIDTH / 2, -SCREEN_HEIGHT / 2 },
				{SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 },
			};
			//menu hitbox - weird to say, but since all object move based off shifting a hitbox, it's necessary

			Bounds2D menuUV = { 
				{0, 1},
				{1, 0}
			};
			//whole image

			objInit(&b->obj, &_menuTable, menuSpawn, menuVelocity, menuBounds);
			b->s = spriteNew(menuSprite, menuSize, menuUV, -0.99f);
			b->scrollSpeed = 0;
			//menu doesn't scroll its tex coords
		}

		else {
			Coord2D dummy = { 0,0 };	//zeroed out coor2d for when it doesn't matter. have a lot of these

			Bounds2D world = {
				{0, 0},
				{SCREEN_WIDTH, SCREEN_HEIGHT}
			};
			//always centered, so no calculations required

			Bounds2D uv = {
				{0, 1},
				{0.5, 0}
			};
			//drawing half the bg at all times

			objInit(&b->obj, &_backgroundTable, dummy, dummy, world);

			b->s = spriteNew(backgroundSprite, world, uv, -0.99f);
			b->scrollSpeed = BACKGROUND_SCROLL_SPEED;
		}

	}
	
	return b;
}

//frees bg and related memory
void backgroundDelete(Background* b) {
	spriteDelete(b->s);
	objDeinit(&b->obj);
	free(b);
}

//scroll texture per frame - for background
static void _backgroundUpdate(Object* obj, uint32_t milliseconds) {
	Background* b = (Background*)obj;
	scrollSprite(b->s, b->scrollSpeed, milliseconds);
	return;
}

//moves forward - for menu
static void _menuUpdate(Object* obj, uint32_t milliseconds) {
	objDefaultUpdate(obj, milliseconds);
	if (obj->position.x >= SCREEN_WIDTH / 2) {	//once we're centered, stop moving. 
		obj->velocity.x = 0;	//since we move 512 per second, we should always cleanly hit the middle of the screen (512)
	}
}

//just draw the sprite where we are. if I had a default draw, it would be this
static void _backgroundDraw(Object* obj) {
	Background* b = (Background*)obj;
	spriteDraw(b->s, obj->position);
}

//stop scrolling bg
void backgroundPauseScroll(Background* b) {
	b->scrollSpeed = 0;
}

//resume scrolling bg
void backgroundResumeScroll(Background* b) {
	b->scrollSpeed = BACKGROUND_SCROLL_SPEED;
}