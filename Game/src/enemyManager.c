#include <stdio.h>
#include <assert.h>

#include "enemyManager.h"
#include "spinner.h"
#include "enemy.h"
#include "explosion.h"
#include "chaser.h"
#include "sinner.h"
#include "wall.h"
#include "boss.h"
#include "shooter.h"
#include "jumper.h"
#include "stomper.h"

#define SPAWN_X       1100
#define SCREEN_HEIGHT 768

#define CHASER_SPAWN_GAP (768 / 8) * 3
#define SINNER_SPAWN_OFFSET 140
#define STOMPER_SPAWN_X     -24
#define WALL_FULL_WIDTH     144
#define WALL_FULL_HEIGHT    39
#define WALL_HEIGHT         19.5f

#define PLAY_AREA_MAX_Y 700
#define JUMPER_SPAWN_HEIGHT 625
//intentionally offscreen
//information we need to know to know where to spawn enemies
//some have fixed spawns

//spawn at 50 to be on top wall and 650 to be on bottom wall

const char* ENEMY_SPAWN_FORMAT = "%d\t%d\t%d";

typedef struct enemyManager_t {
    Object obj;

	Enemy** enemies;    //track all enemies
    Explosion** explosions; //track all explosion graphics
    Wall** walls;   //track all walls: kind of an enemy when you think about it
    Boss* boss;     //track da boss

    EnemyInfo** spawns; //list of enemy spawn points: time, info, and position
    uint8_t currentSpawn;   //which spawn we're waiting to do
    uint8_t maxSpawns;      //how many times enemies spawn
    uint32_t timer;         //how long the game's been going

    Object* target; 
    PowerupManager* powerupSpawner;
    ProjectileManager* projectileSpawner;
    //storage to give information about the player and spawners to enemies

    Background* background;
    //can stop scroll when boss spawns

    bool bossDead; //observers all the way up: lets the game state manager know the game's over
}EnemyManager;

//struct holding enemy id, position, and time to spawn
typedef struct enemyInfo_t {
    uint8_t enemyType;
    float y;
    uint32_t time;
}EnemyInfo;

static uint8_t enemyCount = 24; //max number of enemies
static uint8_t wallCount = 16;  //set number of walls: enough to always tile the whole screen

static void _emUpdate(Object* obj, uint32_t milliseconds);
static void _emDraw(Object* obj);
static void _spawnExplosion(EnemyManager* em, Coord2D pos);

static ObjVtable _domTable = {
    _emDraw,
    _emUpdate,
    NOCOLLIDE,
    dummyCollide,
    dummyCallback
};

