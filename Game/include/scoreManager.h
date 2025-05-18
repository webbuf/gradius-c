#pragma once
#include "baseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct scoreManager_t ScoreManager;

ScoreManager* scoreNew(uint32_t startingScore);

extern ScoreManager* scoreCounter;	//TODO: fix this (sorry not happening)

void scoreDelete(ScoreManager* s);

void increaseScore(ScoreManager* s, uint32_t increase);

uint32_t getScore(ScoreManager* s);
uint32_t getHiScore(ScoreManager* s);

#ifdef __cplusplus
}
#endif