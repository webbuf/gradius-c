#include "uiDraw.h"
#include "animation.h"
#include "object.h"
#include "baseTypes.h"
#include "scoreManager.h"
#include "vicViper.h"
#include "enemyManager.h"

#include <assert.h>

#define COUNTER_ICON_START_X    340
#define COUNTER_START_X		    380
#define HI_COUNTER_ICON_START_X 720
#define HI_COUNTER_START_X      750
#define COUNTER_START_Y		    733
#define COUNTER_GAP			    21
#define DIGIT_SIZE			    21
#define DIGIT_PAD			    1

#define INDICATOR_START_X	220
#define INDICATOR_START_Y   705
#define INDICATOR_WIDTH		93
#define INDICATOR_HEIGHT    21
#define INDICATOR_PAD		6

#define SPEED_INDICATOR		0
#define MISSILE_INDICATOR	6
#define DOUBLE_INDICATOR	2
#define LASER_INDICATOR		8
#define OPTION_INDICATOR	4
#define QUESTION_INDICATOR  10
#define EMPTY_INDICATOR		12

#define DIGIT_U_START	0.49f
#define DIGIT_V_START   1 - 0.0368f
#define DIGIT_U_END		0.5179f
#define DIGIT_V_END     1 - 0.0882f
#define DIGIT_GAP		0.0319f

#define INDICATOR_U_START 0.0239f
#define INDICATOR_V_START 1 - 0.6029f
#define INDICATOR_U_END	  0.1474f
#define INDICATOR_V_END   1 - 0.6544f
#define INDICATOR_U_GAP	  0.1275f
#define INDICATOR_V_GAP   0.0662f

#define INDICATORS_IN_ROW 6

#define LIVES_ICON_START_X      100
#define LIVES_INDICATOR_START_X 150
#define LIVES_WIDTH				21
#define LIVES_HEIGHT			24
#define LIVES_U_START			0.4582f
#define LIVES_V_START			1 - 0.0368f
#define LIVES_U_END				0.4861f
#define LIVES_V_END				1 - 0.0956f

#define P_U_START	  0.4263f
#define P_U_END		  0.4542f
//other values shared with lives indicator

#define HI_WIDTH  36
#define HI_HEIGHT 21
#define HI_U_START 0.6135f
#define HI_V_START 1 - 0.1103f
#define HI_U_END   0.6614f
#define HI_V_END   1 - 0.1618f

#define WIN_TEXT_WIDTH  290
#define WIN_TEXT_HEIGHT  24
#define WIN_TEXT_U_START 0.4267f
#define WIN_TEXT_V_START 1 - 0.4694f
#define WIN_TEXT_U_END   0.8434f
#define WIN_TEXT_V_END   1 - 0.5578f

#define WIN_TEXT_START_X 365
#define WIN_TEXT_START_Y 350

//why do 3 letters individually instead of a font? I only need these three, my spritesheet doesn't have the letters consistently sized or spaced,
//and it doesn't have all of them. I had to make the H myself

typedef struct ui_t {
	Object obj;

	uint8_t lives;
	EnemyManager* em;
	VicViper* player;

	Animation* numbers;
	Animation* powerupIndicators;

	Sprite* livesIcon;
	Sprite* playerIcon;
	Sprite* hiScoreIcon;
	Sprite* winText;

	Coord2D scorePosition;
	Coord2D scoreIconPosision;
	Coord2D pPosition;
	Coord2D hiScorePosition;
	Coord2D hiScoreIconPosition;
	Coord2D powerupPosition;
	Coord2D livesIconPosition;
	Coord2D livesPosition;
	Coord2D winTextPosition;
	//we have so many positions for things
	//in hindsight some of this could have been one static sprite
}UI;

static void _uiUpdate(Object* obj, uint32_t milliseconds);
static void _uiDraw(Object* obj);
static void _counterDraw(UI* ui, uint32_t value, Coord2D pos);
static void _indicatorDraw(UI* ui, PowerupInfo* power, Coord2D pos);

static ObjVtable _uiTable = {
	_uiDraw,
	_uiUpdate,
	NOCOLLIDE,
	dummyCollide,
	dummyCallback
};

