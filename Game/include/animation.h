#pragma once
#include "sprite.h"
#include "baseTypes.h"
#include <gl/GLU.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct anim_t Animation;

//constructors: alternate for if you're taking in sprites over multiples rows instead of just a line
Animation* animationNew(GLuint textureHandle, Bounds2D worldSize, Bounds2D UV, float uvGap, float depth, uint8_t frameCount, uint8_t startingFrame);
Animation* animationNewRows(GLuint textureHandle, Bounds2D worldSize, Bounds2D UV, float uvGap, float depth, uint8_t frameCount, uint8_t startingFrame, uint8_t spritesPerRow, float rowGap);

//draw our current frame. alternate for flipping part of a sprite, and one without to avoid passing in false false all the time
void animationDraw(Animation* a, Coord2D pos);
void animationDrawInverted(Animation* a, Coord2D pos, bool invertX, bool invertY);

//draw specific frame
void animationDrawFrame(Animation* a, Coord2D pos, uint8_t frame);
void animationDrawFrameInverted(Animation* a, Coord2D pos, uint8_t frame, bool invertX, bool invertY);

void animationDelete(Animation* a);

//since animations aren't objects, they need to be told by their owners to advance their timer
void animationUpdateTimer(Animation* a, uint32_t milliseconds);

#ifdef __cplusplus
}
#endif