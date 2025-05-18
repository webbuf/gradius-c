#pragma once
#include "baseTypes.h"
#include <Windows.h>
#include <gl/GLU.h>

#ifdef __cplusplus
extern "C" {
#endif

#define X_NORMAL   0
#define X_INVERTED 1
#define Y_NORMAL   0
#define Y_INVERTED 1

typedef struct sprite_t Sprite;

extern GLuint playerSprites;
extern GLuint enemySprites;
extern GLuint uiSprites;
extern GLuint backgroundSprite;
extern GLuint terrainSprites;
extern GLuint menuSprite;
extern GLuint bossSprites;
//having the sprite handles all be public is gross, but honestly at this point it's too late to do anything about it. it's in like every file, so i think if
//any globals are acceptable it's these

void initTextures();

Sprite* spriteNew(GLuint textureHandle, Bounds2D world, Bounds2D uv, float depth);

void spriteDraw(Sprite* s, Coord2D pos);
void spriteDrawInverted(Sprite* s, Coord2D pos, bool invertX, bool invertY);
//like with animations, have a separate inverted draw function so we only need them when we actually draw inverted

void spriteDelete(Sprite* s);

void scrollSprite(Sprite* s, float speed, uint32_t milliseconds);
//literally move the uvs around to scroll it - for background

#ifdef __cplusplus
}
#endif