//creates a new enemy manager given the spawn file, a time to start at, and everything an enemy might need to know about
EnemyManager* emNew(const char* filename, PowerupManager* powerupSpawner, ProjectileManager* projectileSpawner, Background* back, Object* target, uint32_t startingTime) {
    EnemyManager* em = malloc(sizeof(EnemyManager));

    if (em != NULL) {
        em->enemies = malloc(sizeof(Enemy*) * enemyCount);

        if (em->enemies != NULL) {
            for (unsigned int i = 0; i < enemyCount; i++) {
                em->enemies[i] = NULL;
            }
        }

        em->explosions = malloc(sizeof(Explosion*) * enemyCount);

        if (em->explosions != NULL) {
            for (unsigned int i = 0; i < enemyCount; i++) {
                em->explosions[i] = NULL;
            }
        }

        em->walls = malloc(sizeof(Wall*) * wallCount);

        if (em->walls != NULL) {
            for (unsigned int i = 0; i < wallCount; i++) {
                em->walls[i] = NULL;
            }
        }

        em->boss = NULL;
        //create all the memory we'll need and null it all out

        Coord2D vel = { 0 , 0 };
        Coord2D pos = { 0, 0 };
        Bounds2D hit = { vel, pos };
        //manager so this isn't needed, just has to be there to be an object

        objInit(&em->obj, &_domTable, vel, pos, hit);

        em->target = target;

        FILE* spawnSheet;
        fopen_s(&spawnSheet, filename, "r");
        int spawnPoints = 0;
        int enemyId = 0;
        int yVal = 0;
        int spawnTime = 0;
        //initialize all these to use as buffers for fscanf

        fscanf_s(spawnSheet, "%d", &spawnPoints);   //read the "header" for our spawn csv to know how many enemies there are
        em->spawns = malloc(sizeof(EnemyInfo*) * spawnPoints);

        for (int i = 0; i < spawnPoints; i++) {
            EnemyInfo* ei = malloc(sizeof(EnemyInfo));
            fscanf_s(spawnSheet, ENEMY_SPAWN_FORMAT, &enemyId, &yVal, &spawnTime);    
            ei->enemyType = enemyId;
            ei->y = (float)yVal;
            ei->time = spawnTime;
            em->spawns[i] = ei;
        }
        fclose(spawnSheet);
        //looping through whole file and set spawns

        em->timer = startingTime;
        em->currentSpawn = 0;
        em->maxSpawns = spawnPoints;
        em->powerupSpawner = powerupSpawner;
        em->projectileSpawner = projectileSpawner;
        em->background = back;
        em->bossDead = false;

        //loop through all out spawns and skip the ones that are before the spawn time we were given
        while (em->currentSpawn < em->maxSpawns && em->spawns[em->currentSpawn]->time < startingTime) {
            if (em->spawns[em->currentSpawn]->enemyType == ENEMY_SPAWN_WALL) {  //if we hit a wall, we move them all the way across instead of starting like normal
                Coord2D pos = { WALL_FULL_WIDTH / 2, WALL_HEIGHT };
                for (int i = 0; i < wallCount; i += 2) {
                    if (em->walls[i] == NULL) {
                        em->walls[i] = wallNew(pos);
                        pos.y = PLAY_AREA_MAX_Y - WALL_HEIGHT;
                        em->walls[i + 1] = wallNew(pos);
                    }
                    pos.x += WALL_FULL_WIDTH;
                    pos.y = WALL_HEIGHT;
                }
            }
            else if (em->spawns[em->currentSpawn]->enemyType == ENEMY_DESPAWN_WALL) {
                for (int i = 0; i < wallCount; i += 2) {
                    if (em->walls[i] != NULL) {
                        wallStopLooping(em->walls[i]);  //if we were told to despawn that means they were spawned, so marked them to go away
                    }
                }
            }
            em->currentSpawn += 1;
        }
    }

    return em;
}