//makes the ui and it's million hard coded positions
UI* uiNew(int8_t lives, EnemyManager* enemy, VicViper* player) {
	assert(lives >= 0 && lives < 10);
	UI* ui = malloc(sizeof(UI));
	if (ui != NULL) {
		ui->lives = lives;
		Bounds2D digitSize = {
			{0, 0},
			{DIGIT_SIZE, DIGIT_SIZE}
		};

		Bounds2D digitUV = {
			{DIGIT_U_START, DIGIT_V_START},
			{DIGIT_U_END, DIGIT_V_END}
		};

		Coord2D scorePos = { COUNTER_START_X, COUNTER_START_Y };
		ui->scorePosition = scorePos;

		Coord2D scoreIconPos = { COUNTER_ICON_START_X, COUNTER_START_Y };
		ui->scoreIconPosision = scoreIconPos;

		Coord2D pPos = { COUNTER_ICON_START_X + DIGIT_SIZE + DIGIT_PAD, COUNTER_START_Y };
		ui->pPosition = pPos;

		Coord2D hiPos = { HI_COUNTER_START_X, COUNTER_START_Y };
		ui->hiScorePosition = hiPos;

		Coord2D hiScoreIconPos = { HI_COUNTER_ICON_START_X, COUNTER_START_Y };
		ui->hiScoreIconPosition = hiScoreIconPos;

		Coord2D livesPos = { LIVES_INDICATOR_START_X, COUNTER_START_Y };
		ui->livesPosition = livesPos;

		Coord2D livesIconPos = { LIVES_ICON_START_X, COUNTER_START_Y };
		ui->livesIconPosition = livesIconPos;
		//this is so many constants just saved

		ui->numbers = animationNew(uiSprites, digitSize, digitUV, DIGIT_GAP, 0.6f, 10, 0);

		Bounds2D iconSize = {
			{0, 0},
			{LIVES_WIDTH, LIVES_HEIGHT}
		};

		Bounds2D livesUV = {
			{LIVES_U_START, LIVES_V_START},
			{LIVES_U_END, LIVES_V_END}
		};

		ui->livesIcon = spriteNew(uiSprites, iconSize, livesUV, 0.6f);

		Bounds2D pUV = {
			{P_U_START, LIVES_V_START},
			{P_U_END, LIVES_V_END}
		};

		ui->playerIcon = spriteNew(uiSprites, iconSize, pUV, 0.6f);

		Bounds2D hiSize = {
			{0, 0},
			{HI_WIDTH, HI_HEIGHT}
		};

		Bounds2D hiUV = {
			{HI_U_START, HI_V_START},
			{HI_U_END, HI_V_END}
		};

		ui->hiScoreIcon = spriteNew(uiSprites, hiSize, hiUV, 0.6f);

		Bounds2D indicatorSize = {
			{0, 0},
			{INDICATOR_WIDTH, INDICATOR_HEIGHT}
		};

		Bounds2D indicatorUV = {
			{INDICATOR_U_START, INDICATOR_V_START},
			{INDICATOR_U_END, INDICATOR_V_END}
		};

		Coord2D powerupPos = { INDICATOR_START_X, INDICATOR_START_Y };
		ui->powerupPosition = powerupPos;
		ui->powerupIndicators = animationNewRows(uiSprites, indicatorSize, indicatorUV, INDICATOR_U_GAP, 0.6f, 14, 0, INDICATORS_IN_ROW, INDICATOR_V_GAP);

		Bounds2D winTextSize = {
			{0, 0},
			{WIN_TEXT_WIDTH, WIN_TEXT_HEIGHT}
		};

		Bounds2D winTextUV = {
			{WIN_TEXT_U_START, WIN_TEXT_V_START},
			{WIN_TEXT_U_END, WIN_TEXT_V_END}
		};

		ui->winText = spriteNew(bossSprites, winTextSize, winTextUV, 0.8f);
		Coord2D winTextPos = { WIN_TEXT_START_X, WIN_TEXT_START_Y };
		ui->winTextPosition = winTextPos;

		ui->em = enemy;
		ui->player = player;

		objInit(&ui->obj, &_uiTable, hiPos, hiPos, indicatorSize);	//position and velocity don't matter so fill it with bullshit
	}
	return ui;
}

//hell yeah
void _uiUpdate(Object* obj, uint32_t millieconds) {
	return;
}

