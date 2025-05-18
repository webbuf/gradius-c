#include <Windows.h>
#include <gl/GLU.h>
#include <assert.h>

#include "sprite.h"
#include "SOIL.h"

GLuint playerSprites = 0;
GLuint enemySprites = 0;
GLuint uiSprites = 0;
GLuint backgroundSprite = 0;
GLuint terrainSprites = 0;
GLuint menuSprite = 0;
GLuint bossSprites = 0;

static const char VIC_SPRITESHEET[]     = "asset/vicViper.png";
static const char ENEMY_SPRITESHEET[]   = "asset/enemies.png";
static const char UI_SPRITESHEET[]      = "asset/ui_indicator_tweak.png";
static const char BACKGROUND[]		    = "asset/background.png";
static const char MAINMENU[]			= "asset/mainMenu.png";
static const char TERRAIN_SPRITESHEET[] = "asset/terrain.png";
static const char BOSS_SPRITESHEET[]    = "asset/boss.png";
//spritesheet names

typedef struct sprite_t {
	GLuint textureHandle;

	Bounds2D worldSize;
	Bounds2D UV;
	float depth;
}Sprite;

//init da textures and store the handles
void initTextures() {
	if (playerSprites == 0)
	{
		playerSprites = SOIL_load_OGL_texture(VIC_SPRITESHEET, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		assert(playerSprites != 0);
	}

	if (uiSprites == 0)
	{
		uiSprites = SOIL_load_OGL_texture(UI_SPRITESHEET, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		assert(uiSprites != 0);
	}

	if (enemySprites == 0)
	{
		enemySprites = SOIL_load_OGL_texture(ENEMY_SPRITESHEET, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		assert(enemySprites != 0);
	}

	if (bossSprites == 0)
	{
		bossSprites = SOIL_load_OGL_texture(BOSS_SPRITESHEET, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		assert(bossSprites != 0);
	}

	if (backgroundSprite == 0)
	{
		backgroundSprite = SOIL_load_OGL_texture(BACKGROUND, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		assert(backgroundSprite != 0);
	}

	if (terrainSprites == 0)
	{
		terrainSprites = SOIL_load_OGL_texture(TERRAIN_SPRITESHEET, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		assert(terrainSprites != 0);
	}

	if (menuSprite == 0)
	{
		menuSprite = SOIL_load_OGL_texture(MAINMENU, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		assert(menuSprite != 0);
	}
}

//makes a sprite given handle, size, and uvs
Sprite* spriteNew(GLuint id, Bounds2D world, Bounds2D uv, float depth) {
	Sprite* s = malloc(sizeof(Sprite));
	if (s != NULL) {
		s->textureHandle = id;
		s->worldSize = world;
		s->UV = uv;
		s->depth = depth;
	}
	return s;
}

//draws a sprite off location: based off face c draw
void spriteDraw(Sprite* s, Coord2D pos) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, s->textureHandle);
	glBegin(GL_TRIANGLE_STRIP);
	{
		const GLfloat xPositionLeft = s->worldSize.topLeft.x + pos.x;
		const GLfloat xPositionRight = s->worldSize.botRight.x + pos.x;
		const GLfloat yPositionTop = s->worldSize.topLeft.y + pos.y;
		const GLfloat yPositionBottom = s->worldSize.botRight.y + pos.y;

		const GLfloat uLeft = s->UV.topLeft.x;
		const GLfloat uRight = s->UV.botRight.x;
		const GLfloat vTop = s->UV.topLeft.y;
		const GLfloat vBottom = s->UV.botRight.y;

		const float BG_DEPTH = s->depth;
		//i figure setting all of these is more performant than repeated access?

		glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

		//TL
		glTexCoord2f(uLeft, vTop);
		glVertex3f(xPositionLeft, yPositionTop, BG_DEPTH);

		// BL
		glTexCoord2f(uLeft, vBottom);
		glVertex3f(xPositionLeft, yPositionBottom, BG_DEPTH);

		// TR
		glTexCoord2f(uRight, vTop);
		glVertex3f(xPositionRight, yPositionTop, BG_DEPTH);

		// BR
		glTexCoord2f(uRight, vBottom);
		glVertex3f(xPositionRight, yPositionBottom, BG_DEPTH);
	}
	glEnd();
}

//same as above but you can flip the x and y by changing which uv stores there are
void spriteDrawInverted(Sprite* s, Coord2D pos, bool invertX, bool invertY) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, s->textureHandle);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glBegin(GL_TRIANGLE_STRIP);
	{
		const GLfloat xPositionLeft = s->worldSize.topLeft.x + pos.x;
		const GLfloat xPositionRight = s->worldSize.botRight.x + pos.x;
		const GLfloat yPositionTop = s->worldSize.topLeft.y + pos.y;
		const GLfloat yPositionBottom = s->worldSize.botRight.y + pos.y;

		GLfloat uLeft = 0;
		GLfloat uRight = 0;
		GLfloat vTop = 0;
		GLfloat vBottom = 0;

		if (invertX) {
			uRight = s->UV.topLeft.x;
			uLeft = s->UV.botRight.x;
		}

		else {
			uLeft = s->UV.topLeft.x;
			uRight = s->UV.botRight.x;
		}

		if (invertY) {
			vBottom = s->UV.topLeft.y;
			vTop = s->UV.botRight.y;
		}

		else {
			vTop = s->UV.topLeft.y;
			vBottom = s->UV.botRight.y;
		}

		const float BG_DEPTH = s->depth;
		//i figure setting all of these is more performant than repeated access?

		glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

		//TL
		glTexCoord2f(uLeft, vTop);
		glVertex3f(xPositionLeft, yPositionTop, BG_DEPTH);

		// BL
		glTexCoord2f(uLeft, vBottom);
		glVertex3f(xPositionLeft, yPositionBottom, BG_DEPTH);

		// TR
		glTexCoord2f(uRight, vTop);
		glVertex3f(xPositionRight, yPositionTop, BG_DEPTH);

		// BR
		glTexCoord2f(uRight, vBottom);
		glVertex3f(xPositionRight, yPositionBottom, BG_DEPTH);
	}
	glEnd();
}

//free sprite
void spriteDelete(Sprite* s) {
	free(s);
}

//modify uvs to scroll texture
void scrollSprite(Sprite* s, float speed, uint32_t milliseconds) {
	s->UV.topLeft.x += speed * ((float)milliseconds / 1000) ;

	s->UV.botRight.x += speed * ((float)milliseconds / 1000);
}