//spawns an enemy with this manager, given a specific id to spawn the enemy and the position to spawn it at
void spawnEnemy(EnemyManager* em, int enemyId, Coord2D pos) {
    int index = -1;

    for (unsigned int i = 0; i < enemyCount; i++) {
        if (em->enemies[i] == NULL) {
            index = i;
            break;
        }
    }
    //find a valid position for us to spawn our new enemy in
    //in all cases, constructor params are just where they are and what they'll need to do their jobs, and if they have a powerup

    if (index != -1) {
        Enemy* e = NULL;
        switch (enemyId) {  //switch for all the different enemy types that can spawn
        case ENEMY_SPINNER: //this next line is marked as an error but it literally isn't and it compiles so idk
            SpinnerBuddy* track = spinnerBuddyNew(em->powerupSpawner);  //spinners have a "buddy" that keeps track of when they spawn a powerup: not an object, cause it just does stuff when it's told to
            for (int i = 0; i < 4; i++) {
                for (unsigned int j = 0; j < enemyCount; j++) {
                    if (em->enemies[j] == NULL) {
                        em->enemies[j] = spinnerNew(pos, track, em->target);
                        break;
                        //spinners always spawn in groups of 4, so we have a hard for loop of 4 times we make a new spinner
                    }
                }
                pos.x += 75;    //have a dedicated offset from each other to do in a line
            }
            break;

        case ENEMY_CHASER:
            pos.x = SPAWN_X;
            pos.y = SCREEN_HEIGHT / 8;
            //chasers always spawn in the same configuration, so we ignore input and make that configuration
            for (int i = 0; i < 3; i++) {   //always 3
                for (unsigned int j = 0; j < enemyCount; j++) {
                    if (em->enemies[j] == NULL) {
                        em->enemies[j] = chaserNew(pos, em->target);
                        break;
                    }
                }
                pos.y += CHASER_SPAWN_GAP;  //move by fixed offset
            }
            break;

        //enemies that can or cannot have a powerup have two separate cases that just flag that bool so we can set it in the csv without needing a 4th column
        case ENEMY_SINNER:
            pos.y -= SINNER_SPAWN_OFFSET;
            for (int i = 0; i < 2; i++) {   //last enemy to always go in groups: spawn 2 with a fixed offset
                for (unsigned int j = 0; j < enemyCount; j++) {
                    if (em->enemies[j] == NULL) {
                        em->enemies[j] = sinnerNew(pos, em->target, em->powerupSpawner, false);
                        break;
                    }
                }
                pos.y += 2 * SINNER_SPAWN_OFFSET;   //spawn on either side of where we've specified in the csv
            }
            break;

        case ENEMY_SINNER_POWERUP:  //same as above with powerup flag
            pos.y -= SINNER_SPAWN_OFFSET;
            for (int i = 0; i < 2; i++) {
                for (unsigned int j = 0; j < enemyCount; j++) {
                    if (em->enemies[j] == NULL) {
                        em->enemies[j] = sinnerNew(pos, em->target, em->powerupSpawner, true);
                        break;
                    }
                }
                pos.y += 2 * SINNER_SPAWN_OFFSET;
            }
            break;

            //yay finally the easy ones: use the indices we have to spawn the guy we need
        case ENEMY_SHOOTER:
            em->enemies[index] = shooterNew(pos, em->projectileSpawner, em->powerupSpawner, em->target, false);
            break;

        case ENEMY_SHOOTER_POWERUP:
            em->enemies[index] = shooterNew(pos, em->projectileSpawner, em->powerupSpawner, em->target, true);
            break;

        case ENEMY_JUMPER:
            pos.y = JUMPER_SPAWN_HEIGHT;    //jumpers spawn at a fixed place, so override whatever's in the csv because we know what we need
            em->enemies[index] = jumperNew(pos, em->projectileSpawner, em->powerupSpawner, em->target, false);
            break;

        case ENEMY_JUMPER_POWERUP:
            pos.y = JUMPER_SPAWN_HEIGHT;

            em->enemies[index] = jumperNew(pos, em->projectileSpawner, em->powerupSpawner, em->target, true);
            break;

        case ENEMY_STOMPER:
            pos.x = STOMPER_SPAWN_X;
            em->enemies[index] = stomperNew(pos, em->projectileSpawner, em->powerupSpawner, em->target, false);
            break;

        case ENEMY_STOMPER_POWERUP:
            pos.x = STOMPER_SPAWN_X;

            em->enemies[index] = stomperNew(pos, em->projectileSpawner, em->powerupSpawner, em->target, true);
            break;

        case ENEMY_SPAWN_WALL:
            pos.x = SPAWN_X;
            pos.y = WALL_HEIGHT;

            for(int i = 0; i < wallCount; i += 2) {
                if (em->walls[i] == NULL) {
                    em->walls[i] = wallNew(pos);
                    pos.y = PLAY_AREA_MAX_Y - WALL_HEIGHT;      //spawn on top and bottom of screen
                    em->walls[i + 1] = wallNew(pos);
                }
                pos.x += WALL_FULL_WIDTH;
                pos.y = WALL_HEIGHT;
            }
            break;
            //always creates all 16 walls, but we null check just to be safe

        case ENEMY_DESPAWN_WALL:
            for (int i = 0; i < wallCount; i++) {
                if (em->walls[i] != NULL) {
                    wallStopLooping(em->walls[i]);
                }
            }
            break;
            //tell walls to stop wrapping and kill themselves

        case ENEMY_BOSS:
            if (em->boss == NULL) {
                em->boss = bossNew(em->projectileSpawner, pos, em->target); //fixed slot not in enemies array
                backgroundPauseScroll(em->background);  //stop scrolling the bg when the boss spawns
            }
            break;

        default:
            break;
        }
    }
}