//draws all that shit
void _uiDraw(Object* obj) {
	UI* ui = (UI*)obj;

	spriteDraw(ui->livesIcon, ui->livesIconPosition);
	animationDrawFrame(ui->numbers, ui->livesPosition, ui->lives);

	uint32_t score = getScore(scoreCounter);
	animationDrawFrame(ui->numbers, ui->scoreIconPosision, 1);	//draw a 1 for "player 1", which we always are
	spriteDraw(ui->playerIcon, ui->pPosition);
	_counterDraw(ui, score, ui->scorePosition);

	spriteDraw(ui->hiScoreIcon, ui->hiScoreIconPosition);
	uint32_t hiScore = getHiScore(scoreCounter);
	_counterDraw(ui, hiScore, ui->hiScorePosition);

	PowerupInfo* power = getPowerupInfo(ui->player);
	_indicatorDraw(ui, power, ui->powerupPosition);

	if(emGetWon(ui->em)){
		spriteDraw(ui->winText, ui->winTextPosition);
	}
}

//frees all the memory we made for the ui sprites
void uiDelete(UI* ui) {
	animationDelete(ui->numbers);
	animationDelete(ui->powerupIndicators);
	
	spriteDelete(ui->livesIcon);
	spriteDelete(ui->playerIcon);
	spriteDelete(ui->hiScoreIcon);
	spriteDelete(ui->winText);

	objDeinit(&ui->obj);
	free(ui);
}

//draw one number to 7 places
static void _counterDraw(UI* ui, uint32_t value, Coord2D pos) {
	uint32_t placeValueExtractor = 10;
	Coord2D drawPoint = pos;
	drawPoint.x += 7 * (DIGIT_SIZE + DIGIT_PAD);
	for (int i = 0; i < 7; i++) {
		uint32_t digitToDraw = value % placeValueExtractor;	//mod by ten to get the last digit
		value = value / placeValueExtractor;	//divide by ten to lose that digit
		animationDrawFrame(ui->numbers, drawPoint, digitToDraw);
		drawPoint.x -= (DIGIT_SIZE + DIGIT_PAD);
	}
}

//draw the powerup ui based off its values
static void _indicatorDraw(UI* ui, PowerupInfo* power, Coord2D pos) {
	Coord2D drawPoint = pos;

	uint8_t currentFrame = SPEED_INDICATOR;
	animationDrawFrame(ui->powerupIndicators, drawPoint, currentFrame + (power->powerupLevel == 1));	//if this expression evaluates to true it makes it the selecte instead because it adds one
	//a little gross but I like handling it in only having one draw call
	drawPoint.x += (INDICATOR_WIDTH + INDICATOR_PAD);

	currentFrame = MISSILE_INDICATOR;
	if (power->hasMissile == true) {
		currentFrame = EMPTY_INDICATOR;
	}

	animationDrawFrame(ui->powerupIndicators, drawPoint, currentFrame + (power->powerupLevel == 2));
	drawPoint.x += (INDICATOR_WIDTH + INDICATOR_PAD);

	currentFrame = DOUBLE_INDICATOR;
	if (power->hasDouble == true) {
		currentFrame = EMPTY_INDICATOR;
	}

	animationDrawFrame(ui->powerupIndicators, drawPoint, currentFrame + (power->powerupLevel == 3));
	drawPoint.x += (INDICATOR_WIDTH + INDICATOR_PAD);

	currentFrame = LASER_INDICATOR;
	if (power->hasLaser == true) {
		currentFrame = EMPTY_INDICATOR;
	}

	animationDrawFrame(ui->powerupIndicators, drawPoint, currentFrame + (power->powerupLevel == 4));
	drawPoint.x += (INDICATOR_WIDTH + INDICATOR_PAD);

	currentFrame = OPTION_INDICATOR;
	if (power->followerCount == 2) {
		currentFrame = EMPTY_INDICATOR;
	}

	animationDrawFrame(ui->powerupIndicators, drawPoint, currentFrame + (power->powerupLevel == 5));
	drawPoint.x += (INDICATOR_WIDTH + INDICATOR_PAD);

	currentFrame = QUESTION_INDICATOR;
	if (power->shieldHealth != 0) {
		currentFrame = EMPTY_INDICATOR;
	}
	//if we can't get a powerup agin, it gets grayed out

	animationDrawFrame(ui->powerupIndicators, drawPoint, currentFrame + (power->powerupLevel == 6));
	return;
}