#include <stdlib.h>
#include <Windows.h>

#include "gameStateManager.h"
#include "object.h"
#include "field.h"
#include "background.h"
#include "vicViper.h"
#include "powerupManager.h"
#include "enemyManager.h"
#include "scoreManager.h"
#include "uiDraw.h"
#include "input.h"
#include "sound.h"
#include "soundInfo.h"
#include "explosion.h"

#define DEATH_TIME_LOSS 30000
#define TIME_TIL_MENU   5000

static const char ENEMY_CSV[] = "asset/spawns.csv";

typedef struct gm_t {
    Object obj;

    uint8_t state;
    uint32_t winTimer;
    int8_t lives;

    Bounds2D levelBounds;
    Background* back;
    VicViper* vic;
    PowerupManager* pum;
    EnemyManager* em;
    ProjectileManager* pm;
    ScoreManager* sm;
    UI* ui;
}GameManager;
//has to keep track of EVERYTHING

enum gameState  {
    STATE_MENU,
    STATE_PLAYING,
    STATE_WON
};

static void _gameManagerUpdate(Object* obj, uint32_t milliseconds);
static void _gameManagerDraw(Object* obj);
static void _gameManagerLoad(GameManager* gm, Bounds2D bounds, uint8_t vicLevel, uint32_t score, uint32_t startingTime);
static void _gameManagerUnload(GameManager* gm);
static void _gameManagerPlaySound(int32_t index);
static void _gameManagerLoadSounds();
static void _gameManagerUnloadSounds();
static void _gameManagerInitCallbacks();
static void _gameManagerClearCallbacks();

static ObjVtable _gameManagerTable = {
    _gameManagerDraw,
    _gameManagerUpdate,
    NOCOLLIDE,
    dummyCollide,
    dummyCallback
};

//handles general game flow all the time
static void _gameManagerUpdate(Object* obj, uint32_t milliseconds) {
    GameManager* gm = (GameManager*)obj;

    if (gm->state == STATE_MENU) {
        //if we're in the menu, just waits til enter is pressed and then moves into the playing state
        if (inputKeyPressed(VK_RETURN)) {
            backgroundDelete(gm->back);
            gm->back = NULL;
            gm->state = STATE_PLAYING;
            gm->lives = 3;
            _gameManagerLoad(gm, gm->levelBounds, 0, 0, 0);
        }
    }

    //monitors the player until they die to reset and lose a life
    else if (gm->state == STATE_PLAYING) {
        if (vicGetState(gm->vic) == VIC_STATE_DEAD) {
            gm->lives -= 1;

            uint32_t score = getScore(gm->sm);
            uint32_t elapsedTime = emGetTime(gm->em);
            uint8_t powerLevel = getPowerupInfo(gm->vic)->powerupLevel;
            //store these before we unload so we can restore them next time

            _gameManagerUnload(gm);
            //delete everything

            if (gm->lives >= 0) {
                _gameManagerLoad(gm, gm->levelBounds, powerLevel, score, elapsedTime);
            }
            //if we still have lives, reset the game back in

            else {
                gm->state = STATE_MENU;
                gm->back = backgroundNew(gm->state);
            }
            //if we're out of lives, go back to the menu
        }

        else if (emGetWon(gm->em)) {
            gm->state = STATE_WON;
        }
        //if the boss dies switch to the winner state
    }

    else if (gm->state == STATE_WON) {
        gm->winTimer += milliseconds;
        if (gm->winTimer >= TIME_TIL_MENU) {
            _gameManagerUnload(gm);
            gm->state = STATE_MENU;
            gm->back = backgroundNew(gm->state);
        }
    }
    //if we win go back to menu
}

//just gotta be here
static void _gameManagerDraw(Object* obj) {
    return;
}

//initializes game manager and goes to menu -> needs bounds because other things need it
GameManager* gameManagerNew(Bounds2D bounds) {
    GameManager* gm = malloc(sizeof(GameManager));
    if (gm != NULL) {
        Coord2D dummy = { 0, 0 };
        objInit(&gm->obj, &_gameManagerTable, dummy, dummy, bounds);

        gm->lives = 3;
        gm->levelBounds = bounds;

        gm->state = STATE_MENU;
        gm->back = backgroundNew(gm->state);
        gm->winTimer = 0;

        _gameManagerLoadSounds();
    }
    return gm;
}

//if we have stuff loaded, unload it
void gameManagerDelete(GameManager* gm) {
    objDeinit(&gm->obj);
    if (gm->state == STATE_PLAYING) {
        _gameManagerUnload(gm);
    }
    else {
        backgroundDelete(gm->back);
    }
    free(gm);
}

//makes everything that we need
static void _gameManagerLoad(GameManager* gm, Bounds2D bounds, uint8_t vicLevel, uint32_t score, uint32_t startingTime) {
    _gameManagerPlaySound(SOUND_BGM);

    gm->back = backgroundNew(gm->state);

    gm->pm = pmNew(bounds);

    uint8_t power = vicLevel;
    if (power > 0) {
        power = 1;
    }
    //if you had any level of powerup in the bank, you start with 1 
    gm->vic = vicNew(bounds, gm->pm, power);

    gm->pum = powerupManagerNew();

    uint32_t time = startingTime;
    if (time > DEATH_TIME_LOSS) {
        time -= DEATH_TIME_LOSS;
    }
    else {
        time = 0;
    }
    //lose 30 seconds of progres, if we can. Otherwise, go back to the start

    gm->em = emNew(ENEMY_CSV, gm->pum, gm->pm, gm->back, vicGetObject(gm->vic), time);

    gm->sm = scoreNew(score);

    gm->ui = uiNew(gm->lives, gm->em, gm->vic);

    gm->winTimer = 0;
    _gameManagerInitCallbacks();
}

//calls delete on everything we're keeping track of
static void _gameManagerUnload(GameManager* gm) {
    soundStop(getSoundHandle(SOUND_BGM));
    vicDelete(gm->vic);
    backgroundDelete(gm->back);
    powerupManagerDelete(gm->pum);
    emDelete(gm->em);
    pmDelete(gm->pm);
    scoreDelete(gm->sm);
    uiDelete(gm->ui);
    _gameManagerClearCallbacks();
}

//init/deinit all the sounds
static void _gameManagerLoadSounds() {
    for (int i = 0; i < SOUND_COUNT; i++) {
        setSoundHandle(i, soundLoad(getSoundFilename(i)));
    }
}

static void _gameManagerUnloadSounds() {
    for (int i = 0; i < SOUND_COUNT; i++) {
        soundUnload(getSoundHandle(i));
    }
}

//uses sound.h to play given sound
static void _gameManagerPlaySound(int32_t index) {
    soundPlay(getSoundHandle(index));
}

//set and clear all the callback
static void _gameManagerInitCallbacks() {
    pmSetCB(_gameManagerPlaySound);
    vicSetCB(_gameManagerPlaySound);
    explosionSetCB(_gameManagerPlaySound);
}

static void _gameManagerClearCallbacks() {
    pmClearCB();
    vicClearCB();
    explosionClearCB();
}