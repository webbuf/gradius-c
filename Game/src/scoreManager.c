#include "scoreManager.h"
#include "object.h"
#include "animation.h"
#include "baseTypes.h"

#define DEFAULT_HI_SCORE 50000

typedef struct scoreManager_t {
	uint32_t score;
	uint32_t hiScore;
}ScoreManager;
//not an object, just a score container

ScoreManager* scoreCounter;

//initialize score manager with high score and starting score
ScoreManager* scoreNew(uint32_t startingScore) {
	ScoreManager* s = malloc(sizeof(ScoreManager));
	if (s != NULL) {
		s->score = startingScore;
		if (startingScore > DEFAULT_HI_SCORE) {
			s->hiScore = startingScore;
		}
		else {
			s->hiScore = DEFAULT_HI_SCORE;
		}
	}
	scoreCounter = s;
	return s;
}

//free score memory
void scoreDelete(ScoreManager* s) {
	free(s);
}

//increase score and hi score if needed
void increaseScore(ScoreManager* s, uint32_t increase) {
	s->score += increase;
	s->hiScore = max(s->score, s->hiScore);
}

//getters
uint32_t getScore(ScoreManager* s) {
	return s->score;
}

uint32_t getHiScore(ScoreManager* s) {
	return s->hiScore;
}