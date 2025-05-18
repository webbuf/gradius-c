#pragma once
#include "baseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gm_t GameManager;

typedef void (*GmCallbackFunc)();

GameManager* gameManagerNew(Bounds2D bounds);
void gameManagerDelete(GameManager* gm);

#ifdef __cplusplus
}
#endif