//spawns the explosions effect at the given position
void _spawnExplosion(EnemyManager* em, Coord2D pos) {
    for (int i = 0; i < enemyCount; i++) {
        if (em->explosions[i] == NULL) {
            em->explosions[i] = explosionNew(pos);
            break;
        }
    }
}

//deletes the enemy manager and frees all related memory
void emDelete(EnemyManager* em) {

    objDeinit(&em->obj);

    for (int i = 0; i < enemyCount; i++) {
        if (em->enemies[i] != NULL) {
            enemyDelete(em->enemies[i]);
        }
    }

    free(em->enemies);

    for (int i = 0; i < enemyCount; i++) {
        if (em->explosions[i] != NULL) {
            explosionDelete(em->explosions[i]);
        }
    }

    free(em->explosions);

    for (int i = 0; i < wallCount; i++) {
        if (em->walls[i] != NULL) {
            wallDelete(em->walls[i]);
        }
    }

    free(em->walls);

    for (int i = 0; i < em->maxSpawns; i++) {
        free(em->spawns[i]);
    }

    if (em->boss != NULL) {
        bossDelete(em->boss);
    }

    free(em->spawns);

    free(em);
}

//update function: monitors all duynamic objects for if they need to be deleted
static void _emUpdate(Object* obj, uint32_t milliseconds) {
    EnemyManager* em = (EnemyManager*)obj;

    em->timer += milliseconds;
    if (em->currentSpawn < em->maxSpawns && em->timer >= em->spawns[em->currentSpawn]->time) {
        //avoiding accessing a null point in the array with short circuiting
        Coord2D pos = { SPAWN_X, em->spawns[em->currentSpawn]->y };
        spawnEnemy(em, em->spawns[em->currentSpawn]->enemyType, pos);
        em->currentSpawn += 1;
    }

    for (int i = 0; i < enemyCount; i++) {
        if (em->enemies[i] != NULL) {
            if (em->enemies[i]->shouldDelete == true) {
                Coord2D deathLocation = em->enemies[i]->obj.position;
                _spawnExplosion(em, deathLocation);
                enemyDelete(em->enemies[i]);
                em->enemies[i] = NULL;
            }
        }
    }

    for (int i = 0; i < enemyCount; i++) {
        if (em->explosions[i] != NULL) {
            if (shouldDeleteExplosion(em->explosions[i]) == true) {
                explosionDelete(em->explosions[i]);
                em->explosions[i] = NULL;
            }
        }
    }

    for (int i = 0; i < wallCount; i++) {
        if (em->walls[i] != NULL) {
            if (shouldDeleteWall(em->walls[i]) == true) {
                wallDelete(em->walls[i]);
                em->walls[i] = NULL;
            }
        }
    }

    if (em->boss != NULL) {
        if (bossShouldDelete(em->boss)) {
            bossDelete(em->boss);
            em->boss = NULL;
            backgroundResumeScroll(em->background);
            em->bossDead = true;    //marker for the higher observer to know the game is over
        }
    }
}

//required and getters
static void _emDraw(Object* obj) {
    return;
}

uint32_t emGetTime(EnemyManager* em) {
    return em->timer;
}

bool emGetWon(EnemyManager* em) {
    return em->bossDead;
}