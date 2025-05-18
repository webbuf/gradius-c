#pragma once
#include "baseTypes.h"
#include "object.h"
#include "animation.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct follower_t {
	Object obj;

	Object* followTarget;
	Animation* anim;

	float velocity;

	bool followY;
	bool followX;
} Follower;
//since the player needs to know a lot about the follower, it's struct is public

Follower* followerNew(Object* target, Coord2D pos, uint8_t speedRange);

void followerDelete(Follower* f);

void followerSpeedUp(Follower* f);
//speed is dynamic to keep up with the player

#ifdef __cplusplus
}
#endif