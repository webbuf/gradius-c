#include "animation.h"
#include <assert.h>

static const uint16_t framerate = 83;	//5 frames at 60 fps

typedef struct anim_t {
	Sprite** sprites;

	uint8_t frames;
	uint8_t currentFrame;
	uint32_t frameTimer;
}Animation;

//creates a new animation object given a spritesheet, its size in game, its uv coords, the space between sprites on the sheet, the number of sprites in the animation, its world depth, number of frames, and starting frame
Animation* animationNew(GLuint textureHandle, Bounds2D worldSize, Bounds2D UV, float uvGap, float depth, uint8_t frameCount, uint8_t startingFrame) {
	assert(startingFrame < frameCount);
	Animation* a = malloc(sizeof(Animation));
	Sprite** sprites = malloc(sizeof(Sprite*) * frameCount);

	if (a != NULL) {
		for (uint8_t i = 0; i < frameCount; i++) {
			Sprite* s = spriteNew(textureHandle, worldSize, UV, depth);
			UV.topLeft.x += uvGap;
			UV.botRight.x += uvGap;	//move by gap. by default, we assume all sprites are vertically aligned
			sprites[i] = s;
		}
		a->frames = frameCount;
		a->sprites = sprites;
		a->currentFrame = startingFrame;
		a->frameTimer = 0;
	}

	return a;
}

//same as above, but now there are rows of sprites to take instead of just a straight line
Animation* animationNewRows(GLuint textureHandle, Bounds2D worldSize, Bounds2D UV, float uvGap, float depth, uint8_t frameCount, uint8_t startingFrame, uint8_t spritesPerRow, float rowGap) {
	assert(startingFrame < frameCount);
	Animation* a = malloc(sizeof(Animation));
	Sprite** sprites = malloc(sizeof(Sprite*) * frameCount);

	uint8_t rowPosition = 0;
	float originalUStart = UV.topLeft.x;
	float originalUEnd = UV.botRight.x;

	if (a != NULL) {
		for (uint8_t i = 0; i < frameCount; i++) {
			Sprite* s = spriteNew(textureHandle, worldSize, UV, depth);
			UV.topLeft.x += uvGap;
			UV.botRight.x += uvGap;
			sprites[i] = s;
			rowPosition += 1;

			if (rowPosition == spritesPerRow) {	//rather than a double for loop we do an if every time
				//this is so we can have, say, 6 sprites on row 1 and 3 on row 2. 
				UV.topLeft.x = originalUStart;	//reset our u
				UV.topLeft.y -= rowGap;			//move our v
				UV.botRight.x = originalUEnd;
				UV.botRight.y -= rowGap;
				rowPosition = 0;
			}
		}
		a->frames = frameCount;
		a->sprites = sprites;
		a->currentFrame = startingFrame;
		a->frameTimer = 0;
	}

	return a;
}

//draw an animation at the given position
void animationDraw(Animation* a, Coord2D pos) {
	if(a->frameTimer >= framerate){	//since we move by however many milliseconds at a time, we can overshoot the target
		a->currentFrame = (a->currentFrame + 1) % a->frames;
		a->frameTimer = 0;
		//advancing the animation in the draw keeps us from looping it in the background while it's not being drawn
	}
	spriteDraw(a->sprites[a->currentFrame], pos);	//once we've got the right sprite, use the sprite draw to draw it
}

//same as above, but with the draw inverted sprite instead 
void animationDrawInverted(Animation* a, Coord2D pos, bool invertX, bool invertY) {
	if (a->frameTimer >= framerate) {	
		a->currentFrame = (a->currentFrame + 1) % a->frames;
		a->frameTimer = 0;
	}
	spriteDrawInverted(a->sprites[a->currentFrame], pos, invertX, invertY);
}

//draw a specific frame from the animation
//this allows us to use animations as a collection of related sprites, rather than just as an animation
//useful for player (who has sprite states instead of an animation) and font rendering
void animationDrawFrame(Animation* a, Coord2D pos, uint8_t frame) {
	assert(frame < a->frames);
	if (frame >= a->frames) {
		return;
	}
	//don't attempt to draw an invalid frame

	spriteDraw(a->sprites[frame], pos);
}

//you know the drill: you can invert this one
void animationDrawFrameInverted(Animation* a, Coord2D pos, uint8_t frame, bool invertX, bool invertY) {
	assert(frame < a->frames);
	if (frame >= a->frames) {
		return;
	}
	//don't attempt to draw an invalid frame

	spriteDrawInverted(a->sprites[frame], pos, invertX, invertY);
}

//frees all memory related to an animation
void animationDelete(Animation* a) {
	for (int i = 0; i < a->frames; i++) {
		spriteDelete(a->sprites[i]);
	}

	free(a->sprites);
	free(a);
}

//advances an animation forward by some number of milliseconds
void animationUpdateTimer(Animation* a, uint32_t milliseconds) {
	a->frameTimer += milliseconds;
}

